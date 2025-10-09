#include "Object3d.h"
#include "MatrixMath.h"
#include "Object3dManager.h"
#include <cassert>
#include <fstream>
#include <sstream>

void Object3d::Initialize(Object3dManager* object3DManager, DebugCamera debugCamera)
{
    object3dManager_ = object3DManager;
    debugCamera_ = debugCamera;

    // モデル読み込み
    modelData = LoadObjFile("resources", "plane.obj");

    // テクスチャ読み込み
    TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
    modelData.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath);

    // 頂点リソース作成
    vertexResource = object3dManager_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
    vertexBufferView.StrideInBytes = sizeof(VertexData);
    VertexData* vertexData = nullptr;
    vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

    // マテリアル作成
    materialResource = object3dManager_->GetDxCommon()->CreateBufferResource(sizeof(Material));
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    materialData->color = { 1, 1, 1, 1 };
    materialData->enableLighting = false;
    materialData->uvTransform = MatrixMath::MakeIdentity4x4();

    // 変換行列
    transformationMatrixResource = object3dManager_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
    transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
    transformationMatrixData->WVP = MatrixMath::MakeIdentity4x4();
    transformationMatrixData->World = MatrixMath::MakeIdentity4x4();

    // 平行光源
    directionalLightResource = object3dManager_->GetDxCommon()->CreateBufferResource(sizeof(DirectionalLight));
    directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
    directionalLightData->color = { 1, 1, 1, 1 };
    directionalLightData->direction = MatrixMath::Normalize({ 0, -1, 0 });
    directionalLightData->intensity = 1.0f;

    // Transform初期化
    transform = { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
    cameraTransform = { { 1.0f, 1.0f, 1.0f }, { 0.3f, 0.0f, 0.0f }, { 0.0f, 4.0f, -10.0f } };
}

void Object3d::Update()
{
    Matrix4x4 worldMatrix = MatrixMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
    Matrix4x4 cameraMatrix = MatrixMath::MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
    Matrix4x4 viewMatrix = debugCamera_.GetViewMatrix();
    Matrix4x4 projectionMatrix = MatrixMath::MakePerspectiveFovMatrix(
        0.45f,
        static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight),
        0.1f, 100.0f);

    transformationMatrixData->WVP = MatrixMath::Multiply(worldMatrix, MatrixMath::Multiply(viewMatrix, projectionMatrix));
    transformationMatrixData->World = worldMatrix;
}

void Object3d::Draw()
{
    ID3D12GraphicsCommandList* commandList = object3dManager_->GetDxCommon()->GetCommandList();

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

    D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = object3dManager_->GetDxCommon()->GetGPUDescriptorHandle(
        object3dManager_->GetDxCommon()->GetSRVDescriptorHeap(),
        object3dManager_->GetDxCommon()->GetSRVDescriptorSize(),
        modelData.material.textureIndex);
    commandList->SetGraphicsRootDescriptorTable(2, textureHandle);

    commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

    commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
}
Object3d::ModelData Object3d::LoadObjFile(const std::string& directoryPath, const std::string filename)
{
    ModelData modelData;
    std::vector<Vector4> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;
    std::string line;

    std::ifstream file(directoryPath + "/" + filename);
    assert(file.is_open());

    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        if (identifier == "v") {
            Vector4 pos;
            s >> pos.x >> pos.y >> pos.z;
            pos.x *= -1.0f;
            pos.w = 1.0f;
            positions.push_back(pos);
        } else if (identifier == "vt") {
            Vector2 uv;
            s >> uv.x >> uv.y;
            uv.y = 1.0f - uv.y;
            texcoords.push_back(uv);
        } else if (identifier == "vn") {
            Vector3 n;
            s >> n.x >> n.y >> n.z;
            n.x *= -1.0f;
            normals.push_back(n);
        } else if (identifier == "f") {
            VertexData tri[3];
            for (int i = 0; i < 3; ++i) {
                std::string v;
                s >> v;
                std::istringstream vtx(v);
                uint32_t idx[3];
                for (int j = 0; j < 3; ++j) {
                    std::string tmp;
                    std::getline(vtx, tmp, '/');
                    idx[j] = std::stoi(tmp);
                }
                tri[i].position = positions[idx[0] - 1];
                tri[i].texcoord = texcoords[idx[1] - 1];
                tri[i].normal = normals[idx[2] - 1];
            }
            modelData.vertices.push_back(tri[2]);
            modelData.vertices.push_back(tri[1]);
            modelData.vertices.push_back(tri[0]);
        } else if (identifier == "mtllib") {
            std::string mtlFile;
            s >> mtlFile;
            modelData.material = LoadMaterialTemplateFile(directoryPath, mtlFile);
        }
    }
    return modelData;
}
Object3d::MaterialData Object3d::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
     //1.中で必要となる変数の宣言
         MaterialData materialData; // 構築するMaterialData
         // 2.ファイルを開く
         std::string line; // ファイルから読んだ１行を格納するもの
         std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
         assert(file.is_open()); // とりあえず開けなかったら止める
         // 3.実際にファイルを読み、MaterialDataを構築していく
         while (std::getline(file, line)) {
             std::string identifier;
             std::istringstream s(line);
             s >> identifier;
             // identifierに応じた処理
             if (identifier == "map_Kd") {
                 std::string textureFilename;
                 s >> textureFilename;
                 // 連結してファイルパスにする
                 materialData.textureFilePath = directoryPath + "/" + textureFilename;
             }
         }
         // 4.materialDataを返す
         return materialData;
}
