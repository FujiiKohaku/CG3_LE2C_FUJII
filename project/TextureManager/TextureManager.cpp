#include "TextureManager.h"
#include <format>
TextureManager* TextureManager::instance = nullptr;
// ImGuiで0番を使用するため、1番から使用
uint32_t TextureManager::kSRVIndexTop = 1;
TextureManager* TextureManager::GetInstance()
{
    // インスタンスがなければ生成
    if (instance == nullptr) {
        instance = new TextureManager();
    }
    return instance;
}
void TextureManager::Finalize()
{
    // インスタンスが存在するなら削除してnullptrに
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}

void TextureManager::Initialize(DirectXCommon* dxCommon)
{
    dxCommon_ = dxCommon;
    // SRVの数と同数
    textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

void TextureManager::LoadTexture(const std::string& filePath)

{ // 読み込み済みテクスチャを検索
    auto it = std::find_if(
        textureDatas.begin(),
        textureDatas.end(),
        [&](TextureData& textureData) { return textureData.filePath == filePath; });
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

    // 追加したテクスチャデータの参照を取得する
    TextureData& textureData = textureDatas.back();

    // ファイルパスを記録
    textureData.filePath = filePath;

    // テクスチャメタデータを取得
    textureData.metadata = mipImages.GetMetadata();

    // テクスチャリソースを生成
    textureData.resource = dxCommon_->CreateTextureResource(dxCommon_->GetDevice(), textureData.metadata);

    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = dxCommon_->UploadTextureData(textureData.resource, mipImages);
    // コマンドを閉じてGPUに送る
    dxCommon_->GetCommandList()->Close();
    ID3D12CommandList* lists[] = { dxCommon_->GetCommandList() };
    dxCommon_->GetCommandQueue()->ExecuteCommandLists(_countof(lists), lists);

    uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;

    Logger::Log(std::format("Texture Loaded: {}, SRV Index: {}", filePath, srvIndex));
    // CPU・GPUハンドルを取得
    textureData.srvHandleCPU = dxCommon_->GetCPUDescriptorHandle(dxCommon_->GetSRVDescriptorHeap(), dxCommon_->GetSRVDescriptorSize(), srvIndex);

    textureData.srvHandleGPU = dxCommon_->GetGPUDescriptorHandle(dxCommon_->GetSRVDescriptorHeap(), dxCommon_->GetSRVDescriptorSize(), srvIndex);

    // SRVの設定を行う
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc {};
    srvDesc.Format = textureData.metadata.format; // テクスチャのフォーマット
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
    srvDesc.Texture2D.MipLevels = static_cast<UINT>(textureData.metadata.mipLevels);

    // SRVの生成（
    dxCommon_->GetDevice()->CreateShaderResourceView(
        textureData.resource.Get(), // リソース
        &srvDesc, // SRVの設定
        textureData.srvHandleCPU); // CPUハンドル（書き込み先）
    // GPUにアップロード完了を保証
    dxCommon_->WaitForGPU();
    // コマンド再利用可能に
    dxCommon_->GetCommandAllocator()->Reset();
    dxCommon_->GetCommandList()->Reset(dxCommon_->GetCommandAllocator(), nullptr);
    Logger::Log(std::format("srvIndex = {}", srvIndex));
}



uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath)
{
    // 読み込み済みテクスチャデータを検索
    auto it = std::find_if(
        textureDatas.begin(),
        textureDatas.end(),
        [&](TextureData& textureData) { return textureData.filePath == filePath; });

    if (it != textureDatas.end()) {
        // SRVヒープ上のインデックス = 配列の位置 + kSRVIndexTop
        uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it)) + kSRVIndexTop;
        return textureIndex;
    }

    // 読み込まれていない場合はエラー
    assert(0);
    return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex)
{
    // 範囲外アクセス防止（kSRVIndexTop考慮）
    assert(textureIndex >= kSRVIndexTop);
    assert(textureIndex - kSRVIndexTop < textureDatas.size());

    // 正しい位置から取得
    TextureData& textureData = textureDatas[textureIndex - kSRVIndexTop];

    // GPUハンドルを返す
    return textureData.srvHandleGPU;
}