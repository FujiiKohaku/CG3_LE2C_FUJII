#include "GraphicsDevice.h"
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

void GraphicsDevice::Initialize(HWND hwnd, uint32_t width, uint32_t height)
{

    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
    assert(SUCCEEDED(hr));

    Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter; // アダプタ用の変数
    // よい順にアダプタを頼む
    for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; ++i) {

        // アダプターの情報を取得する
        DXGI_ADAPTER_DESC3 adapterDesc {}; // com
        hr = adapter->GetDesc3(&adapterDesc); // comGet
        assert(SUCCEEDED(hr)); // 取得できないのは一大事
        // ソフトウェアアダプタでなければ採用!
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) { // get
            // 採用したアダプタの情報をログに出力wstringの方なので注意
            // Utility::Log(logStream, Utility::ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description))); // 一旦コメントアウト後で戻ってくるね-

            break;
        }
        adapter.Reset();
    }
    assert(adapter);

    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_12_2,
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0
    };
    const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };

    // 高い順に生成できるか試していく
    for (size_t i = 0; i < _countof(levels); ++i) {
        hr = D3D12CreateDevice(adapter.Get(), levels[i], IID_PPV_ARGS(&device_));
        if (SUCCEEDED(hr)) {
            /*Utility::Log(logStream, std::format("FeatureLevel : {}\n", featureLevelStrings[i]));*/ // 後で戻ってくるよん
            break;
        }
    }
    // 最終的に device_ が生成できていない場合は assert
    assert(device_ != nullptr);
    /* Utility::Log(logStream, "Complete create D3D12Device!!!\n");*/ // 後で戻ってくるよん

    // コマンドキュー作成
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc {};
    hr = device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
    assert(SUCCEEDED(hr));

    hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
    assert(SUCCEEDED(hr));

    // コマンドリストを生成する
    hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
    // コマンドリストの生成が上手くいかなかったので起動できない
    assert(SUCCEEDED(hr));
}
