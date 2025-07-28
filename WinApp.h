#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
class WinApp {
public:
    WinApp();
    ~WinApp();

    // 初期化
    void Initialize(HINSTANCE hinstance, int nCmdShow, const std::wstring& title, int width, int height);

    // メインウィンドウ取得
    HWND GetHwnd() const { return hwnd_; }


      // ウィンドウプロシージャ
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);



    private:
    HWND hwnd_ = nullptr;
    HINSTANCE hInstance_ = nullptr;
    std::wstring className_ = L"CG2WindowClass";


};
