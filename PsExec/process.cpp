#include "process.h"
#include "runtime.h"
#include "winhelp.h"
#include "logger.h"

using namespace Runtime;
using namespace Runtime::Functions;


BOOL Process::Create(LPWSTR lpCommandLine, LP_INFORMATION lpInfo)
{
	::PROCESS_INFORMATION pi{ 0 };
	::STARTUPINFO si{ sizeof(si) };
	if (CreateProcessW(nullptr, lpCommandLine, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
		lpInfo->bCreated = true;
		_GetProcessInformation(pi.hProcess, lpInfo);
		WaitForSingleObject(pi.hProcess, INFINITE);
		_GetProcessExitCode(pi.hProcess, lpInfo);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	else {
		auto errorCode = GetLastError();
		auto errorMessage = WindowsHelper::GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		return FALSE;
	}

	return TRUE;
}

BOOL Process::CreateWithLogon(LPCWSTR lpcUsername, LPCWSTR lpcDomain, LPCWSTR lpcPassword, LPWSTR lpCommandLine, LP_INFORMATION lpInfo)
{
	::PROCESS_INFORMATION pi{ 0 };
	::STARTUPINFO si{ sizeof(si) };
	if (CreateProcessWithLogonW(lpcUsername, lpcDomain, lpcPassword, LOGON_WITH_PROFILE, nullptr, lpCommandLine, 0, nullptr, nullptr, &si, &pi)) {
		lpInfo->bCreated = true;
		_GetProcessInformation(pi.hProcess, lpInfo);
		WaitForSingleObject(pi.hProcess, INFINITE);
		_GetProcessExitCode(pi.hProcess, lpInfo);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	else {
		auto errorCode = GetLastError();
		auto errorMessage = WindowsHelper::GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		return FALSE;
	}

	return TRUE;
}

BOOL Process::CreateWithToken(DWORD dwProcessId, LPWSTR lpCommandLine, LP_INFORMATION lpInfo)
{
	// Get Process Handle
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
	if (!hProcess) {
		auto errorCode = GetLastError();
		auto errorMessage = WindowsHelper::GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		return FALSE;
	}

	// Get Process Token
	HANDLE hToken;
	if (!OpenProcessToken(hProcess, MAXIMUM_ALLOWED, &hToken)) {
		auto errorCode = GetLastError();
		auto errorMessage = WindowsHelper::GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		return FALSE;
	}

	// Duplicate token
	HANDLE hDupToken;
	if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, nullptr, SecurityIdentification, TokenPrimary, &hDupToken)) {
		auto errorCode = GetLastError();
		auto errorMessage = WindowsHelper::GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		return FALSE;
	}

	// Create Process
	::PROCESS_INFORMATION pi{ 0 };
	::STARTUPINFO si{ sizeof(si) };
	if (CreateProcessWithTokenW(hDupToken, 0, nullptr, lpCommandLine, 0, nullptr, nullptr, &si, &pi)) {
		lpInfo->bCreated = true;
		_GetProcessInformation(pi.hProcess, lpInfo);
		WaitForSingleObject(pi.hProcess, INFINITE);
		_GetProcessExitCode(pi.hProcess, lpInfo);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	else {
		auto errorCode = GetLastError();
		auto errorMessage = WindowsHelper::GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		return FALSE;
	}

	return TRUE;
}

void Process::DisplayStartInformation(LPWSTR lpCommandLine)
{
	Logger::Print(L"================================================================\n");
	Logger::Print(L"PsExec Start:\n");
	Logger::Print(L"    Command Line: %s\n", lpCommandLine);
	Logger::Print(L"================================================================\n");
}

void Process::DisplaySummaryInformation(LP_INFORMATION lpInfo)
{
	Logger::Print(L"================================================================\n");
	Logger::Print(L"PsExec Summary:\n");
	Logger::Print(L"	Process ID: 0x%08X\n", lpInfo->dwPID);
	Logger::Print(L"	User: %s\\%s\n", lpInfo->szDomain, lpInfo->szName);
	Logger::Print(L"	Elevated: %s\n", lpInfo->lpElevated);
	Logger::Print(L"	Elevation Type: %s\n", lpInfo->lpElevationType);
	Logger::Print(L"	Integrity Level: %s\n", lpInfo->lpIntergrityLevel);
	Logger::Print(L"	Image Path: %s\n", lpInfo->szImagePath);
	Logger::Print(L"	Return Code: 0x%08X\n", lpInfo->dwExitCode);
	Logger::Print(L"================================================================\n");
}

BOOL Process::_GetProcessInformation(HANDLE hProcess, LP_INFORMATION lpInfo)
{
	// Open handle to the process token
	HANDLE hToken = nullptr;
	if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
		auto errorCode = GetLastError();
		auto errorMessage = WindowsHelper::GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		return FALSE;
	}

	// Get process id
	lpInfo->dwPID = GetProcessId(hProcess);

	// Display username
	DWORD nameSize = sizeof(lpInfo->szName) / sizeof(lpInfo->szName[0]);
	DWORD domainSize = sizeof(lpInfo->szDomain) / sizeof(lpInfo->szDomain[0]);
	WindowsHelper::GetProcessNameAndDomain(hToken, lpInfo->szName, &nameSize, lpInfo->szDomain, &domainSize);

	// Display elevation, elevation type, and integrity level
	lpInfo->lpElevated = WindowsHelper::GetProcessElevation(hToken);
	lpInfo->lpElevationType = WindowsHelper::GetProcessElevationType(hToken);
	lpInfo->lpIntergrityLevel = WindowsHelper::GetProcessIntegrityLevel(hToken);

	// Display executable path
	DWORD imagePathSize{ sizeof(lpInfo->szImagePath) / sizeof(lpInfo->szImagePath[0]) };
	QueryFullProcessImageNameW(hProcess, 0, lpInfo->szImagePath, &imagePathSize);

	return TRUE;
}

BOOL Process::_GetProcessExitCode(HANDLE hProcess, LP_INFORMATION lpInfo)
{
	if (!GetExitCodeProcess(hProcess, &lpInfo->dwExitCode)) {
		auto errorCode = GetLastError();
		auto errorMessage = WindowsHelper::GetWindowsErrorMessage(errorCode);
		LOG_ERROR(L"%s (0x%08x)\n", errorMessage, errorCode);
		LocalFree(errorMessage);
		return FALSE;
	};

	return TRUE;
}
