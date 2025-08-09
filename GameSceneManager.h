#pragma once
#include "DescriptorHeapWrapper.h"
#include "DeviceManager.h"
#include "DirectionalLightBuffer.h"
#include "Dxc.h"
#include "Logger.h"
#include "WinApp.h"
#include "CommonStructs.h"
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
};
