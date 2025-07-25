#pragma once
#include "Utility.h"
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <format>
#include <wrl.h>
class GraphicsDevice {
public:
    void Initialize(HWND hwnd, uint32_t width, uint32_t height);
    void BeginFrame(); // コマンドの記録開始
    void EndFrame(); // コマンドの記録終了とキック
    // ゲッター☆
    ID3D12Device* GetDevice() const { return device_.Get(); }
    ID3D12CommandQueue* GetCommandQueue() const { return commandQueue_.Get(); }
    ID3D12CommandAllocator* GetCommandAllocator() const { return commandAllocator_.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }
    IDXGISwapChain4* GetSwapChain() const { return swapChain_.Get(); }
    ID3D12Resource* GetBackBuffer(uint32_t index) const { return swapChainResources_[index].Get(); }

private:
    static const uint32_t kNumBackBuffers = 2;
    UINT backBufferIndex_ = 0; // 現在のバックバッファ番号（SwapChainから取得）

    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_; // DXGIファクトリー
    Microsoft::WRL::ComPtr<ID3D12Device> device_; // D3D12デバイス
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_; // コマンドキュー
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_; // コマンドアロケーター
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_; // コマンドリスト
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_; // スワップチェイン
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_; // RTV用ヒープ
    Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[kNumBackBuffers]; // バックバッファ(スワップチェインリソース)
};
