#pragma once
#include "Logger.h"
#include "StringUtility.h"
#include <WinApp.h>
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h> // ComPtr 用
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
class DirectXCommon {
public:
    // 初期化
    void Initialize(WinApp* winApp);

private:
    // DXGIファクトリーの生成
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
    // デバイス
    Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;

    // コマンドキュー
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
    // コマンドアロケータ
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
    // コマンドリスト
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;

    // スワップチェイン
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;

    // 深度バッファリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;
    //  WindowsAPI
    WinApp* winApp_ = nullptr;
    // デバイス初期化関数
    void InitializeDevice();
    // コマンド初期化
    void InitializeCommand();
    // スワップチェイン
    void InitializeSwapChain();
    // 深度バッファ
    void InitializeDepthBuffer();
};
