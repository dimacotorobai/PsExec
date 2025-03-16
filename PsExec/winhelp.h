#ifndef WINHELP_H
#define WINHELP_H

#include "ntdll.h"

namespace WindowsHelper 
{
	void DisplayWindowsError(DWORD errorCode);
	
	BOOL GetProcessNameAndDomain(HANDLE hToken, wchar_t* lpName, LPDWORD nameSize, wchar_t* lpDomain, LPDWORD domainSize);
	const wchar_t* GetProcessElevation(HANDLE hToken);
	const wchar_t* GetProcessElevationType(HANDLE hToken);
	const wchar_t* GetProcessIntegrityLevel(HANDLE hToken);

	DWORD GetProcessId(LPWSTR lpProcessName);

	BOOL EnableDebugPrivilege();
	BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
}
#endif