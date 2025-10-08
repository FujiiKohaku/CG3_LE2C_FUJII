#pragma once
#include "Struct.h" // Vector2, Vector3, Vector4, Matrix4x4 など
#include <cstdint> // uint32_t など
#include <d3d12.h> // D3D12型定義（ID3D12Resourceなど）
#include <wrl.h> // ComPtrスマートポインタ
#include <string> // std::string
#include "TextureManager.h"
class SpriteManager;
class Sprite {
public:
    // 初期化
    void Initialize(SpriteManager* spriteManager, std::string textureFilePath);

    void Update();

    void Draw();

    // 位置のGetter/Setter
    const Vector2& GetPosition() const { return position; }
    void SetPosition(const Vector2& pos) { position = pos; }

    // 回転のGetter / Setter
    float GetRotation() const { return rotation; }
    void SetRotation(float rot) { rotation = rot; }

    // 色のGetter/Setter
    const Vector4& GetColor() const { return materialData->color; }
    void SetColor(const Vector4& color) { materialData->color = color; }

    // サイズのGetter/Setter
    const Vector2& GetSize() const { return size; }
    void SetSize(const Vector2& s) { size = s; }

private:
    // 頂点データ
    struct VertexData {
        Vector4 position;
        Vector2 texcoord;
        Vector3 normal;
    };

    // マテリアル
    struct Material {
        Vector4 color;
        int32_t enableLighting;
        float padding[3];
        Matrix4x4 uvTransform;
    };
    struct TransformationMatrix {
        Matrix4x4 WVP;
        Matrix4x4 World;
    };
    // 変換情報
    struct Transform {
        Vector3 scale;
        Vector3 rotate;
        Vector3 translate;
    };

    Vector2 position = { 0.0f, 0.0f };

    float rotation = 0.0f;

    // トランスフォーム
    Transform transform {
        { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }
    };
    Vector2 size = { 640.0f, 360.0f };
    // SpriteManagerのポインタ
    SpriteManager* spriteManager_ = nullptr;

    // バッファリソース
    //   バッファリソース（GPU用データ）
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource; // 頂点バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource; // インデックスバッファ
                                                          // CPU側のデータ参照用
    VertexData* vertexData = nullptr;
    uint32_t* indexData = nullptr;

    // バッファビュー（使い方情報）
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;

    // バッファリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
    // バッファリソース内のデータを指すポインタ
    TransformationMatrix* transformationMatrixData = nullptr;
    //  マテリアル関連
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr; // ConstantBuffer
    Material* materialData = nullptr; // GPUメモリ内のデータアクセス用ポインタ
    // テクスチャ関連
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_ {};
    void CreateVertexBuffer();
    void CreateMaterialBuffer();
    void CreateTransformationMatrixBuffer();
    int textureIndex = 0; // テクスチャ番号
};
