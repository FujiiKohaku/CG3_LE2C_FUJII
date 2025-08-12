#pragma once
#include <Windows.h>

// 既存ヘッダ（今のmainがincludeしているものと同じでOK）
#include "CommonStructs.h"
#include "DirectionalLightBuffer.h"
#include "GameSceneManager.h"
#include "IndexBuffer.h"
#include "MaterialBuffer.h"
#include "MatrixMath.h"
#include "ModelLoder.h"
#include "SoundManager.h"
#include "Texture.h"
#include "UVTransformManager.h"
#include "VertexBuffer.h"
#include "WVPBuffer.h"
#include "WVPManager.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
class App {
public:
    void Initialize(HINSTANCE hInstance, const wchar_t* title, int32_t width, int32_t height);
    void Run();
    void Finalize();

private:
    void BeginFrame();
    void UpdateFrame();
    void DrawFrame();
    void EndFrame();

private:
    // ここから下は「元のWinMain内の変数名をそのまま」メンバにします
    GameSceneManager gameSceneManager;

    // auto& deviceManager = gameSceneManager.GetDeviceManager(); は毎回取得します（参照メンバは作りません）

    HRESULT hr {};
    VertexBuffer vertexBuffer;

    MaterialBuffer materialBuffer;

    // 3D
    WVPBuffer wvpBufferObject;
    WVPManager wvpManagerObject;

    // 2D
    WVPBuffer wvpBufferSprite;
    WVPManager wvpManagerSprite;

    UVTransformManager uvManager;

    DirectionalLightBuffer directionalLightBuffer;

    MaterialBuffer spriteMaterial;

    // トランスフォーム類
    Transform transformSprite { { 1, 1, 1 }, { 0, 0, 0 }, { 0, 0, 0 } };
    Transform transform { { 1, 1, 1 }, { 0, 0, 0 }, { 0, 0, 0 } };
    Transform cameraTransform { { 1, 1, 1 }, { 0, 0, 0 }, { 0, 0, -5.0f } };
    Transform uvTransformSprite { { 1, 1, 1 }, { 0, 0, 0 }, { 0, 0, 0 } };

    // ディスクリプタサイズ
    uint32_t descriptorSizeSRV {};
    uint32_t descriptorSizeRTV {};
    uint32_t descriptorSizeDSV {};

    // モデルとテクスチャ
    ModelData modelData;
    Texture texture2;
    Texture texture;

    // スプライト用
    VertexBuffer spriteVertexBuffer;
    IndexBuffer indexBufferSprite;

    // マテリアル中身
    Material materialData {};
    Material materialDataSprite {};
    // 追加バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite;
    TransformationMatrix* transformationMatrixDataSprite = nullptr;

    // サウンド
    SoundData bgm {};

    // そのまま使用している定数・状態
    float waveTime = 0.0f;
    const int32_t kClientWidth = 1280;
    const int32_t kClientHeight = 720;
};
