#pragma once
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"
#include <Windows.h>
#include <d3d12.h>
#include <string>
#include <wrl.h>

class Texture {
public:
    void LoadFromFile(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::string& filePath);
    void CreateSRV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

    const DirectX::TexMetadata& GetMetadata() const { return metadata_; }
    ID3D12Resource* GetResource() const { return textureResource_.Get(); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return gpuHandle_; }


private:
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_;
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource_;
    DirectX::TexMetadata metadata_;

    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle_ {}; 
};
