#include "TextureManager.h"
TextureManager* TextureManager::instance = nullptr;

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