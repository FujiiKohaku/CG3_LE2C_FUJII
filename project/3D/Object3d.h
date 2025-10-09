#pragma once
#include "MatrixMath.h"
#include "Struct.h" // Vector2, Vector3, Vector4, Matrix4x4 など
#include <cstdint> // uint32_t など
#include <d3d12.h> // D3D12型定義（ID3D12Resourceなど）
#include <string> // std::string
#include <vector>
#include <wrl.h> // ComPtrスマートポインタ
class Object3dManager;
class Object3d {
public:
    // 初期化
    void Initialize(Object3dManager* object3DManager);
    // mtlファイルの読み取り

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

    static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

    // .objファイルの読み取り
    static ModelData LoadObjFile(const std::string& directoryPath, const std::string filename);
};
