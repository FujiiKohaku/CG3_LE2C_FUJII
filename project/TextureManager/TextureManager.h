#pragma once
#include "DirectXCommon.h"
#include "DirectXTex/DirectXTex.h"
#include "DirectXTex/d3dx12.h"
#include <algorithm> // std::find_if
#include <cassert> // assert
#include <iostream>
#include <string>
#include <vector>
class TextureManager {
public:
    // シングルトンインスタンスの取得
    static TextureManager* GetInstance();
    // 終了
    void Finalize();
    // 初期化
    void Initialize(DirectXCommon* dxCommon);

    void LoadTexture(const std::string& filePath);
    // SRVインデックスの取得
    uint32_t GetTextureIndexByFilePath(const std::string& filePath);
    // テクスチャ番号からGPUハンドルを取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

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
    DirectXCommon* dxCommon_ = nullptr;
    // SRVインデックスの開始番号
    static uint32_t kSRVIndexTop;
};
