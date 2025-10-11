#include "Model.h"
#include "ModelCommon.h"
#include <cassert>
#include <cstring>

void Model::Initialize(ModelCommon* modelCommon)
{
    // ModelCommonのポインタを引数からメンバ変数に記録する
    modelCommon_ = modelCommon;

    // モデル読み込み
    modelData_ = Object3d::LoadObjFile("resources", "plane.obj");
    // 頂点データの初期化
    vertexResource_ = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
    VertexData* vertexData = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());

    // マテリアルの初期化
    //   マテリアル作成
    materialResource_ = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(Material));
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    materialData_->color = { 1, 1, 1, 1 };
    materialData_->enableLighting = false;
    materialData_->uvTransform = MatrixMath::MakeIdentity4x4();

    // テクスチャ読み込み
    TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
    // テクスチャ番号取得
    modelData_.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData_.material.textureFilePath);
}

void Model::Draw()
{
    ID3D12GraphicsCommandList* commandList = modelCommon_->GetDxCommon()->GetCommandList();
    // vertexBufferViewを設定
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // マテリアル定数バッファビューをセット
    commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
    // SRVのdescriptorをセット
    D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = modelCommon_->GetDxCommon()->GetGPUDescriptorHandle(
        modelCommon_->GetDxCommon()->GetSRVDescriptorHeap(),
        modelCommon_->GetDxCommon()->GetSRVDescriptorSize(),
        modelData_.material.textureIndex);
    commandList->SetGraphicsRootDescriptorTable(2, textureHandle);
    // 描画コマンド
    commandList->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);
}
