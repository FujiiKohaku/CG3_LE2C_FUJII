#include "VertexBuffer.h"
#include <cassert>
#include <cstring> // std::memcpy

void VertexBuffer::Initialize(ID3D12Device* device, const std::vector<VertexData>& vertices)
{
    assert(!vertices.empty());

    const UINT sizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices.size());
    strideInBytes_ = sizeof(VertexData);

    // 頂点バッファ用リソース作成
    vertexResource_ = CreateBufferResource(device, sizeInBytes);

    // 頂点バッファビュー設定
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeInBytes;
    vertexBufferView_.StrideInBytes = strideInBytes_;

    // データ転送
    VertexData* mappedData = nullptr;
    HRESULT hr = vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
    assert(SUCCEEDED(hr));
    std::memcpy(mappedData, vertices.data(), sizeInBytes);
    vertexResource_->Unmap(0, nullptr);
}
