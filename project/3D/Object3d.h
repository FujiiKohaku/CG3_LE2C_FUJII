#pragma once
#include "DebugCamera.h"
#include "MatrixMath.h"
#include "Struct.h" // Vector2, Vector3, Vector4, Matrix4x4 など
#include "TextureManager.h"
#include <cstdint> // uint32_t など
#include <d3d12.h> // D3D12型定義（ID3D12Resourceなど）
#include <string> // std::string
#include <vector>
#include <wrl.h> // ComPtrスマートポインタ
class Object3dManager;
class Object3d {
public:
    // 初期化
    void Initialize(Object3dManager* object3DManager, DebugCamera debugCamera);
    // mtlファイルの読み取り

    void update();

    void Draw();

private:
    // 頂点データ
    struct VertexData {
        Vector4 position;
        Vector2 texcoord;
        Vector3 normal;
    };
    // マテリアル情報（テクスチャファイルパスなど）
    struct MaterialData {
        std::string textureFilePath;
        uint32_t textureIndex = 0;
    };

    // モデルデータ（頂点配列とマテリアル情報）
    struct ModelData {
        std::vector<VertexData> vertices;
        MaterialData material;
    };

    // マテリアルデータ
    struct Material {
        Vector4 color;
        int32_t enableLighting;
        float padding[3];
        Matrix4x4 uvTransform;
    };

    // 座標変換データ
    struct TransformationMatrix {
        Matrix4x4 WVP;
        Matrix4x4 World;
    };
    // ライティング情報
    struct DirectionalLight {
        Vector4 color;
        Vector3 direction;
        float intensity;
    };
    struct Transform {
        Vector3 scale;
        Vector3 rotate;
        Vector3 translate;
    };
    Object3dManager* object3dManager_ = nullptr;

    // objファイルのデータ
    ModelData modelData;

    // バッファリソース頂点
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
    // バッファリソースのデータをCPU側で編集するためのポインタ
    VertexData* vertexData = nullptr;
    // バッファリソースの使い道をGPUに伝えるための情報
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

    // バッファリソースマテリアル
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource; // ConstantBuffer
    // バッファリソース内のデータを指すポインタ
    Material* materialData = nullptr;

    // バッファリソース(位置)
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
    // バッファリソース内のデータを指すポインタ
    TransformationMatrix* transformationMatrixData = nullptr;

    Transform transform;

    Transform cameraTransform;

    DebugCamera debugCamera_;

    static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

    // .objファイルの読み取り
    static ModelData LoadObjFile(const std::string& directoryPath, const std::string filename);
};
