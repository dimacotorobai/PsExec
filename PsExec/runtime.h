#ifndef RUNTIME_H
#define RUNTIME_H

#include "ntdll.h"

typedef int(__cdecl* PFN_SPRINTF_S)(char* buffer, size_t sizeOfBuffer, const char* format, ...);
typedef int(__cdecl* PFN_SWPRINTF_S)(wchar_t* buffer, size_t sizeOfBuffer, const wchar_t* format, ...);


namespace Runtime {
	extern PPEB pPEB;

	extern HANDLE hStdIn;
	extern HANDLE hStdOut;
	extern HANDLE hStdError;

	extern LPWSTR pCommandLine;
	extern LPWSTR* pArgV;
	extern INT iArgC;

	namespace Functions {
		extern PFN_SPRINTF_S sprintf_s;
		extern PFN_SWPRINTF_S swprintf_s;


		int WcStringLength(const wchar_t* str);
		int WcStringCompare(const wchar_t* str1, const wchar_t* str2);
		int WcStringToInt(const wchar_t* str);
		void* SetMemory(void* dest, int c, size_t count);

		LPWSTR GetCommandLineArgs();
		LPWSTR* ConvertCommandLineToArgV(LPWSTR pCommandLine, int* pNumArgs);
	}

	bool Init(PPEB peb);
	bool InitConsole(PPEB peb);
	bool InitNtdllServices();
	void Destroy();
}

#endif