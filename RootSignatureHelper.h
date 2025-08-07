#pragma once
#include <d3d12.h>
#include <wrl.h>

class Logger; // 前方宣言だけでOK

class RootSignatureHelper {
public:
    static Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateDefaultRootSignature(
        ID3D12Device* device,
        Logger& logger); // ← ostreamではなくLoggerに変更
};
