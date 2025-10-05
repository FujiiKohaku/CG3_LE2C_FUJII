#include "DirectXCommon.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <format>
#include <wrl.h>
void DirectXCommon::Initialize(WinApp* winApp)
{

    // NULLチェック
    assert(winApp);
    winApp_ = winApp;
    // ここに初期化処理を書いていく
}

#pragma region デバイス初期化

// デバイス初期化関数
void DirectXCommon::InitializeDevice()
{
    HRESULT hr;
    // デバッグレイヤーをONにする
#ifdef _DEBUG

    Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr; // COM
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        // デバックレイヤーを有効化する
        debugController->EnableDebugLayer();
        // さらに6PU側でもチェックリストを行うようにする
        debugController->SetEnableGPUBasedValidation(TRUE);
    }
#endif // _DEBUG
    // DXGIファクトリーの生成
    hr = CreateDXGIFactory(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
    // 初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が多いのでassertにしておく
    assert(SUCCEEDED(hr));
    IDXGIAdapter4* useAdapter = nullptr; // com
    // よい順にアダプタを頼む
    for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {

        // アダプターの情報を取得する
        DXGI_ADAPTER_DESC3 adapterDesc {}; // com
        hr = useAdapter->GetDesc3(&adapterDesc); // comGet
        assert(SUCCEEDED(hr)); // 取得できないのは一大事
        // ソフトウェアアダプタでなければ採用!
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) { // get
            // 採用したアダプタの情報をログに出力wstringの方なので注意
            Logger::Log(StringUtility::ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description))); // get
            break;
        }
        useAdapter = nullptr; // ソフトウェアアダプタの場合は見なかったことにする
    }

    // 適切なアダプタが見つからなかったので起動できない
    assert(useAdapter != nullptr);

    // 昨日レベルとログ出力用の文字列
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
    };

    const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };
    // 高い順に生成できるか試していく
    for (size_t i = 0; i < _countof(featureLevels); ++i) {
        // 採用したアダプターでデバイスを生成
        hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
        // 指定した昨日レベルでデバイスは生成できたか確認
        if (SUCCEEDED(hr)) {
            // 生成できたのでログ出力を行ってループを抜ける

            Logger::Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
            break;
        }
    }
    // デバイスの生成が上手くいかなかったので起動できない
    assert(device != nullptr);
    Logger::Log("Complete create D3D12Device!!!\n"); // 初期化完了のログを出す
}
#pragma endregion

#pragma region コマンド初期化
void DirectXCommon::InitializeCommand()
{
    HRESULT hr;

    // コマンドキューを生成する
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc {};
    hr = device->CreateCommandQueue(&commandQueueDesc,
        IID_PPV_ARGS(&commandQueue));
    // コマンドキューの生成が上手くいかなかったので起動できない
    assert(SUCCEEDED(hr));

    // コマンドアロケーターを生成する
    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    // コマンドキューアロケーターの生成があ上手くいかなかったので起動できない
    assert(SUCCEEDED(hr));

    // コマンドリストを生成する
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
    // コマンドリストの生成が上手くいかなかったので起動できない
    assert(SUCCEEDED(hr));
}
#pragma endregion

void DirectXCommon::InitializeSwapChain()
{
    HRESULT hr;

    // スワップチェーンを生成する
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc {};
    swapChainDesc.Width = WinApp::kClientWidth; // 画面の幅。ウィンドウのクライアント領域を同じものんにしておく
    swapChainDesc.Height = WinApp::kClientHeight; // 画面の高さ。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 色の形式
    swapChainDesc.SampleDesc.Count = 1; // マルチサンプルしない
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 描画のターゲットとしてりようする
    swapChainDesc.BufferCount = 2; // ダブルバッファ
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // モニターに移したら,中身を吐き
    // コマンドキュー,ウィンドウバンドル、設定を渡して生成する
    hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp_->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf())); // com.Get,OF
    assert(SUCCEEDED(hr));
}

void DirectXCommon::InitializeDepthBuffer()
{
    HRESULT hr;

    // === 生成するResourceの設定 ===
    D3D12_RESOURCE_DESC resourceDesc {};
    resourceDesc.Width = WinApp::kClientWidth;
    resourceDesc.Height = WinApp::kClientHeight;
    resourceDesc.MipLevels = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    // === 利用するHeapの設定 ===
    D3D12_HEAP_PROPERTIES heapProperties {};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    // === 深度値のクリア設定 ===
    D3D12_CLEAR_VALUE depthClearValue {};
    depthClearValue.DepthStencil.Depth = 1.0f;
    depthClearValue.DepthStencil.Stencil = 0;
    depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // === Resourceの生成 ===　
    hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClearValue,
        IID_PPV_ARGS(&depthStencilResource));

    assert(SUCCEEDED(hr));
}

// ディスクリプタヒープ生成関数
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisivle)
{
    // ディスクリプタヒープの生成02_02
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeap = nullptr;

    D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc {};
    DescriptorHeapDesc.Type = heapType;
    DescriptorHeapDesc.NumDescriptors = numDescriptors;
    DescriptorHeapDesc.Flags = shaderVisivle ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT hr = device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(&DescriptorHeap));
    // ディスクリプタヒープが作れなかったので起動できない
    assert(SUCCEEDED(hr)); // 1
    return DescriptorHeap;
}

void DirectXCommon::InitializeDescriptorHeaps()
{

    descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    // RTV用のヒープ（Shaderからは使わないのでfalse）
    rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

    // DSV用のヒープ（Shaderからは使わないのでfalse）
    dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

    // SRV用のヒープ（Shaderから使うのでtrue）
    srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
}
