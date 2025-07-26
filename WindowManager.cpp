#include "WindowManager.h"
#include "externals/imgui/imgui_impl_win32.h"

bool WindowManager::Initialize(const wchar_t* className, const wchar_t* windowTitle, int32_t width, int32_t height)
{
    className_ = className;

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = className_;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        return false;
    }

    RECT wrc = { 0, 0, width, height };
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

    hwnd_ = CreateWindow(
        className_,
        windowTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wrc.right - wrc.left,
        wrc.bottom - wrc.top,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr);

    if (!hwnd_) {
        return false;
    }

    ShowWindow(hwnd_, SW_SHOW);
    return true;
}

LRESULT CALLBACK WindowManager::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    // ① ImGuiにメッセージを処理させる（←これが重要！）
    /*if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
    //    return true;
    //}*/
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}
