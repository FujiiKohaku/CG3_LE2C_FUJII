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

    // スワップチェーン生成
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc {};
    swapChainDesc.Width = width; // 画面の幅。ウィンドウのクライアント領域を同じものんにしておく
    swapChainDesc.Height = height; // 画面の高さ。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 色の形式
    swapChainDesc.SampleDesc.Count = 1; // マルチサンプルしない
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 描画のターゲットとしてりようする
    swapChainDesc.BufferCount = 2; // ダブルバッファ
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // モニターに移したら,中身を吐き
    // コマンドキュー,ウィンドウバンドル、設定を渡して生成する
    hr = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf())); // com.Get,OF
    assert(SUCCEEDED(hr));

    // RTVディスクリプタ―ヒープ作成
    D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc {};
    DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    DescriptorHeapDesc.NumDescriptors = kNumBackBuffers;
    DescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    hr = device_->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(&rtvHeap_));
    //---swapChainリソースの取得---
    for (UINT i = 0; i < kNumBackBuffers; ++i) {
        hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&swapChainResources_[i]));
        assert(SUCCEEDED(hr));
    }

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[kNumBackBuffers];
    UINT rtvIncrementSize = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    rtvHandles[0] = rtvStartHandle;
    device_->CreateRenderTargetView(swapChainResources_[0].Get(), &rtvDesc, rtvHandles[0]);

    rtvHandles[1].ptr = rtvHandles[0].ptr + rtvIncrementSize;
    device_->CreateRenderTargetView(swapChainResources_[1].Get(), &rtvDesc, rtvHandles[1]);
}

void GraphicsDevice::BeginFrame()
{
    OutputDebugStringA("== BeginFrame\n");

    commandList_->Close(); // 開いていても失敗しても無視でOK

    HRESULT hr = commandAllocator_->Reset();
    if (FAILED(hr)) {
        OutputDebugStringA("!! commandAllocator Reset failed\n");
    }

    hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
    if (FAILED(hr)) {
        OutputDebugStringA("!! commandList Reset failed\n");
    }

    backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();

    // 【ここが重要】Present → RenderTarget に状態遷移
    D3D12_RESOURCE_BARRIER barrier {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = swapChainResources_[backBufferIndex_].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList_->ResourceBarrier(1, &barrier);

    // 描画先とクリア
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
    UINT rtvDescriptorSize = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    rtvHandle.ptr += static_cast<SIZE_T>(backBufferIndex_) * rtvDescriptorSize;

    FLOAT clearColor[4] = { 0.1f, 0.1f, 0.3f, 1.0f };
    commandList_->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    commandList_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}





void GraphicsDevice::EndFrame()
{
    // 【ここが重要】RenderTarget → Present に状態遷移
    D3D12_RESOURCE_BARRIER barrier {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = swapChainResources_[backBufferIndex_].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList_->ResourceBarrier(1, &barrier);

    // コマンドリスト終了とキック
    HRESULT hr = commandList_->Close();
    if (FAILED(hr)) {
        OutputDebugStringA("!! Close failed\n");
    }

    ID3D12CommandList* cmdLists[] = { commandList_.Get() };
    commandQueue_->ExecuteCommandLists(1, cmdLists);

    // 表示
    swapChain_->Present(1, 0);

    OutputDebugStringA("== EndFrame\n");
}
