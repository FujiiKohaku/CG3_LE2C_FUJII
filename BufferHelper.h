#pragma once
// Windows & DirectX
#include "Utility.h"
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
// 追加ヘッダー（DirectXTexやd3dx12）
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

// === バッファ作成（UPLOADヒープ） ===
Microsoft::WRL::ComPtr<ID3D12Resource>
CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

// === ディスクリプタヒープ作成 ===
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>
CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device,
    D3D12_DESCRIPTOR_HEAP_TYPE heapType,
    UINT numDescriptors,
    bool shaderVisible);

// === テクスチャリソース作成（DEFAULTヒープ） ===
Microsoft::WRL::ComPtr<ID3D12Resource>
CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device,
    const DirectX::TexMetadata& metadata);

Microsoft::WRL::ComPtr<ID3D12Resource>
UploadTextureData(
    Microsoft::WRL::ComPtr<ID3D12Resource> texture,
    const DirectX::ScratchImage& mipImages,
    Microsoft::WRL::ComPtr<ID3D12Device> device,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);

DirectX::ScratchImage LoadTexture(const std::string& filePath);

Microsoft::WRL::ComPtr<ID3D12Resource>
CreateDepthStencilTextureResource(
    Microsoft::WRL::ComPtr<ID3D12Device> device,
    int32_t width,
    int32_t height);