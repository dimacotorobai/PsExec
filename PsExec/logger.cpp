#include "logger.h"
#include "runtime.h"


namespace Logger {
    LPWSTR g_lpszPrintBuffer = nullptr;
    LPWSTR g_lpszLogBuffer = nullptr;
}

bool Logger::Init()
{
    g_lpszPrintBuffer = static_cast<LPWSTR>(LocalAlloc(LPTR, MESSAGE_SIZE * sizeof(wchar_t)));
    g_lpszLogBuffer = static_cast<LPWSTR>(LocalAlloc(LPTR, MESSAGE_SIZE * sizeof(wchar_t)));
    return g_lpszPrintBuffer != nullptr
        && g_lpszLogBuffer != nullptr;
}

void Logger::Destroy()
{
    LocalFree(g_lpszPrintBuffer);
    LocalFree(g_lpszLogBuffer);
}
