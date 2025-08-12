#pragma once
#include <Windows.h>

class CrashDump {
public:
    // SetUnhandledExceptionFilter へ渡す関数ポインタ用
    static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);
};
