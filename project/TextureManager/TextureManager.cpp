#include "TextureManager.h"
TextureManager* TextureManager::instance = nullptr;

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
const uint32_t DirectXCommon::kMaxSRVCount = 512;