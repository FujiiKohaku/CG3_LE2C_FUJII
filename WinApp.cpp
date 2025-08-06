#include "WinApp.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

// ImGuiのWndProc用関数
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

WinApp::WinApp() { }

WinApp::~WinApp()
{
    if (hwnd_) {
        DestroyWindow(hwnd_);
    }
    UnregisterClass(className_.c_str(), hInstance_);
}

void WinApp::Initialize(HINSTANCE hinstance, int nCmdShow, const std::wstring& title, int width, int height)
{
    hInstance_ = hinstance;

    // ウィンドウクラス登録
    WNDCLASS wc {};
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = className_.c_str();
    wc.hInstance = hInstance_;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClass(&wc);

    RECT wrc = { 0, 0, width, height };
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

    hwnd_ = CreateWindow(
        className_.c_str(),
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wrc.right - wrc.left,
        wrc.bottom - wrc.top,
        nullptr, nullptr, hInstance_, nullptr);

    ShowWindow(hwnd_, nCmdShow);
}

LRESULT CALLBACK WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
        return true;
    }

    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

// ==================== ImGui 処理 ====================

void WinApp::ImGuiInitialize(ID3D12Device* device, DXGI_FORMAT rtvFormat, ID3D12DescriptorHeap* srvHeap, int bufferCount)
{
    imguiSrvHeap_ = srvHeap;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsClassic();
    ImGui_ImplWin32_Init(hwnd_);
    ImGui_ImplDX12_Init(
        device,
        bufferCount,
        rtvFormat,
        srvHeap,
        srvHeap->GetCPUDescriptorHandleForHeapStart(),
        srvHeap->GetGPUDescriptorHandleForHeapStart());
}

void WinApp::ImGuiBeginFrame()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void WinApp::ImGuiEndFrame(ID3D12GraphicsCommandList* commandList)
{
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

void WinApp::ImGuiShutdown()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
