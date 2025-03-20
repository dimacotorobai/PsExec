#ifndef WINHELP_H
#define WINHELP_H

#include "ntdll.h"

namespace WindowsHelper 
{
	// Windows error related stuff
	void DisplayWindowsError(DWORD errorCode);
	
	// Process token get information stuff
	BOOL GetProcessNameAndDomain(HANDLE hToken, wchar_t* lpName, LPDWORD nameSize, wchar_t* lpDomain, LPDWORD domainSize);
	const wchar_t* GetProcessElevation(HANDLE hToken);
	const wchar_t* GetProcessElevationType(HANDLE hToken);
	const wchar_t* GetProcessIntegrityLevel(HANDLE hToken);

	// Find process ids by exe file name
	DWORD GetProcessId(LPWSTR lpProcessName);

	// Enable privileges stuff
	BOOL EnableDebugPrivilege();
	BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
}
#endif