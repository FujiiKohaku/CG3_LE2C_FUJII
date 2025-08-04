#include "Render.h"

void Renderer::Initialize(
    DeviceManager* deviceManager_,
    ViewportManager* viewportManager_,
    ID3D12RootSignature* rootSignature_,
    ID3D12PipelineState* pipelineState_,
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_,
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_)
{
    deviceManager = deviceManager_;
    viewportManager = viewportManager_;
    rootSignature = rootSignature_;
    pipelineState = pipelineState_;
    rtvHandle = rtvHandle_;
    dsvHandle = dsvHandle_;
}

void Renderer::PreDraw()
{
    float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
    deviceManager->ClearBackBuffer(dsvHandle, clearColor);
    deviceManager->GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    deviceManager->GetCommandList()->RSSetViewports(1, &viewportManager->GetViewport());
    deviceManager->GetCommandList()->RSSetScissorRects(1, &viewportManager->GetScissorRect());

    deviceManager->GetCommandList()->SetGraphicsRootSignature(rootSignature);
    deviceManager->GetCommandList()->SetPipelineState(pipelineState);
}

void Renderer::PostDraw(ID3D12Fence* fence, HANDLE fenceEvent, UINT64& fenceValue)
{
    auto& barrier = deviceManager->GetTempBarrier();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    deviceManager->GetCommandList()->ResourceBarrier(1, &barrier);

    deviceManager->ExecuteCommandListAndPresent(fence, fenceEvent, fenceValue);
}
