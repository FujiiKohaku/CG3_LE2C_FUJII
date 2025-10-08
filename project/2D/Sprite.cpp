#include "Sprite.h"
#include "MatrixMath.h"
#include "SpriteManager.h"

#pragma region 初期化処理
void Sprite::Initialize(SpriteManager* spriteManager, std::string textureFilePath)
{
    // 引数で受け取ってメンバ変数に記録する
    spriteManager_ = spriteManager;
    // 頂点バッファの生成
    CreateVertexBuffer();
    // マテリアルバッファの生成
    CreateMaterialBuffer();
    // 変換行列バッファの生成
    CreateTransformationMatrixBuffer();
    // テクスチャの読み込み
    textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
}
#pragma endregion

#pragma region 更新処理
void Sprite::Update()
{
    // 頂点データを書き込む
    vertexData[0].position = { 0.0f, 1.0f, 0.0f, 1.0f }; // 左下
    vertexData[0].texcoord = { 0.0f, 1.0f };
    vertexData[0].normal = { 0.0f, 0.0f, -1.0f };

    vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f }; // 左上
    vertexData[1].texcoord = { 0.0f, 0.0f };
    vertexData[1].normal = { 0.0f, 0.0f, -1.0f };

    vertexData[2].position = { 1.0f, 1.0f, 0.0f, 1.0f }; // 右下
    vertexData[2].texcoord = { 1.0f, 1.0f };
    vertexData[2].normal = { 0.0f, 0.0f, -1.0f };

    vertexData[3].position = { 1.0f, 0.0f, 0.0f, 1.0f }; // 右上
    vertexData[3].texcoord = { 1.0f, 0.0f };
    vertexData[3].normal = { 0.0f, 0.0f, -1.0f };

    // インデックスデータを書き込む

    indexData[0] = 0;
    indexData[1] = 1;
    indexData[2] = 2;
    indexData[3] = 1;
    indexData[4] = 3;
    indexData[5] = 2;

    transform.translate = { position.x, position.y, 0.0f };

    transform.rotate = { 0.0f, 0.0f, rotation };

    transform.scale = { size.x, size.y, 1.0f };
    //  メイクアフィンマトリックス02_02
    Matrix4x4 worldMatrix = MatrixMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
    // ViewMatrixを作って単位行列を代入（
    Matrix4x4 viewMatrix = MatrixMath::MakeIdentity4x4();
    // projectionMatrixを作成
    // Sprite専用（左上原点）
    Matrix4x4 orthoSprite = MatrixMath::MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);

    // 最終的に行列をまとめてGPUに送る
    transformationMatrixData->WVP = MatrixMath::Multiply(worldMatrix, MatrixMath::Multiply(viewMatrix, orthoSprite));
    transformationMatrixData->World = worldMatrix;
}
#pragma endregion

#pragma region 描画処理
void Sprite::Draw()
{
    

    // コマンドリストを取得
    ID3D12GraphicsCommandList* commandList = spriteManager_->GetDxCommon()->GetCommandList();
    // 頂点バッファとインデックスバッファをセット
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    commandList->IASetIndexBuffer(&indexBufferView);
    // 定数バッファ(マテリアル)をセット
    commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
    // 定数バッファ(変換行列)をセット
    commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
    // テクスチャをセット
    commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex));
    // 描画コマンド
    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}
#pragma endregion

#pragma region 頂点バッファの設定
// 頂点バッファの生成
void Sprite::CreateVertexBuffer()
{
    // 頂点リソースを作る
    vertexResource = spriteManager_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * 4);
    // indexリソースを作る
    indexResource = spriteManager_->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * 6);
    // VertexBufferViewを作成する

    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress(); // リソース先頭のアドレスを使う
    vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * 4); // 使用するリソースの頂点のサイズ
    vertexBufferView.StrideInBytes = sizeof(VertexData); // 1頂点あたりのサイズ
    // IndexBufferViewを作成する

    indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress(); // リソースの先頭のアドレスから使う
    indexBufferView.SizeInBytes = sizeof(uint32_t) * 6; // 使用するリソースのサイズはインデックス６つ分のサイズ
    indexBufferView.Format = DXGI_FORMAT_R32_UINT; // インデックスはuint32_tとする
    // データを書き込むためのアドレスを取得
    vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
}
#pragma endregion

#pragma region マテリアルバッファの設定

void Sprite::CreateMaterialBuffer()
{

    // マテリアルリソースを作る
    materialResource = spriteManager_->GetDxCommon()->CreateBufferResource(sizeof(Material));
    // 書き込むためのアドレスを取得
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    // 今回は赤を書き込んでみる
    materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData->enableLighting = false;
    materialData->uvTransform = MatrixMath::MakeIdentity4x4(); // 06_01_UuvTransform行列を単位行列で初期化
}

#pragma endregion

#pragma region 変換行列バッファの設定

void Sprite::CreateTransformationMatrixBuffer()
{

    // リソースを作成
    transformationMatrixResource = spriteManager_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));

    // 書き込み用アドレスを取得
    transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

    // 初期値として単位行列を入れておく（安全のため）
    transformationMatrixData->WVP = MatrixMath::MakeIdentity4x4();
    transformationMatrixData->World = MatrixMath::MakeIdentity4x4();
}
#pragma endregion