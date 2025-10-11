#pragma once
#include "DirectXCommon.h"
#include "Object3d.h"
#include <string>
#include <vector>
#include <wrl.h>
#include "modelcommon.h"
class Model {
public:
    // 初期化（ModelCommonを受け取る）
    void Initialize(ModelCommon* modelCommon);

    // 描画
    void Draw();

    // ===============================
    // 構造体
    // ===============================
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

    struct DirectionalLight {
        Vector4 color;
        Vector3 direction;
        float intensity;
    };

    struct MaterialData {
        std::string textureFilePath;
        uint32_t textureIndex = 0;
    };

    struct VertexData {
        Vector4 position;
        Vector2 texcoord;
        Vector3 normal;
    };

    struct ModelData {
        std::vector<VertexData> vertices;
        MaterialData material;
    };
    // 変換情報
    struct Transform {
        Vector3 scale;
        Vector3 rotate;
        Vector3 translate;
    };
    //  Objファイルのデータ（頂点・マテリアルなど）
    Object3d::ModelData modelData_;
    // getter
    const Object3d::ModelData& GetModelData() const { return modelData_; }

private:
    // ModelCommonのポインタ（共通設定）
    ModelCommon* modelCommon_ = nullptr;

    // 頂点リソース（頂点バッファ用のGPUリソース）
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;

    // 頂点バッファビュー
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ {};

    //  頂点リソースにデータを書き込むためのポインタ
    Object3d::VertexData* vertexData_ = nullptr;

    // マテリアルリソース（マテリアル用の定数バッファ）
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

    // マテリアルリソースにデータを書き込むためのポインタ
    Object3d::Material* materialData_ = nullptr;
};
