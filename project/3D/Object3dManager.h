#pragma once
#include "DirectXCommon.h"
class Object3dManager {
public:
    // 初期化
    void Initialize(DirectXCommon* dxCommon);

    // 共通描画前処理
    void PreDraw();

    // getter
    DirectXCommon* GetDxCommon() const { return dxCommon_; }

private:
    // ルートシグネチャ作成
    void CreateRootSignature();
    // グラフィックスパイプライン作成
    void CreateGraphicsPipeline();

    // 借りるDirectXCommonのポインタ
    DirectXCommon* dxCommon_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
    ID3DBlob* signatureBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
};
