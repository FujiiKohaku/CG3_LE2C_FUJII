#pragma once

// 標準ライブラリ
#include <d3d12.h>
#include <wrl.h>

// 自作ヘッダー（必要に応じてパス調整）
#include "BufferHelper.h" // CreateBufferResource の宣言
#include "CommonStructs.h" // Material 構造体の定義（Vector4, Matrix4x4 など）

class MaterialBuffer {
public:
    MaterialBuffer() = default;
    ~MaterialBuffer() = default;

    // 初期化＋書き込み（1行で完結）
    void Create(ID3D12Device* device, const Material& material);

    // データの更新
    void Update(const Material& material);

    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
    {
        return materialResource_->GetGPUVirtualAddress();
    }
    // GPUリソースの取得
    ID3D12Resource* GetResource() const { return materialResource_.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
};
