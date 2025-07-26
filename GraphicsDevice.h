#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <format>
#include <wrl.h>
class GraphicsDevice {
public:
    void Initialize(HWND hwnd, int width, int height);
    void BeginFrame();
    void EndFrame();
    void Present();

    ID3D12Device* GetDevice() const { return device_.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }

private:
    void CreateDevice();
    void CreateCommandObjects();
    void CreateSwapChain(HWND hwnd, int width, int height);
    void CreateDescriptorHeaps();
    void CreateRenderTargetViews();

private:
    // デバイス本体
    Microsoft::WRL::ComPtr<ID3D12Device> device_;
   
    // コマンド関係
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

    // スワップチェーン
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
    Microsoft::WRL::ComPtr<ID3D12Resource> renderTargets_[2];
    UINT backBufferIndex_ = 0;

    // フェンス
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    UINT64 fenceValue_ = 0;
    HANDLE fenceEvent_ = nullptr;

    // RTV
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
    UINT rtvDescriptorSize_ = 0;

    // DSV
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;
    UINT dsvDescriptorSize_ = 0;

    // SRV
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
    UINT srvDescriptorSize_ = 0;
};
