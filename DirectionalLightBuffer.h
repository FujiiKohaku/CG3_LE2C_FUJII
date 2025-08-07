#pragma once

#include "BufferHelper.h"
#include "MatrixMath.h"
#include"CommonStructs.h"
#include <d3d12.h>
#include <wrl.h>

class DirectionalLightBuffer {
public:
    // 初期化（リソース作成と初期データ設定）
    void Initialize(ID3D12Device* device);

    // 更新（毎フレームなどで呼ぶ用）
    void Update();

    // GPUアドレス取得
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

    // ライトのパラメータ取得・設定用（ImGuiなどから使う）
    DirectionalLight* GetData() { return lightData_; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
    DirectionalLight* lightData_ = nullptr;
};
