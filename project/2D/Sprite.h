#pragma once
class SpriteManager;
class Sprite {
public:
    // 初期化
    void Initialize(SpriteManager* spriteManager);

private:
    // SpriteManagerのポインタ
    SpriteManager* spriteManager_ = nullptr;
    // 頂点データ
    struct VertexData {
        Vector4 position;
        Vector2 texcoord;
        Vector3 normal;
    };
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
};
