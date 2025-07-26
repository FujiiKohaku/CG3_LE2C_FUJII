#include "GraphicsDevice.h"
#include <cassert>

void GraphicsDevice::CreateDevice()
{
    HRESULT hr;
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
    CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
    Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter;
    for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {

        // アダプターの情報を取得する
        DXGI_ADAPTER_DESC3 adapterDesc {};
        useAdapter->GetDesc3(&adapterDesc);
        // ソフトウェアアダプタでなければ採用!
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
            // ログファイルに出力したいならここに追記して
            break;
        }
        useAdapter = nullptr; // ソフトウェアアダプタの場合は見なかったことにする
    }
    assert(useAdapter != nullptr);

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
    };

    const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };

    for (size_t i = 0; i < _countof(featureLevels); ++i) {
        // 採用したアダプターでデバイスを生成
        hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
        // 指定した昨日レベルでデバイスは生成できたか確認
        if (SUCCEEDED(hr)) {
            // 生成できたのでログ出力を行ってループを抜ける
            OutputDebugStringA(std::format("FeatureLevel : {}\n", featureLevelStrings[i]).c_str());
            break;
        }
    }
    // デバイスの生成が上手くいかなかったので起動できない
    assert(device_ != nullptr); // デバイス作成失敗
}

void GraphicsDevice::CreateCommandObjects()
{
    HRESULT hr;

    // コマンドキューを生成する
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc {};
    commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    hr = device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
    assert(SUCCEEDED(hr)); // コマンドキューの生成が失敗

    // コマンドアロケーターを生成する
    hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
    assert(SUCCEEDED(hr));

    // コマンドリストを生成する
    hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
    // コマンドリストの生成が上手くいかなかったので起動できない
    assert(SUCCEEDED(hr));
}

void GraphicsDevice::CreateSwapChain(HWND hwnd, int width, int height)
{
    HRESULT hr;
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
    hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
    assert(SUCCEEDED(hr));

    // スワップチェイン作成
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;
    // コマンドキュー,ウィンドウバンドル、設定を渡して生成する
    Microsoft::WRL::ComPtr<IDXGISwapChain1> tempSwapChain;
    hr = dxgiFactory->CreateSwapChainForHwnd(
        commandQueue_.Get(),
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &tempSwapChain);
    assert(SUCCEEDED(hr));
    // IDXGISwapChain1 → IDXGISwapChain4 に変換
    hr = tempSwapChain.As(&swapChain_);
    assert(SUCCEEDED(hr));

    // 現在のバックバッファインデックスを取得
    backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
}

void GraphicsDevice::CreateDescriptorHeaps()
{


        CreateDescriptorHeap(graphicsDevice.GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);



}
