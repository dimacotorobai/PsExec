#include "winhelp.h"
#include "runtime.h"
#include "logger.h"

#include <TlHelp32.h>

using namespace Runtime;
using namespace Runtime::Functions;


LPWSTR WindowsHelper::GetWindowsErrorMessage(_In_ DWORD errorCode)
{
	LPWSTR errorMessage = nullptr;

	// Return if no error
	if (errorCode == ERROR_SUCCESS)
		return errorMessage;

	// Get error message
	size_t size = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPWSTR>(&errorMessage),
		0,
		NULL
	);

	// Remove "\r\n"
	SetMemory(errorMessage + WcStringLength(errorMessage) - 2, 0, 4);
	return errorMessage;
}

BOOL WindowsHelper::GetProcessNameAndDomain(HANDLE hToken, wchar_t* lpName, LPDWORD nameSize, wchar_t* lpDomain, LPDWORD domainSize)
{
	// Get memory allocation size
	DWORD dwLengthNeeded{ 0 };
	GetTokenInformation(hToken, TokenUser, nullptr, 0, &dwLengthNeeded);

	// Get token user
	PTOKEN_USER pTokenUser = (PTOKEN_USER)LocalAlloc(LPTR, dwLengthNeeded);
	if (pTokenUser == nullptr || !GetTokenInformation(hToken, TokenUser, pTokenUser, dwLengthNeeded, &dwLengthNeeded)) {
		auto errorCode = GetLastError();
		auto errorMessage = GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		LocalFree(pTokenUser);
		return FALSE;
	}

	// Get domain and user
	SID_NAME_USE sidType;
	if (!LookupAccountSidW(nullptr, pTokenUser->User.Sid, lpName, nameSize, lpDomain, domainSize, &sidType)) {
		auto errorCode = GetLastError();
		auto errorMessage = GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		LocalFree(pTokenUser);
		return FALSE;
	}
	LocalFree(pTokenUser);
	return TRUE;
}

const wchar_t* WindowsHelper::GetProcessElevation(HANDLE hToken)
{
	DWORD dwSize;
	TOKEN_ELEVATION elevation;

	if (!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize)) {
		auto errorCode = GetLastError();
		auto errorMessage = GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		return L"";
	}

	return elevation.TokenIsElevated ? L"Yes" : L"No";;
}

const wchar_t* WindowsHelper::GetProcessElevationType(HANDLE hToken)
{
	DWORD dwSize;
	TOKEN_ELEVATION_TYPE elevationType;

	if (!GetTokenInformation(hToken, TokenElevationType, &elevationType, sizeof(elevationType), &dwSize)) {
		auto errorCode = GetLastError();
		auto errorMessage = GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		return L"";
	}

	switch (elevationType) {
	case TokenElevationTypeDefault:
		return L"Default (Standard User)";

	case TokenElevationTypeFull:
		return L"Full (Elevated)";

	case TokenElevationTypeLimited:
		return L"Limited (Limited User)";

	default:
		return L"Unknown";
	}
}

const wchar_t* WindowsHelper::GetProcessIntegrityLevel(HANDLE hToken)
{
	DWORD dwLengthNeeded;
	GetTokenInformation(hToken, TokenIntegrityLevel, nullptr, 0, &dwLengthNeeded);

	PTOKEN_MANDATORY_LABEL pTIL = (PTOKEN_MANDATORY_LABEL)LocalAlloc(LPTR, dwLengthNeeded);
	if (pTIL == nullptr || !GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, dwLengthNeeded, &dwLengthNeeded)) {
		auto errorCode = GetLastError();
		auto errorMessage = GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		LocalFree(pTIL);
		return L"";
	}

	PUCHAR subAuthCount = GetSidSubAuthorityCount(pTIL->Label.Sid);
	if (subAuthCount == nullptr) {
		auto errorCode = GetLastError();
		auto errorMessage = GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		LocalFree(pTIL);
		return L"";
	}

	PDWORD subAuthority = GetSidSubAuthority(pTIL->Label.Sid, static_cast<DWORD>(*subAuthCount - 1));
	if (subAuthority == nullptr) {
		auto errorCode = GetLastError();
		auto errorMessage = GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		LocalFree(pTIL);
		return L"";
	}

	LocalFree(pTIL);
	switch (*subAuthority) {
	case SECURITY_MANDATORY_UNTRUSTED_RID:
		return L"Untrusted";

	case SECURITY_MANDATORY_LOW_RID:
		return L"Low";

	case SECURITY_MANDATORY_MEDIUM_RID:
		return L"Medium";

	case SECURITY_MANDATORY_MEDIUM_PLUS_RID:
		return L"Medium Plus";

	case SECURITY_MANDATORY_HIGH_RID:
		return L"High";

	case SECURITY_MANDATORY_SYSTEM_RID:
		return L"System";

	case SECURITY_MANDATORY_PROTECTED_PROCESS_RID:
		return L"Protected";

	default:
		return L"Unknown";
	}
}

DWORD WindowsHelper::GetProcessId(LPWSTR lpProcessName)
{
	DWORD dwProcId{ 0 };
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		auto errorCode = GetLastError();
		auto errorMessage = GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		return 0;
	}

	PROCESSENTRY32W pe32;
	pe32.dwSize = sizeof(pe32);
	if (Process32FirstW(hSnapshot, &pe32)) {
		do {
			if (!WcStringCompare(pe32.szExeFile, lpProcessName)) {
				dwProcId = pe32.th32ProcessID;
				break;
			}
		} while (Process32NextW(hSnapshot, &pe32));
	}
	else {
		auto errorCode = GetLastError();
		auto errorMessage = GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		return 0;
	}

	CloseHandle(hSnapshot);
	return dwProcId;
}

BOOL WindowsHelper::EnableDebugPrivilege()
{
	HANDLE hToken;
	if (OpenProcessToken(NtCurrentProcess, TOKEN_ADJUST_PRIVILEGES, &hToken)) {
		return SetPrivilege(hToken, SE_DEBUG_NAME, TRUE);
	}

	return FALSE;
}

BOOL WindowsHelper::SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	BOOL result{ FALSE };
	LUID luid;

	if (LookupPrivilegeValueW(nullptr, lpszPrivilege, &luid))
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = (bEnablePrivilege) ? SE_PRIVILEGE_ENABLED : 0;
		if (AdjustTokenPrivileges(hToken, FALSE, &tp, NULL, (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
		{
			result = (GetLastError() == ERROR_SUCCESS);
		}
	}

	return result;
}
