#include "IndexBuffer.h"
#include "BufferHelper.h" // CreateBufferResource を使うため
#include <cassert>
#include <cstring>

void IndexBuffer::Initialize(ID3D12Device* device, const std::vector<uint32_t>& indices)
{
    assert(!indices.empty());

    const UINT sizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices.size());

    // インデックスバッファ用のリソースを作成
    indexResource_ = CreateBufferResource(device, sizeInBytes);

    // インデックスバッファビューを設定
    indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes = sizeInBytes;
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

    // データを書き込む
    uint32_t* mappedData = nullptr;
    HRESULT hr = indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
    assert(SUCCEEDED(hr));
    std::memcpy(mappedData, indices.data(), sizeInBytes);
    indexResource_->Unmap(0, nullptr);
}
