#pragma once
#include "Struct.h"
#include <cstdint>
#include <string>
#include <vector>
// 変換情報
struct Transform {
    Vector3 scale;
    Vector3 rotate;
    Vector3 translate;
};
// 頂点、マテリアル関連
struct VertexData {
    Vector4 position;
    Vector2 texcoord;
    Vector3 normal;

};
struct Material {
    Vector4 color;
    int32_t enableLighting;
    int32_t lightingMode; // ★追加
    float padding[2]; // 16バイトアラインメント用
    Matrix4x4 uvTransform;
    int32_t enableCaustics;
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
// パーティクル等
struct Fragment {
    Vector3 position;
    Vector3 velocity;
    Vector3 rotation;
    Vector3 rotationSpeed;
    float alpha;
    bool active;
};

// モデルデータ
struct MaterialData {
    std::string textureFilePath;
};

struct ModelData {
    std::vector<VertexData> vertices;
    MaterialData material;
};
// UVなし
struct VertexDataNoUV {
    Vector4 position;
    Vector3 normal;
};
struct ModelDataNoUV {
    std::vector<VertexDataNoUV> vertices;
};
