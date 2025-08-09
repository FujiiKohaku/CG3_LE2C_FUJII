#pragma once
#include "CommonStructs.h"
#include "DescriptorHeapWrapper.h"
#include "DeviceManager.h"
#include "DirectionalLightBuffer.h"
#include "Dxc.h"
#include "Logger.h"
#include "WinApp.h"
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>

class GameSceneManager {
public:
    void Initialize(HINSTANCE hInst, int nCmdShow,
        const wchar_t* title, uint32_t width, uint32_t height);

    WinApp& GetWinApp() { return win_; }
    DeviceManager& GetDeviceManager() { return deviceManager_; }
    Logger& GetLogger() { return log_; }
    // 第2陣
    DescriptorHeapWrapper& GetDSVHeap() { return dsvHeap_; }
    DescriptorHeapWrapper& GetSRVHeap() { return srvHeap_; }
    ID3D12Fence* GetFence() { return fence_.Get(); }
    HANDLE GetFenceEvent() { return fenceEvent_; }
    uint64_t& GetFenceValue() { return fenceValue_; }

    // 追加：描画で使うやつ
    ID3D12RootSignature* GetRootSignature() { return rootSignature_.Get(); }
    ID3D12PipelineState* GetPipelineState() { return pipelineState_.Get(); }
    // 追加第3陣
    uint32_t GetDescriptorSizeSRV() const { return descriptorSizeSRV_; }
    uint32_t GetDescriptorSizeRTV() const { return descriptorSizeRTV_; }
    uint32_t GetDescriptorSizeDSV() const { return descriptorSizeDSV_; }

    // SRV/DSV のハンドルを index 指定で取得（便利版）
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUHandle(uint32_t index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUHandle(uint32_t index) const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUHandle(uint32_t index) const;

    // ルートシグネチャ/PSO を外から使えるように
    ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
    ID3D12PipelineState* GetPipelineState() const { return pipelineState_.Get(); }

private:
    Logger log_;
    WinApp win_;
    DeviceManager deviceManager_;
    // 第2陣
    DescriptorHeapWrapper dsvHeap_;
    DescriptorHeapWrapper srvHeap_;
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencillResource_;
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    uint64_t fenceValue_ = 0;
    HANDLE fenceEvent_ = nullptr;

    // 追加：DXC / ルートシグネチャ / PSO
    Dxc dxc_;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

    // 追加第3陣

    uint32_t descriptorSizeSRV_ = 0;
    uint32_t descriptorSizeRTV_ = 0;
    uint32_t descriptorSizeDSV_ = 0;
};
