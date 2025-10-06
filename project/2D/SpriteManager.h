#pragma once
// #include "Logger.h"
// #include <cassert>
// #include <d3d12.h>
// #include <dxcapi.h>
// #include <wrl.h>
// #pragma comment(lib, "d3d12.lib")
// #pragma comment(lib, "dxgi.lib")
#include "DirectXCommon.h"

class SpriteManager {
public:
    // 初期化
    void Initialize(DirectXCommon* dxCommon);

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
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPinelineState = nullptr;
};
