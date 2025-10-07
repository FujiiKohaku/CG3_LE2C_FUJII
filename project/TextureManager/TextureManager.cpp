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
    // ‰Šú‰»ˆ—‚ª‚ ‚ê‚Î‚±‚±‚É‹Lq
    // SRV‚Ì”‚Æ“¯”
    textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

void TextureManager::Finalize()
{

    delete instance;
    instance = nullptr;
}