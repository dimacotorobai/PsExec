#ifndef RUNTIME_H
#define RUNTIME_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winternl.h>

typedef int(__cdecl* PFN_SPRINTF_S)(char* buffer, size_t sizeOfBuffer, const char* format, ...);
typedef int(__cdecl* PFN_SWPRINTF_S)(wchar_t* buffer, size_t sizeOfBuffer, const wchar_t* format, ...);


namespace Runtime {
	extern HANDLE hStdIn;
	extern HANDLE hStdOut;
	extern HANDLE hStdError;

	namespace Functions {
		extern PFN_SPRINTF_S sprintf_s;
		extern PFN_SWPRINTF_S swprintf_s;
	}

	bool Init();
	bool InitConsole();
	bool InitNtdllServices();
}

#endif