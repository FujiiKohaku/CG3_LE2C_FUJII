#pragma once
#include "DirectXCommon.h"
#include "DirectXTex/DirectXTex.h"
#include "DirectXTex/d3dx12.h"
#include <iostream>
#include <string>
#include <vector>
class TextureManager {
public:
    // �V���O���g���C���X�^���X�擾
    static TextureManager* GetInstance();
    // ������
    void Initialize();

    // �I��
    void Finalize();
    // �e�N�X�`���t�@�C���̓ǂݍ��݊֐�
    void LoadTexture(const std::string& filePath);

private:
    // �e�N�X�`��1�����̃f�[�^
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

    // �e�N�X�`���R���e�i�f�[�^
    std::vector<TextureData> textureDatas;
};
