#pragma once
#include "Utility.h"
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <format>
#include <wrl.h>
// リンク
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class DeviceManager {

public:
    void Initialize(std::ofstream& logStream);

    ID3D12Device* GetDevice() const { return device_.Get(); }
    IDXGIAdapter4* GetAdapter() const { return useAdapter_.Get(); }
    IDXGIFactory7* GetFactory() const { return dxgiFactory_.Get(); }

private:
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
    Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_;
    Microsoft::WRL::ComPtr<ID3D12Device> device_;
};
