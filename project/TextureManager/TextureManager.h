#pragma once
#include "DirectXCommon.h"
#include "DirectXTex/DirectXTex.h"
#include "DirectXTex/d3dx12.h"
#include <iostream>
#include <string>
#include <vector>
class TextureManager {
public:
    // シングルトンインスタンスの取得
    static TextureManager* GetInstance();
    // 終了
    void Finalize();

private:
    static TextureManager* instance;

    TextureManager() = default;
    ~TextureManager() = default;
    TextureManager(TextureManager&) = delete;
    TextureManager& operator=(TextureManager&) = delete;
    // テクスチャ1枚分のデータ
    struct TextureData {
        std::string filePath;
        DirectX::TexMetadata metadata;
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
    };
    // テクスチャデータ
    std::vector<TextureData> textureDatas;

    // 最大SRV数（最大テクスチャ枚数）
    static const uint32_t kMaxSRVCount;

};
