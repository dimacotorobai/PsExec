#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winternl.h>
#include <shellapi.h>

#include "runtime.h"
#include "winhelp.h"
#include "process.h"


const wchar_t* usageMessage = L"\
Usage:\n\
	PsExec.exe <application>\n\
	PsExec.exe -u <username> -p <password> <application>\n\
";

int main(int argc, wchar_t* argv[]) {
	if (argc < 2) {
		WriteConsole(Runtime::hStdOut, usageMessage, wcslen(usageMessage), nullptr, nullptr);
		return -1;
	}

	wchar_t *lpUsername, *lpPassword, *lpCommandline;
	lpUsername = lpPassword = lpCommandline = nullptr;
	for(int i = 0; i < argc; i++) {
		if (!wcscmp(L"-u", argv[i])) {
			lpUsername = argv[i+1];
		}
		else if (!wcscmp(L"-p", argv[i])) {
			lpPassword = argv[i + 1];
		}
	}
	lpCommandline = argv[argc - 1];

	Process::INFORMATION info{ 0 };
	Process::DisplayStartInformation(lpCommandline);
	if (lpUsername && lpPassword)
		Process::CreateWithLogon(lpUsername, nullptr, lpPassword, lpCommandline, &info);
	else
		Process::Create(lpCommandline, &info);

	Process::DisplaySummaryInformation(&info);
	return info.dwExitCode;
}

int mainCRTStartup(PPEB peb) {
	int exitCode{ -1 };
	if (Runtime::Init()) {
		// Get command line arguments
		int argc{ 0 };
		wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
		if (!argv) ExitProcess(-1);

		exitCode = main(argc, argv);

		// Free memory
		LocalFree(argv);
	}
	
	ExitProcess(exitCode);
}