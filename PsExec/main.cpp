#define WIN32_LEAN_AND_MEAN
#include "ntdll.h"

#include "runtime.h"
#include "winhelp.h"
#include "process.h"


const wchar_t* usageMessage = L"\
Usage:\n\
	PsExec.exe <application>\n\
	PsExec.exe -u <username> -p <password> <application>\n\
	PsExec.exe -tp <TARGET_PID> <application>\n\
	PsExec.exe -tn <TARGET_NAME> <application>\n\
";

int main(int argc, wchar_t* argv[]) {
	if (argc < 2) {
		WriteConsoleW(Runtime::hStdOut, usageMessage, wcslen(usageMessage), nullptr, nullptr);
		return -1;
	}

	wchar_t *lpCommandline, *lpUsername, *lpPassword, *lpProcessId, *lpProcessName;
	lpUsername = lpPassword = lpCommandline = lpProcessId = lpProcessName = nullptr;
	
	for(int i = 0; i < argc; i++) {
		if (!wcscmp(L"-u", argv[i])) {
			lpUsername = argv[i+1];
		}
		else if (!wcscmp(L"-p", argv[i])) {
			lpPassword = argv[i + 1];
		}
		else if (!wcscmp(L"-tp", argv[i])) {
			lpProcessId = argv[i + 1];
		}
		else if (!wcscmp(L"-tn", argv[i])) {
			lpProcessName = argv[i + 1];
		}
	}
	lpCommandline = argv[argc - 1];

	Process::INFORMATION info{ 0 };
	Process::DisplayStartInformation(lpCommandline);
	WindowsHelper::EnableDebugPrivilege();
	if (lpUsername && lpPassword) {
		Process::CreateWithLogon(lpUsername, nullptr, lpPassword, lpCommandline, &info);
	}
	else if (lpProcessId || lpProcessName) {
		DWORD dwProcessId = lpProcessId? Runtime::Functions::wtoi(lpProcessId): WindowsHelper::GetProcessId(lpProcessName);
		Process::CreateWithToken(dwProcessId, lpCommandline, &info);
	}
	else {
		Process::Create(lpCommandline, &info);
	}

	Process::DisplaySummaryInformation(&info);
	return info.dwExitCode;
}

int mainCRTStartup(PPEB peb) {
	int exitCode{ -1 };
	if (Runtime::Init(peb)) {
		int argc{ 0 };
		wchar_t** argv = Runtime::Functions::ConvertCommandLineToArgV(Runtime::Functions::GetCommandLineArgs(), &argc);
		exitCode = main(argc, argv);
		Runtime::Destroy();
	}
	
	ExitProcess(exitCode);
}