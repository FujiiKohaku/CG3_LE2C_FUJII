#pragma once
#include "BufferHelper.h" // CreateBufferResource関数を含む
#include "CommonStructs.h"
#include <d3d12.h>
#include <vector>
#include <wrl.h>

class VertexBuffer {
public:
    VertexBuffer() = default;
    ~VertexBuffer() = default;

    void Initialize(ID3D12Device* device, const std::vector<VertexData>& vertices);

    const D3D12_VERTEX_BUFFER_VIEW& GetView() const { return vertexBufferView_; }
    ID3D12Resource* GetResource() const { return vertexResource_.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ {};
    UINT strideInBytes_ = 0;
};
