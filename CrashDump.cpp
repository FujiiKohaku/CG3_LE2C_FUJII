#include "CrashDump.h"

#include <dbghelp.h>
#include <strsafe.h>
#pragma comment(lib, "dbghelp.lib")

LONG WINAPI CrashDump::ExportDump(EXCEPTION_POINTERS* exception)
{
    // 時刻→ファイル名
    SYSTEMTIME time;
    GetLocalTime(&time);

    wchar_t filePath[MAX_PATH] = { 0 };
    CreateDirectory(L"./Dumps", nullptr);
    StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp",
        time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);

    // ダンプファイルを作成
    HANDLE dumpFileHandle = CreateFile(
        filePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        nullptr,
        CREATE_ALWAYS,
        0,
        nullptr);

    // 必要情報
    DWORD processId = GetCurrentProcessId();
    DWORD threadId = GetCurrentThreadId();

    MINIDUMP_EXCEPTION_INFORMATION info {};
    info.ThreadId = threadId;
    info.ExceptionPointers = exception;
    info.ClientPointers = TRUE;

    // 最小限のダンプを出力
    MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle,
        MiniDumpNormal, &info, nullptr, nullptr);

    return EXCEPTION_EXECUTE_HANDLER;
}
