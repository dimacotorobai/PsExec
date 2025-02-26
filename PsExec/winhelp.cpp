#include "winhelp.h"
#include "runtime.h"


using namespace Runtime;
using namespace Runtime::Functions;


void WindowsHelper::DisplayWindowsError(DWORD errorCode)
{
	// Return if no error
	if (errorCode == ERROR_SUCCESS) return;

	// Get error message
	LPWSTR errorMessage = nullptr;
	size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&errorMessage, 0, NULL);
	memset(errorMessage + wcslen(errorMessage) - 2, 0, 2);

	// Get full message
	size_t finalMessageSize = 2 * (size + 64);
	wchar_t* finalMessage = (wchar_t*)LocalAlloc(LPTR, finalMessageSize);
	if (finalMessage != nullptr) {
		swprintf_s(finalMessage, finalMessageSize, L"[ERROR]: WinError 0x%08x(%s)\n", errorCode, errorMessage);
		WriteConsoleW(hStdOut, finalMessage, wcslen(finalMessage), nullptr, nullptr);
		LocalFree(finalMessage);
	}

	LocalFree(errorMessage);
}

BOOL WindowsHelper::GetProcessNameAndDomain(HANDLE hToken, wchar_t* lpName, LPDWORD nameSize, wchar_t* lpDomain, LPDWORD domainSize)
{
	// Get memory allocation size
	DWORD dwLengthNeeded{ 0 };
	GetTokenInformation(hToken, TokenUser, nullptr, 0, &dwLengthNeeded);

	// Get token user
	PTOKEN_USER pTokenUser = (PTOKEN_USER)LocalAlloc(LPTR, dwLengthNeeded);
	if (pTokenUser == nullptr || !GetTokenInformation(hToken, TokenUser, pTokenUser, dwLengthNeeded, &dwLengthNeeded)) {
		DisplayWindowsError(GetLastError());
		LocalFree(pTokenUser);
		return FALSE;
	}

	// Get domain and user
	SID_NAME_USE sidType;
	if (!LookupAccountSidW(nullptr, pTokenUser->User.Sid, lpName, nameSize, lpDomain, domainSize, &sidType)) {
		DisplayWindowsError(GetLastError());
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
		DisplayWindowsError(GetLastError());
		return L"";
	}

	return elevation.TokenIsElevated ? L"Yes" : L"No";;
}

const wchar_t* WindowsHelper::GetProcessElevationType(HANDLE hToken)
{
	DWORD dwSize;
	TOKEN_ELEVATION_TYPE elevationType;

	if (!GetTokenInformation(hToken, TokenElevationType, &elevationType, sizeof(elevationType), &dwSize)) {
		DisplayWindowsError(GetLastError());
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
		DisplayWindowsError(GetLastError());
		LocalFree(pTIL);
		return L"";
	}

	PUCHAR subAuthCount = GetSidSubAuthorityCount(pTIL->Label.Sid);
	if (subAuthCount == nullptr) {
		DisplayWindowsError(GetLastError());
		LocalFree(pTIL);
		return L"";
	}

	PDWORD subAuthority = GetSidSubAuthority(pTIL->Label.Sid, static_cast<DWORD>(*subAuthCount - 1));
	if (subAuthority == nullptr) {
		DisplayWindowsError(GetLastError());
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
