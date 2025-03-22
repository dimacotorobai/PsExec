#include "process.h"
#include "runtime.h"
#include "winhelp.h"


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
		WindowsHelper::DisplayWindowsError(GetLastError());
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
		WindowsHelper::DisplayWindowsError(GetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOL Process::CreateWithToken(DWORD dwProcessId, LPWSTR lpCommandLine, LP_INFORMATION lpInfo)
{	
	// Get Process Handle
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
	if (!hProcess) {
		WindowsHelper::DisplayWindowsError(GetLastError());
		return FALSE;
	}

	// Get Process Token 
	HANDLE hToken;
	if (!OpenProcessToken(hProcess, MAXIMUM_ALLOWED, &hToken)) {
		WindowsHelper::DisplayWindowsError(GetLastError());
		return FALSE;
	}

	// Duplicate token
	HANDLE hDupToken;
	if(!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, nullptr, SecurityIdentification, TokenPrimary, &hDupToken)) {
		WindowsHelper::DisplayWindowsError(GetLastError());
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
		WindowsHelper::DisplayWindowsError(GetLastError());
		return FALSE;
	}

	return TRUE;
}

void Process::DisplayStartInformation(LPWSTR lpCommandLine)
{
	// Allocate print buffer
	constexpr size_t BUFFER_SIZE{ 512 };
	wchar_t buffer[BUFFER_SIZE]{ 0 };

	memset(buffer, 0, sizeof(buffer));
	swprintf_s(buffer, BUFFER_SIZE, L"================================================================\n");
	WriteConsoleW(Runtime::hStdOut, buffer, wcslen(buffer), nullptr, nullptr);

	memset(buffer, 0, sizeof(buffer));
	swprintf_s(buffer, BUFFER_SIZE, L"PsExec Start:\n");
	WriteConsoleW(Runtime::hStdOut, buffer, wcslen(buffer), nullptr, nullptr);

	memset(buffer, 0, sizeof(buffer));
	swprintf_s(buffer, BUFFER_SIZE, L"    Command Line: %s\n", lpCommandLine);
	WriteConsoleW(Runtime::hStdOut, buffer, wcslen(buffer), nullptr, nullptr);

	memset(buffer, 0, sizeof(buffer));
	swprintf_s(buffer, BUFFER_SIZE, L"================================================================\n");
	WriteConsoleW(Runtime::hStdOut, buffer, wcslen(buffer), nullptr, nullptr);
}

void Process::DisplaySummaryInformation(LP_INFORMATION lpInfo)
{
	// Allocate print buffer
	constexpr size_t BUFFER_SIZE{ 512 };
	wchar_t buffer[BUFFER_SIZE]{ 0 };

	memset(buffer, 0, sizeof(buffer));
	swprintf_s(buffer, BUFFER_SIZE, L"================================================================\n");
	WriteConsoleW(Runtime::hStdOut, buffer, wcslen(buffer), nullptr, nullptr);

	memset(buffer, 0, sizeof(buffer));
	swprintf_s(buffer, BUFFER_SIZE, L"PsExec Summary:\n");
	WriteConsoleW(Runtime::hStdOut, buffer, wcslen(buffer), nullptr, nullptr);

	memset(buffer, 0, sizeof(buffer));
	swprintf_s(buffer, BUFFER_SIZE, L"	Process ID: 0x%08X\n", lpInfo->dwPID);
	WriteConsoleW(Runtime::hStdOut, buffer, wcslen(buffer), nullptr, nullptr);

	memset(buffer, 0, sizeof(buffer));
	swprintf_s(buffer, BUFFER_SIZE, L"	User: %s\\%s\n", lpInfo->szDomain, lpInfo->szName);
	WriteConsoleW(Runtime::hStdOut, buffer, wcslen(buffer), nullptr, nullptr);

	memset(buffer, 0, sizeof(buffer));
	swprintf_s(buffer, BUFFER_SIZE, L"	Elevated: %s\n", lpInfo->lpElevated);
	WriteConsoleW(Runtime::hStdOut, buffer, wcslen(buffer), nullptr, nullptr);

	memset(buffer, 0, sizeof(buffer));
	swprintf_s(buffer, BUFFER_SIZE, L"	Elevation Type: %s\n", lpInfo->lpElevationType);
	WriteConsoleW(Runtime::hStdOut, buffer, wcslen(buffer), nullptr, nullptr);

	memset(buffer, 0, sizeof(buffer));
	swprintf_s(buffer, BUFFER_SIZE, L"	Integrity Level: %s\n", lpInfo->lpIntergrityLevel);
	WriteConsoleW(Runtime::hStdOut, buffer, wcslen(buffer), nullptr, nullptr);

	memset(buffer, 0, sizeof(buffer));
	swprintf_s(buffer, BUFFER_SIZE, L"	Image Path: %s\n", lpInfo->szImagePath);
	WriteConsoleW(Runtime::hStdOut, buffer, wcslen(buffer), nullptr, nullptr);

	memset(buffer, 0, sizeof(buffer));
	swprintf_s(buffer, BUFFER_SIZE, L"	Return Code: 0x%08X\n", lpInfo->dwExitCode);
	WriteConsoleW(Runtime::hStdOut, buffer, wcslen(buffer), nullptr, nullptr);

	memset(buffer, 0, sizeof(buffer));
	swprintf_s(buffer, BUFFER_SIZE, L"================================================================\n");
	WriteConsoleW(Runtime::hStdOut, buffer, wcslen(buffer), nullptr, nullptr);
}

BOOL Process::_GetProcessInformation(HANDLE hProcess, LP_INFORMATION lpInfo)
{
	// Open handle to the process token
	HANDLE hToken = nullptr;
	if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
		WindowsHelper::DisplayWindowsError(GetLastError());
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
		WindowsHelper::DisplayWindowsError(GetLastError());
		return FALSE;
	};

	return TRUE;
}
