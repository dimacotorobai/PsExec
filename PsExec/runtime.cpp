#include "runtime.h"

/*********************************
* Constants
*********************************/
#define MAXIMUM_COMMAND_LINE_ARGUMENTS 20


/*********************************
* Declare global variables
*********************************/
namespace Runtime {
	PPEB pPEB = nullptr;

	HANDLE hStdIn		= INVALID_HANDLE_VALUE;
	HANDLE hStdOut		= INVALID_HANDLE_VALUE;
	HANDLE hStdError	= INVALID_HANDLE_VALUE;

	LPWSTR pCommandLine = nullptr;
	LPWSTR* pArgV = nullptr;
	INT iArgC = 0;
	
	namespace Functions {
		PFN_SPRINTF_S sprintf_s		= nullptr;
		PFN_SWPRINTF_S	swprintf_s	= nullptr;
		
		int wtoi(const wchar_t* lpNumber)
		{
			int result = 0;
			if (lpNumber) {
				int numLength = wcslen(lpNumber);

				// Hexadecimal
				if (numLength > 2 && (lpNumber[0] == '0' && (lpNumber[1] == 'x' || lpNumber[1] == 'X'))) {
					int temp = 0;
					for (int i = 2; i < numLength; i++) {
						if (lpNumber[i] == 'A')
							temp = 10;
						else if (lpNumber[i] == 'B')
							temp = 11;
						else if (lpNumber[i] == 'C')
							temp = 12;
						else if (lpNumber[i] == 'D')
							temp = 13;
						else if (lpNumber[i] == 'E')
							temp = 14;
						else if (lpNumber[i] == 'F')
							temp = 15;
						else
							temp = lpNumber[i] - '0';

						result = result * 16 + temp;
					}
				}
				// Decimal implementation
				else {
					for (int i = 0; i < numLength; i++) {
						result = result * 10 + (lpNumber[i] - '0');
					}
				}
			}

			return result;
		}

		LPWSTR GetCommandLineArgs()
		{
			if (!pCommandLine) {
				SIZE_T bufferSize{ 1 << 12 };
				if (NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess, reinterpret_cast<PVOID*>(&pCommandLine), 0, &bufferSize, MEM_COMMIT, PAGE_READWRITE))) {
					for (int i = 0; i < pPEB->ProcessParameters->CommandLine.Length; i++) {
						pCommandLine[i] = pPEB->ProcessParameters->CommandLine.Buffer[i];
					}
				}
			}
			return pCommandLine;
		}

		LPWSTR* ConvertCommandLineToArgV(LPWSTR pCommandLine, int* pNumArgs)
		{
			if (!pArgV && pCommandLine && pNumArgs) {
				SIZE_T bufferSize{ 1 << 12 };
				if (NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess, reinterpret_cast<PVOID*>(&pArgV), 0, &bufferSize, MEM_COMMIT, PAGE_READWRITE))) {
					iArgC = 0;
					bool bInQuotes{ false };
					int commandLineLength = wcslen(pCommandLine);
					for (int i = 0; i < commandLineLength; i++) {
						if (pCommandLine[i] == '\"') {
							if (!bInQuotes) {
								pArgV[iArgC++] = &pCommandLine[i + 1];
							}

							pCommandLine[i] = '\0';
							bInQuotes = !bInQuotes;
							continue;
						}

						if (pCommandLine[i] == ' ' && !bInQuotes && pCommandLine[i + 1] != '\0') {
							pArgV[iArgC++] = &pCommandLine[i + 1];
							pCommandLine[i] = '\0';
						}
					}
					*pNumArgs = iArgC;
				}
			}

			return pArgV;
		}
	}
}


/*********************************
* Function Implementations
*********************************/
bool Runtime::Init(PPEB peb) {
	pPEB = peb;	
	return InitConsole(peb) && InitNtdllServices();
}

bool Runtime::InitConsole(PPEB peb) {
	hStdIn = peb->ProcessParameters->StandardInput;
	hStdOut = peb->ProcessParameters->StandardOutput;
	hStdError = peb->ProcessParameters->StandardError;

	return hStdIn != INVALID_HANDLE_VALUE &&
		hStdOut != INVALID_HANDLE_VALUE &&
		hStdError != INVALID_HANDLE_VALUE;
}

bool Runtime::InitNtdllServices() {
	HMODULE hModule = GetModuleHandleW(L"ntdll");
	if (hModule != NULL) {
		Functions::sprintf_s = reinterpret_cast<PFN_SPRINTF_S>(GetProcAddress(hModule, "sprintf_s"));
		Functions::swprintf_s = reinterpret_cast<PFN_SWPRINTF_S>(GetProcAddress(hModule, "swprintf_s"));
	}

	return Functions::sprintf_s != nullptr && Functions::swprintf_s != nullptr;
}

void Runtime::Destroy()
{
	if (pArgV) {
		SIZE_T regionSize{ 0 };
		auto status = NtFreeVirtualMemory(NtCurrentProcess, reinterpret_cast<PVOID*>(&pArgV), &regionSize, MEM_RELEASE);
		if(NT_SUCCESS(status))
			pArgV = nullptr;
	}
	
	if (pCommandLine) {
		SIZE_T regionSize{ 0 };
		auto status = NtFreeVirtualMemory(NtCurrentProcess, reinterpret_cast<PVOID*>(&pCommandLine), &regionSize, MEM_RELEASE);
		if (NT_SUCCESS(status))
			pCommandLine = nullptr;
	}
}
