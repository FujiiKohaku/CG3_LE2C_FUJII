#pragma once
#include <Windows.h>
#include <cstdint>
// WindowsAPI
class WinApp {

public:
    // 初期化
    void initialize();

    // 終了
    void Finalize();

    // クライアント領域のサイズ
    static const int32_t kClientWidth = 1280;
    static const int32_t kClientHeight = 720;
    // 静的メンバ関数
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    // ウィンドウハンドルのgetter
    HWND GetHwnd() const { return hwnd_; }
    // ウィンドウクラスのgetter
    HINSTANCE GetHinstance() const { return wc_.hInstance; }
    //メッセージの処理
    bool ProcessMessage();

private:
    // ウィンドウハンドル
    HWND hwnd_ = nullptr;
    // ウィンドウクラスの設定
    WNDCLASS wc_ {};
};
