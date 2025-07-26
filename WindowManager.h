#pragma once
#include <Windows.h>
#include <cstdint>

class WindowManager {
public:
    bool Initialize(const wchar_t* className, const wchar_t* windowTitle, int32_t width, int32_t height);
    HWND GetHwnd() const { return hwnd_; }

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
    HWND hwnd_ = nullptr;
    const wchar_t* className_ = L"";
};
