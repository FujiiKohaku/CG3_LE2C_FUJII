#pragma once

#include <d3d12.h>
#include <wrl.h>

class DeviceManager;

class RenderHelper {
public:
    RenderHelper(DeviceManager& deviceManager);

    void PreDraw(const float clearColor[4],
        ID3D12DescriptorHeap* dsvHeap,
        const D3D12_VIEWPORT& viewport,
        const D3D12_RECT& scissorRect,
        ID3D12RootSignature* rootSignature,
        ID3D12PipelineState* pipelineState,
        ID3D12DescriptorHeap* srvHeap);

    void DrawModel(const D3D12_VERTEX_BUFFER_VIEW& vbView,
        UINT vertexCount,
        D3D12_GPU_VIRTUAL_ADDRESS wvp,
        D3D12_GPU_VIRTUAL_ADDRESS material,
        D3D12_GPU_VIRTUAL_ADDRESS light,
        D3D12_GPU_DESCRIPTOR_HANDLE texHandle);

    void DrawSprite(const D3D12_VERTEX_BUFFER_VIEW& vbView,
        const D3D12_INDEX_BUFFER_VIEW& ibView,
        D3D12_GPU_VIRTUAL_ADDRESS transform,
        D3D12_GPU_VIRTUAL_ADDRESS material,
        D3D12_GPU_DESCRIPTOR_HANDLE texHandle);

    void PostDraw(ID3D12Fence* fence, HANDLE fenceEvent, UINT64& fenceValue);

private:
    DeviceManager& deviceManager;
};
