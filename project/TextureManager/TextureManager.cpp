#include "TextureManager.h"
#include <algorithm>
TextureManager* TextureManager::instance = nullptr;

// ImGuiで0盤を使用するため、１番から使用
uint32_t TextureManager::kSRVIndexTop = 1;

TextureManager* TextureManager::GetInstance()
{
    if (instance == nullptr) {
        instance = new TextureManager();
    }
    return instance;
}

void TextureManager::Initialize()
{
    // 初期化処理があればここに記述
    // SRVの数と同数
    textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

void TextureManager::Finalize()
{

    delete instance;
    instance = nullptr;
}

void TextureManager::LoadTexture(const std::string& filePath)
{

    // 読み込み済みテクスチャを検索
    auto it = std::find_if(textureDatas.begin(), textureDatas.end(), [&](TextureData& textureData) { return textureData.filePath == filePath; });

    if (it != textureDatas.end()) {

        // 読み込み済みなら早期return
        return;
    }
    // テクスチャ枚数上限チェック
    assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount);

    // テクスチャファイルを読んでプログラムで扱えるようにする
    DirectX::ScratchImage image {};
    std::wstring filePathW = StringUtility::ConvertString(filePath);
    HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
    // std::wcout << L"LoadFromWICFile HR: " << std::hex << hr << std::endl;
    assert(SUCCEEDED(hr));

    // ミップマップの作成
    DirectX::ScratchImage mipImages {};
    hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
    assert(SUCCEEDED(hr));

    // テクスチャデータを追加
    textureDatas.resize(textureDatas.size() + 1);
    // 追加したテクスチャデータの参照取得
    TextureData& textureData = textureDatas.back();

    textureData.filePath = filePath;
    textureData.metadata = mipImages.GetMetadata();
    textureData.resource = DirectXCommon::GetInstance()->CreateTextureResource(DirectXCommon::GetInstance()->GetDevice(), textureData.metadata);

    // テクスチャデータの要素数番号をSRVのインデックスとする
    uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;
    textureData.srvHandleCPU = DirectXCommon::GetInstance()->GetSRVCPUDescriptorHandle(srvIndex);
    textureData.srvHandleGPU = DirectXCommon::GetInstance()->GetSRVGPUDescriptorHandle(srvIndex);

    // --SRV作成--
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc {};
    srvDesc.Format = textureData.metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
    srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

    // --- SRV作成 ---
    DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filepath)
{

    // 読み込み済みテクスチャを検索
    auto it = std::find_if(textureDatas.begin(), textureDatas.end(), [&](TextureData& textureData) { return textureData.filePath == filepath; });
    if (it != textureDatas.end()) {
        // 読み込み済みなら要素番号を返す
        uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
        return textureIndex;
    }
    assert(0);
    return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex)
{
    // 範囲外指定違反チェック
    assert(textureIndex < textureDatas.size());
    TextureData& textureData = textureDatas[textureIndex];
    return textureData.srvHandleGPU;
}
