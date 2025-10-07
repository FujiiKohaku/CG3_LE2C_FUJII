#pragma once
#include "DirectXCommon.h"
#include "DirectXTex/DirectXTex.h"
#include "DirectXTex/d3dx12.h"
#include <iostream>
#include <string>
#include <vector>
class TextureManager {
public:
    // シングルトンインスタンス取得
    static TextureManager* GetInstance();
    // 初期化
    void Initialize();

    // 終了
    void Finalize();
    // テクスチャファイルの読み込み関数
    void LoadTexture(const std::string& filePath);

private:
    // テクスチャ1枚分のデータ
    struct TextureData {
        std::string filePath;
        DirectX::TexMetadata metadata;
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
    };

    static TextureManager* instance;
    TextureManager() = default;
    ~TextureManager() = default;
    TextureManager(TextureManager&) = delete;
    TextureManager& operator=(TextureManager&) = delete;

    // テクスチャコンテナデータ
    std::vector<TextureData> textureDatas;
};
