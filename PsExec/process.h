#ifndef PROCESS_H
#define PROCESS_H

#include "ntdll.h"

namespace Process {
	typedef struct _Information {
		// CreateProcess success status
		bool bCreated;
		
		// Handles to the process and thread
		DWORD dwPID;
		
		// Name and domain of the user running the process
		wchar_t szName[64];
		wchar_t szDomain[64];
		
		// Related to runing as standard user or administrator
		const wchar_t* lpElevated;
		const wchar_t* lpElevationType;
		const wchar_t* lpIntergrityLevel;

		// Image Path
		wchar_t szImagePath[MAX_PATH];

		// Return code
		DWORD dwExitCode;
	} *LP_INFORMATION, INFORMATION;

	
	BOOL Create(LPWSTR lpCommandLine, LP_INFORMATION lpInfo);
	BOOL CreateWithLogon(LPCWSTR lpcUsername, LPCWSTR lpcDomain, LPCWSTR lpcPassword, LPWSTR lpCommandLine, LP_INFORMATION lpInfo);
	BOOL CreateWithToken(DWORD dwProcessId, LPWSTR lpCommandLine, LP_INFORMATION lpInfo);
	void DisplayStartInformation(LPWSTR lpCommandLine);
	void DisplaySummaryInformation(LP_INFORMATION lpInfo);

	BOOL _GetProcessInformation(HANDLE hProcess, LP_INFORMATION lpInfo);
	BOOL _GetProcessExitCode(HANDLE hProcess, LP_INFORMATION lpInfo);
}
#endif