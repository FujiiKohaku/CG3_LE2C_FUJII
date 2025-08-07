#include "Texture.h"
#include "BufferHelper.h" // CreateTextureResource / UploadTextureData を使う

void Texture::LoadFromFile(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::string& filePath)
{
    // --- ファイルからテクスチャ読み込み ---
    DirectX::ScratchImage mipImages = LoadTexture(filePath);

    // --- メタデータを保存（Format, サイズ, MipLevels など）---
    metadata_ = mipImages.GetMetadata();

    // --- テクスチャリソース生成 ---
    textureResource_ = CreateTextureResource(device, metadata_);

    // --- GPUに転送するための中間リソースを作ってアップロード ---
    intermediateResource_ = UploadTextureData(textureResource_.Get(), mipImages, device, cmdList);
}

void Texture::CreateSRV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc {};
    srvDesc.Format = metadata_.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata_.mipLevels);

    device->CreateShaderResourceView(textureResource_.Get(), &srvDesc, cpuHandle);

 
    gpuHandle_ = gpuHandle;
}
