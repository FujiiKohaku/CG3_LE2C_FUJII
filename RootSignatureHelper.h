// RootSignatureHelper.h
#pragma once
#include <d3d12.h>
#include <ostream>
#include <wrl.h>

class RootSignatureHelper {
public:
    static Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateDefaultRootSignature(
        ID3D12Device* device,
        std::ostream& logStream);
};
