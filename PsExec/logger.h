#ifndef LOGGER_H
#define LOGGER_H

#include "Runtime.h"

#define MESSAGE_SIZE 1024

using namespace Runtime;
using namespace Runtime::Functions;

namespace Logger {
    extern LPWSTR g_lpszPrintBuffer;
    extern LPWSTR g_lpszLogBuffer;

    bool Init();
    void Destroy();

    template<typename... Args>
    void Print(
        const wchar_t* format,
        Args... args);

    template<typename... Args>
    void Log(
        const wchar_t* filename,
        int             lineNumber,
        const wchar_t* function,
        const wchar_t* logLevel,
        const wchar_t* format,
        Args...         args);
}

template<typename... Args>
void Logger::Print(
    const wchar_t* format,
    Args... args)
{
    SetMemory(g_lpszPrintBuffer, 0, MESSAGE_SIZE * sizeof(wchar_t));
    swprintf_s(g_lpszPrintBuffer, MESSAGE_SIZE, format, args...);
    WriteConsoleW(hStdOut, g_lpszPrintBuffer, WcStringLength(g_lpszPrintBuffer), nullptr, nullptr);
}

template<typename... Args>
void Logger::Log(
    const wchar_t* filename,
    int            lineNumber,
    const wchar_t* function,
    const wchar_t* logLevel,
    const wchar_t* format,
    Args...        args)
{

    SetMemory(g_lpszLogBuffer, 0, MESSAGE_SIZE * sizeof(wchar_t));
    int returnCount = swprintf_s(g_lpszLogBuffer, MESSAGE_SIZE, L"[%s:%d (%s)][%s] ", filename, lineNumber, function, logLevel);
    swprintf_s(&g_lpszLogBuffer[returnCount], MESSAGE_SIZE - returnCount, format, args...);
    WriteConsoleW(hStdOut, g_lpszLogBuffer, WcStringLength(g_lpszLogBuffer), nullptr, nullptr);
}

#define LOG_DEBUG(msg, ...)     Logger::Log(TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), TEXT("DEBUG"), msg, __VA_ARGS__)
#define LOG_INFO(msg, ...)      Logger::Log(TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), TEXT("INFO"),  msg, __VA_ARGS__)
#define LOG_WARN(msg, ...)      Logger::Log(TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), TEXT("WARN"),  msg, __VA_ARGS__)
#define LOG_ERROR(msg, ...)     Logger::Log(TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), TEXT("ERROR"), msg, __VA_ARGS__)

#endif
