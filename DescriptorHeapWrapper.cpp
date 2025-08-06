#include "DescriptorHeapWrapper.h"
#include "externals/DirectXTex/d3dx12.h" // D3D12_DESCRIPTOR_HEAP_DESC ‚Å•Ö—˜

void DescriptorHeapWrapper::Create(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible)
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = type;
    desc.NumDescriptors = numDescriptors;
    desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;

    device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap_));
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapWrapper::GetCPUHandleStart() const
{
    return heap_->GetCPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapWrapper::GetGPUHandleStart() const
{
    return heap_->GetGPUDescriptorHandleForHeapStart();
}
