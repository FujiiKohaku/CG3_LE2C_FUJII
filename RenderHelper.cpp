#include "RenderHelper.h"
#include "DeviceManager.h"

RenderHelper::RenderHelper(DeviceManager& deviceManager)
    : deviceManager(deviceManager)
{
}

void RenderHelper::PreDraw(const float clearColor[4],
    ID3D12DescriptorHeap* dsvHeap,
    const D3D12_VIEWPORT& viewport,
    const D3D12_RECT& scissorRect,
    ID3D12RootSignature* rootSignature,
    ID3D12PipelineState* pipelineState,
    ID3D12DescriptorHeap* srvHeap)
{
    auto cmd = deviceManager.GetCommandList();

    // バックバッファと深度のクリア
    deviceManager.ClearBackBuffer(dsvHeap->GetCPUDescriptorHandleForHeapStart(), clearColor);
    cmd->ClearDepthStencilView(dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // ビューポートとシザー
    cmd->RSSetViewports(1, &viewport);
    cmd->RSSetScissorRects(1, &scissorRect);

    // ルートシグネチャとパイプライン設定
    cmd->SetGraphicsRootSignature(rootSignature);
    cmd->SetPipelineState(pipelineState);
    cmd->SetDescriptorHeaps(1, &srvHeap);
}

void RenderHelper::DrawModel(const D3D12_VERTEX_BUFFER_VIEW& vbView,
    UINT vertexCount,
    D3D12_GPU_VIRTUAL_ADDRESS wvp,
    D3D12_GPU_VIRTUAL_ADDRESS material,
    D3D12_GPU_VIRTUAL_ADDRESS light,
    D3D12_GPU_DESCRIPTOR_HANDLE texHandle)
{
    auto cmd = deviceManager.GetCommandList();

    cmd->SetGraphicsRootConstantBufferView(1, wvp);
    cmd->SetGraphicsRootConstantBufferView(0, material);
    cmd->SetGraphicsRootConstantBufferView(3, light);
    cmd->SetGraphicsRootDescriptorTable(2, texHandle);
    cmd->IASetVertexBuffers(0, 1, &vbView);
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmd->DrawInstanced(vertexCount, 1, 0, 0);
}

void RenderHelper::DrawSprite(const D3D12_VERTEX_BUFFER_VIEW& vbView,
    const D3D12_INDEX_BUFFER_VIEW& ibView,
    D3D12_GPU_VIRTUAL_ADDRESS transform,
    D3D12_GPU_VIRTUAL_ADDRESS material,
    D3D12_GPU_DESCRIPTOR_HANDLE texHandle)
{
    auto cmd = deviceManager.GetCommandList();

    cmd->SetGraphicsRootConstantBufferView(1, transform);
    cmd->SetGraphicsRootConstantBufferView(0, material);
    cmd->SetGraphicsRootDescriptorTable(2, texHandle);
    cmd->IASetVertexBuffers(0, 1, &vbView);
    cmd->IASetIndexBuffer(&ibView);
    cmd->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void RenderHelper::PostDraw(ID3D12Fence* fence, HANDLE fenceEvent, UINT64& fenceValue)
{
    deviceManager.GetTempBarrier().Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    deviceManager.GetTempBarrier().Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    deviceManager.GetCommandList()->ResourceBarrier(1, &deviceManager.GetTempBarrier());
    deviceManager.ExecuteCommandListAndPresent(fence, fenceEvent, fenceValue);
}
