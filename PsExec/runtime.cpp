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
		PFN_SPRINTF_S	sprintf_s = nullptr;
		PFN_SWPRINTF_S	swprintf_s = nullptr;
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

	return hStdIn != INVALID_HANDLE_VALUE
		&& hStdOut != INVALID_HANDLE_VALUE
		&& hStdError != INVALID_HANDLE_VALUE;
}

bool Runtime::InitNtdllServices() {
	HMODULE hModule = GetModuleHandleW(L"ntdll");
	if (hModule != NULL) {
		Functions::sprintf_s = reinterpret_cast<PFN_SPRINTF_S>(GetProcAddress(hModule, "sprintf_s"));
		Functions::swprintf_s = reinterpret_cast<PFN_SWPRINTF_S>(GetProcAddress(hModule, "swprintf_s"));
	}

	return Functions::sprintf_s != nullptr
		&& Functions::swprintf_s != nullptr;
}

void Runtime::Destroy()
{
	if (pArgV) {
		SIZE_T regionSize{ 0 };
		auto status = NtFreeVirtualMemory(NtCurrentProcess, reinterpret_cast<PVOID*>(&pArgV), &regionSize, MEM_RELEASE);
		if (NT_SUCCESS(status)) {
			pArgV = nullptr;
			iArgC = 0;
		}
	}

	if (pCommandLine) {
		SIZE_T regionSize{ 0 };
		auto status = NtFreeVirtualMemory(NtCurrentProcess, reinterpret_cast<PVOID*>(&pCommandLine), &regionSize, MEM_RELEASE);
		if (NT_SUCCESS(status))
			pCommandLine = nullptr;
	}
}

int Runtime::Functions::WcStringLength(const wchar_t* str)
{
	int length = 0;
	while (str && str[length])
		length++;

	return length;
}

int Runtime::Functions::WcStringCompare(const wchar_t* str1, const wchar_t* str2)
{
	wchar_t c1, c2;
	do {
		c1 = *str1++;
		c2 = *str2++;
		if (c2 == '\0')
			return c1 - c2;

	} while (c1 == c2);

	return c1 < c2 ? -1 : 1;
}

int Runtime::Functions::WcStringToInt(const wchar_t* str)
{
	int result = 0;
	if (str) {
		int numLength = WcStringLength(str);
		if (numLength > 2 && (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))) {
			int temp = 0;
			for (int i = 2; i < numLength; i++) {
				if (str[i] >= 'A' && str[i] <= 'F')
					temp = str[i] - 'A';
				else if (str[i] >= 'a' && str[i] <= 'f')
					temp = str[i] - 'a';
				else
					temp = str[i] - '0';

				result = result * 16 + temp;
			}
		}
		else {
			for (int i = 0; i < numLength; i++) {
				result = result * 10 + (str[i] - '0');
			}
		}
	}

	return result;
}

void* Runtime::Functions::SetMemory(void* dest, int val, size_t len)
{
	unsigned char* ptr = reinterpret_cast<unsigned char*>(dest);
	while (len-- > 0)
		*ptr++ = val;

	return nullptr;
}

LPWSTR Runtime::Functions::GetCommandLineArgs()
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

LPWSTR* Runtime::Functions::ConvertCommandLineToArgV(LPWSTR pCommandLine, int* pNumArgs)
{
	if (!pArgV && pCommandLine && pNumArgs) {
		SIZE_T bufferSize{ 1 << 12 };
		if (NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess, reinterpret_cast<PVOID*>(&pArgV), 0, &bufferSize, MEM_COMMIT, PAGE_READWRITE))) {
			bool bInQuotes{ false };
			int commandLineLength = WcStringLength(pCommandLine);
			iArgC = 0;

			for (int i = 0; i < commandLineLength; i++) {
				if (pCommandLine[i] == '\"') {
					if (!bInQuotes)
						pArgV[iArgC++] = &pCommandLine[i];

					bInQuotes = !bInQuotes;
					continue;
				}

				if (!bInQuotes && pCommandLine[i] == ' ' && pCommandLine[i + 1] != '\0') {
					if (pCommandLine[i + 1] != ' ')
						pArgV[iArgC++] = &pCommandLine[i + 1];

					pCommandLine[i] = '\0';
					continue;
				}

				if (i == 0) {
					pArgV[iArgC++] = pCommandLine;
				}
			}

			*pNumArgs = iArgC;
		}
	}

	return pArgV;
}
