#include "runtime.h"

/*********************************
* Declare global variables
*********************************/
namespace Runtime {
	HANDLE hStdIn		= INVALID_HANDLE_VALUE;
	HANDLE hStdOut		= INVALID_HANDLE_VALUE;
	HANDLE hStdError	= INVALID_HANDLE_VALUE;
	
	namespace Functions {
		PFN_SPRINTF_S sprintf_s		= nullptr;
		PFN_SWPRINTF_S	swprintf_s	= nullptr;
	}
}


/*********************************
* Function Implementations
*********************************/
bool Runtime::Init() {
	return InitConsole() && InitNtdllServices();
}

bool Runtime::InitConsole() {
	hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	hStdError = GetStdHandle(STD_ERROR_HANDLE);

	return hStdIn != INVALID_HANDLE_VALUE &&
		hStdOut != INVALID_HANDLE_VALUE &&
		hStdError != INVALID_HANDLE_VALUE;
}

bool Runtime::InitNtdllServices() {
	HMODULE hModule = GetModuleHandle(TEXT("ntdll"));
	if (hModule != NULL) {
		Functions::sprintf_s = reinterpret_cast<PFN_SPRINTF_S>(GetProcAddress(hModule, "sprintf_s"));
		Functions::swprintf_s = reinterpret_cast<PFN_SWPRINTF_S>(GetProcAddress(hModule, "swprintf_s"));
	}

	return Functions::sprintf_s != nullptr && Functions::swprintf_s != nullptr;
}