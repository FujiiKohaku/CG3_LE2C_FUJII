#include "WinApp.h"
#include "imgui/imgui.h" // ImGui 本体
#include "imgui/imgui_impl_dx12.h" // DirectX12 連携
#include "imgui/imgui_impl_win32.h" // Win32 連携
#include <Windows.h>
#include <cstdint>
// #include <imgui/imgui_impl_win32.cpp>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
// ウィンドウプロシージャ
LRESULT CALLBACK WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{

    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
        return true;
    }

    // メッセージに応じて固有の処理を行う
    switch (msg) {
        // ウィンドウが破棄された
    case WM_DESTROY:
        // OSに対して、アプリの終了を伝える
        PostQuitMessage(0);
        return 0;
    }
    // 標準のメッセージ処理を行う
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

bool WinApp::ProcessMessage()
{
    MSG msg {};
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    if (msg.message == WM_QUIT) {
        return true;
    }

    return false;
}

void WinApp::initialize()
{

    CoInitializeEx(0, COINIT_MULTITHREADED);
    // 出力

    // ウィンドウプロシージャ
    wc_.lpfnWndProc = WindowProc;
    // ウィンドウクラス名(何でもよい)
    wc_.lpszClassName = L"CG2WindowClass";
    // インスタンスバンドル
    wc_.hInstance = GetModuleHandle(nullptr);
    // カーソル
    wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);
    // ウィンドウクラスを登録する
    RegisterClass(&wc_);

    // ウィンドウサイズを表す構造体体にクライアント領域を入れる
    RECT wrc = { 0, 0, kClientWidth, kClientHeight };
    // クライアント領域をもとに実際のサイズにwrcを変更してもらう
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

    // ウィンドウの生成
    hwnd_ = CreateWindow(wc_.lpszClassName, // 利用するクラス名
        L"CG2", // タイトルバーの文字(何でもよい)
        WS_OVERLAPPEDWINDOW, // よく見るウィンドウスタイル
        CW_USEDEFAULT, // 表示X座標(Windowsに任せる)
        CW_USEDEFAULT, // 表示Y座標(WindowsOSに任せる)
        wrc.right - wrc.left, // ウィンドウ横幅
        wrc.bottom - wrc.top, // ウィンドウ縦幅
        nullptr, // 親ウィンドウハンドル
        nullptr, // メニューハンドル
        wc_.hInstance, // インスタンスハンドル
        nullptr); // オプション

    // ウィンドウを表示する
    ShowWindow(hwnd_, SW_SHOW);
}

void WinApp::Finalize()
{
    CloseWindow(hwnd_);
    CoUninitialize();
}
