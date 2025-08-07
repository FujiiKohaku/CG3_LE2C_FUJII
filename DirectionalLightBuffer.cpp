#include "DirectionalLightBuffer.h"
#include <cassert>

void DirectionalLightBuffer::Initialize(ID3D12Device* device)
{
    // 定数バッファを作成
    resource_ = CreateBufferResource(device, sizeof(DirectionalLight));
    // 書き込み用アドレス取得
    HRESULT hr = resource_->Map(0, nullptr, reinterpret_cast<void**>(&lightData_));
    assert(SUCCEEDED(hr));

    // 初期値設定
    lightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白色
    lightData_->direction = MatrixMath::Normalize({ 0.0f, -1.0f, 0.0f }); // 真上から下方向
    lightData_->intensity = 1.0f; // 明るさ
}

void DirectionalLightBuffer::Update()
{
    // 今のところ何もしない（ImGuiで直接lightData_を書き換える想定）
    // 必要ならここで加工・制限・補正もできる
}

D3D12_GPU_VIRTUAL_ADDRESS DirectionalLightBuffer::GetGPUVirtualAddress() const
{
    return resource_->GetGPUVirtualAddress();
}
