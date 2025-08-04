#pragma once
#include "DeviceManager.h"
#include "ViewportManager.h"
#include <d3d12.h>

class Renderer {
public:
    void Initialize(
        DeviceManager* deviceManager,
        ViewportManager* viewportManager,
        ID3D12RootSignature* rootSignature,
        ID3D12PipelineState* pipelineState,
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle,
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle);

    void PreDraw();
    void PostDraw(ID3D12Fence* fence, HANDLE fenceEvent, UINT64& fenceValue);

private:
    DeviceManager* deviceManager = nullptr;
    ViewportManager* viewportManager = nullptr;
    ID3D12RootSignature* rootSignature = nullptr;
    ID3D12PipelineState* pipelineState = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle {};
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle {};
};
