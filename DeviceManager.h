#pragma once
#include "BufferHelper.h"
#include "Utility.h"
#include "WinApp.h"
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <format>
#include <wrl.h>
// リンク
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class DeviceManager {
public:
    void Initialize(std::ofstream& logStream, WinApp* winApp, uint32_t width, uint32_t height);

    ID3D12Device* GetDevice() const { return device_.Get(); }
    IDXGIAdapter4* GetAdapter() const { return useAdapter_.Get(); }
    IDXGIFactory7* GetFactory() const { return dxgiFactory_.Get(); }
    ID3D12CommandQueue* GetCommandQueue() const { return commandQueue_.Get(); }
    ID3D12CommandAllocator* GetCommandAllocator() const { return commandAllocator_.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }
    IDXGISwapChain4* GetSwapChain() const { return swapChain_.Get(); }
    const DXGI_SWAP_CHAIN_DESC1& GetSwapChainDesc() const { return swapChainDesc_; }
    ID3D12DescriptorHeap* GetRTVDescriptorHeap() const { return rtvDescriptorHeap_.Get(); }

private:
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
    Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_;
    Microsoft::WRL::ComPtr<ID3D12Device> device_;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc_ {};
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc_ {};
};
