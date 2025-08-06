#pragma once
#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <string>

class WinApp {
public:
    WinApp();
    ~WinApp();

    // ウィンドウ初期化
    void Initialize(HINSTANCE hinstance, int nCmdShow, const std::wstring& title, int width, int height);

    // メインウィンドウハンドル取得
    HWND GetHwnd() const { return hwnd_; }

    // ウィンドウプロシージャ
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    // ImGui 関連
    void ImGuiInitialize(ID3D12Device* device, DXGI_FORMAT rtvFormat, ID3D12DescriptorHeap* srvHeap, int bufferCount);
    void ImGuiBeginFrame();
    void ImGuiEndFrame(ID3D12GraphicsCommandList* commandList);
    void ImGuiShutdown();

private:
    HWND hwnd_ = nullptr;
    HINSTANCE hInstance_ = nullptr;
    std::wstring className_ = L"CG2WindowClass";

    ID3D12DescriptorHeap* imguiSrvHeap_ = nullptr;
};
