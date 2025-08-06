#pragma once
#include <d3d12.h>
#include <wrl.h>

class DescriptorHeapWrapper {
public:
    DescriptorHeapWrapper() = default;
    ~DescriptorHeapWrapper() = default;

    void Create(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible);

    ID3D12DescriptorHeap* GetHeap() const { return heap_.Get(); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandleStart() const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleStart() const;

private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap_;
};
