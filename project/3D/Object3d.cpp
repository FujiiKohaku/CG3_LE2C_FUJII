#include "Object3d.h"
#include "MatrixMath.h"
#include "Model.h"
#include "Object3dManager.h"
#include <cassert>
#include <fstream>
#include <sstream>
void Object3d::Initialize(Object3dManager* object3DManager, DebugCamera debugCamera)
{
    object3dManager_ = object3DManager;
    debugCamera_ = debugCamera;

    //// モデル読み込み
    // modelData = LoadObjFile("resources", "plane.obj");

    //// テクスチャ読み込み
    // TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
    // modelData.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath);

    //// 頂点リソース作成
    // vertexResource = object3dManager_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
    // vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    // vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
    // vertexBufferView.StrideInBytes = sizeof(VertexData);
    // VertexData* vertexData = nullptr;
    // vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    // std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

    //// マテリアル作成
    // materialResource = object3dManager_->GetDxCommon()->CreateBufferResource(sizeof(Material));
    // materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    // materialData->color = { 1, 1, 1, 1 };
    // materialData->enableLighting = false;
    // materialData->uvTransform = MatrixMath::MakeIdentity4x4();

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

    /* commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
     commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);*/

    /* commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());*/
    commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

    /*D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = object3dManager_->GetDxCommon()->GetGPUDescriptorHandle(
        object3dManager_->GetDxCommon()->GetSRVDescriptorHeap(),
        object3dManager_->GetDxCommon()->GetSRVDescriptorSize(),
        modelData.material.textureIndex);
    commandList->SetGraphicsRootDescriptorTable(2, textureHandle);*/

    commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
    if (model_) {
        model_->Draw();
    }

    /*   commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);*/
}
Object3d::ModelData Object3d::LoadObjFile(const std::string& directoryPath, const std::string filename)
{

    std::vector<Vector4> positions; // 位置
    std::vector<Vector3> normals; // 法線
    std::vector<Vector2> texcoords; // テクスチャ座標
    std::string line; // ファイルから読んだ一行を格納するもの
    Object3d::ModelData modelData;
    // 2.ファイルを開く
    std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
    assert(file.is_open()); // とりあえず開けなかったら止める

    // 3.実際にファイルを読み,ModelDataを構築していく
    while (std::getline(file, line)) {
        std::string identifiler;
        std::istringstream s(line);
        s >> identifiler; // 先頭の識別子を読む

        // identifierに応じた処理
        if (identifiler == "v") {
            Vector4 position;
            s >> position.x >> position.y >> position.z;
            // 左手座標にする
            // position.x *= -1.0f;

            position.w = 1.0f;
            positions.push_back(position);
        } else if (identifiler == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;
            // 上下逆にする

            // texcoord.y *= -1.0f;
            texcoord.y = 1.0f - texcoord.y;
            // CG2_06_02_kusokusosjsusuawihoafwhgiuwhkgfau
            texcoords.push_back(texcoord);
        } else if (identifiler == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            // 左手座標にする
            normal.x *= -1.0f;

            normals.push_back(normal);
        } else if (identifiler == "f") {
            VertexData triangle[3]; // 三つの頂点を保存
            // 面は三角形限定。その他は未対応
            for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
                std::string vertexDefinition;
                s >> vertexDefinition;
                // 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してえIndexを取得する
                std::istringstream v(vertexDefinition);
                uint32_t elementIndices[3];
                for (int32_t element = 0; element < 3; ++element) {
                    std::string index;

                    std::getline(v, index, '/'); // 区切りでインデックスを読んでいく
                    elementIndices[element] = std::stoi(index);
                }
                // 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
                Vector4 position = positions[elementIndices[0] - 1];
                Vector2 texcoord = texcoords[elementIndices[1] - 1];
                Vector3 normal = normals[elementIndices[2] - 1];
                // X軸を反転して左手座標系に

                triangle[faceVertex] = { position, texcoord, normal };
            }
            // 逆順にして格納（2 → 1 → 0）
            modelData.vertices.push_back(triangle[2]);
            modelData.vertices.push_back(triangle[1]);
            modelData.vertices.push_back(triangle[0]);
            //?
        } else if (identifiler == "mtllib") {
            // materialTemplateLibraryファイルの名前を取得する
            std::string materialFilename;
            s >> materialFilename;
            // 基本的にobjファイルと同一階層mtlは存在させるので、ディレクトリ名とファイル名を渡す。
            modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
        }
    }
    // 4.ModelDataを返す
    return modelData;
}

Object3d::MaterialData Object3d::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
    // 1.中で必要となる変数の宣言
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
