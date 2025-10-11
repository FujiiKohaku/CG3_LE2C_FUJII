#include "Object3d.h"
#include "MatrixMath.h"
#include "Model.h"
#include "Object3dManager.h"
#include <cassert>
#include <fstream>
#include <sstream>

#pragma region 初期化処理
void Object3d::Initialize(Object3dManager* object3DManager, DebugCamera debugCamera)
{
    // Object3dManager と DebugCamera を受け取って保持
    object3dManager_ = object3DManager;
    debugCamera_ = debugCamera;

    // ================================
    // Transformバッファ初期化
    // ================================
    transformationMatrixResource = object3dManager_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
    transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
    transformationMatrixData->WVP = MatrixMath::MakeIdentity4x4();
    transformationMatrixData->World = MatrixMath::MakeIdentity4x4();

    // ================================
    // 平行光源データ初期化
    // ================================
    directionalLightResource = object3dManager_->GetDxCommon()->CreateBufferResource(sizeof(DirectionalLight));
    directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
    directionalLightData->color = { 1, 1, 1, 1 };
    directionalLightData->direction = MatrixMath::Normalize({ 0, -1, 0 });
    directionalLightData->intensity = 1.0f;

    // ================================
    // Transform初期値設定
    // ================================
    transform = { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
    cameraTransform = { { 1.0f, 1.0f, 1.0f }, { 0.3f, 0.0f, 0.0f }, { 0.0f, 4.0f, -10.0f } };
}
#pragma endregion

#pragma region 更新処理
void Object3d::Update()
{
    // ================================
    // 各種行列を作成
    // ================================
    Matrix4x4 worldMatrix = MatrixMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
    Matrix4x4 viewMatrix = debugCamera_.GetViewMatrix(); // カメラから取得
    Matrix4x4 projectionMatrix = MatrixMath::MakePerspectiveFovMatrix(
        0.45f,
        static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight),
        0.1f, 100.0f);

    // ================================
    // 行列をGPUに転送
    // ================================
    transformationMatrixData->WVP = MatrixMath::Multiply(worldMatrix, MatrixMath::Multiply(viewMatrix, projectionMatrix));
    transformationMatrixData->World = worldMatrix;
}
#pragma endregion

#pragma region 描画処理
void Object3d::Draw()
{
    ID3D12GraphicsCommandList* commandList = object3dManager_->GetDxCommon()->GetCommandList();

    // Transform定数バッファをセット
    commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

    // ライト情報をセット
    commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

    // モデルが設定されていれば描画
    if (model_) {
        model_->Draw();
    }
}
#pragma endregion

#pragma region OBJ読み込み処理
// ===============================================
// OBJファイルの読み込み
// ===============================================
Object3d::ModelData Object3d::LoadObjFile(const std::string& directoryPath, const std::string filename)
{
    std::vector<Vector4> positions; // 位置
    std::vector<Vector3> normals; // 法線
    std::vector<Vector2> texcoords; // テクスチャ座標
    std::string line;

    Object3d::ModelData modelData;

    // ファイルを開く
    std::ifstream file(directoryPath + "/" + filename);
    assert(file.is_open());

    while (std::getline(file, line)) {
        std::string identifiler;
        std::istringstream s(line);
        s >> identifiler;

        // ================================
        // 頂点情報の読み込み
        // ================================
        if (identifiler == "v") {
            Vector4 position;
            s >> position.x >> position.y >> position.z;
            position.w = 1.0f;
            positions.push_back(position);
        } else if (identifiler == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;
            texcoord.y = 1.0f - texcoord.y; // 上下反転
            texcoords.push_back(texcoord);
        } else if (identifiler == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            normal.x *= -1.0f; // 左手座標系用
            normals.push_back(normal);
        } else if (identifiler == "f") {
            VertexData triangle[3];
            for (int i = 0; i < 3; ++i) {
                std::string vertexDef;
                s >> vertexDef;
                std::istringstream v(vertexDef);
                uint32_t indices[3];
                for (int j = 0; j < 3; ++j) {
                    std::string index;
                    std::getline(v, index, '/');
                    indices[j] = std::stoi(index);
                }
                triangle[i] = {
                    positions[indices[0] - 1],
                    texcoords[indices[1] - 1],
                    normals[indices[2] - 1]
                };
            }
            // 逆順に格納（時計回り→反時計回り）
            modelData.vertices.push_back(triangle[2]);
            modelData.vertices.push_back(triangle[1]);
            modelData.vertices.push_back(triangle[0]);
        } else if (identifiler == "mtllib") {
            // マテリアルファイル読み込み
            std::string materialFilename;
            s >> materialFilename;
            modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
        }
    }

    return modelData;
}
#pragma endregion

#pragma region MTL読み込み処理
// ===============================================
// マテリアル（.mtl）ファイルの読み込み
// ===============================================
Object3d::MaterialData Object3d::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
    MaterialData materialData;
    std::ifstream file(directoryPath + "/" + filename);
    assert(file.is_open());

    std::string line;
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        if (identifier == "map_Kd") {
            std::string textureFilename;
            s >> textureFilename;
            materialData.textureFilePath = directoryPath + "/" + textureFilename;
        }
    }
    return materialData;
}
#pragma endregion
