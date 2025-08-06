#pragma once
#include <cstdint>
#include <d3d12.h>
#include <vector>
#include <wrl.h>

class IndexBuffer {
public:
    void Initialize(ID3D12Device* device, const std::vector<uint32_t>& indices);
    const D3D12_INDEX_BUFFER_VIEW& GetView() const { return indexBufferView_; }
    ID3D12Resource* GetResource() const { return indexResource_.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
    D3D12_INDEX_BUFFER_VIEW indexBufferView_ {};
};
