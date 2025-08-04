// WVPBuffer.h
#pragma once
#include "BufferHelper.h"
#include "CommonStructs.h" // TransformationMatrix 構造体の定義
#include "MatrixMath.h"
#include <d3d12.h>
#include <wrl.h>

class WVPBuffer {
public:
    void Create(ID3D12Device* device);
    void Update(const Matrix4x4& wvp, const Matrix4x4& world);

    ID3D12Resource* GetResource() const { return wvpResource.Get(); }
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return wvpResource->GetGPUVirtualAddress(); }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
    TransformationMatrix* wvpData = nullptr;
};
