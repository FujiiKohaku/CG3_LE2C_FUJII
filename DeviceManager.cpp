#include "DeviceManager.h"

void DeviceManager::Initialize(std::ofstream& logStream, WinApp* winApp, uint32_t width, uint32_t height)
{

    // DXGIファクトリ作成
    // HRESULTはWindows系のエラー子どであり
    // 関数が成功したかをSUCCEEDEDマクロで判定できる
    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(dxgiFactory_.GetAddressOf()));
    // 初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が多いのでassertにしておく
    assert(SUCCEEDED(hr));
    // 使用するアダプタ用の変数,最初にnullptrを入れておく
    IDXGIAdapter4* useAdapter = nullptr; // com
    // よい順にアダプタを頼む
    for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {

        // アダプターの情報を取得する
        DXGI_ADAPTER_DESC3 adapterDesc {}; // com
        hr = useAdapter->GetDesc3(&adapterDesc); // comGet
        assert(SUCCEEDED(hr)); // 取得できないのは一大事
        // ソフトウェアアダプタでなければ採用!
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) { // get
            // 採用したアダプタの情報をログに出力wstringの方なので注意
            Utility::Log(std::format(L"Use Adapater:{}\n", adapterDesc.Description)); // get
            break;
        }
        useAdapter = nullptr; // ソフトウェアアダプタの場合は見なかったことにする
    }

    // 適切なアダプタが見つからなかったので起動できない
    assert(useAdapter != nullptr);
    device_ = nullptr;
    // 昨日レベルとログ出力用の文字列
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
    };

    const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };
    // 高い順に生成できるか試していく
    for (size_t i = 0; i < _countof(featureLevels); ++i) {
        // 採用したアダプターでデバイスを生成
        hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device_));
        // 指定した昨日レベルでデバイスは生成できたか確認
        if (SUCCEEDED(hr)) {
            // 生成できたのでログ出力を行ってループを抜ける
            Utility::Log(std::format("FeatureLevel : {}\n", std::string(featureLevelStrings[i])));
            break;
        }
    }
    assert(device_ != nullptr);
    Utility::Log("Complete create D3D12Device!!!\n"); // 初期化完了のログを出す

    // コマンドキューを生成する

    D3D12_COMMAND_QUEUE_DESC commandQueueDesc {};
    hr = device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
    // コマンドキューの生成が上手くいかなかったので起動できない
    assert(SUCCEEDED(hr));

    // コマンドアロケーターを生成する

    hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
    // コマンドキューアロケーターの生成があ上手くいかなかったので起動できない
    assert(SUCCEEDED(hr));

    hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
    // コマンドリストの生成が上手くいかなかったので起動できない
    assert(SUCCEEDED(hr));

    // スワップチェーンを生成する
    swapChainDesc_.Width = 1280; // 画面の幅。ウィンドウのクライアント領域を同じものんにしておく
    swapChainDesc_.Height = 720; // 画面の高さ。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 色の形式
    swapChainDesc_.SampleDesc.Count = 1; // マルチサンプルしない
    swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 描画のターゲットとしてりようする
    swapChainDesc_.BufferCount = 2; // ダブルバッファ
    swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // モニターに移したら,中身を吐き
    // コマンドキュー,ウィンドウバンドル、設定を渡して生成する
    hr = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), winApp->GetHwnd(), &swapChainDesc_, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf())); // com.Get,OF
    assert(SUCCEEDED(hr));

    // ディスクリプタ―ヒープの生成
    rtvDescriptorHeapDesc_.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDescriptorHeapDesc_.NumDescriptors = 2;
    hr = device_->CreateDescriptorHeap(&rtvDescriptorHeapDesc_, IID_PPV_ARGS(&rtvDescriptorHeap_));
    // ディスクリプタ―ヒープが作れなかったので起動できない
    assert(SUCCEEDED(hr));
    // ==== SwapChain からバックバッファを取得 ====
    hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResources_[0]));
    assert(SUCCEEDED(hr));
    hr = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResources_[1]));
    assert(SUCCEEDED(hr));
    //==RTVを作成==//
    rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
    // 1つ目
    rtvHandles_[0] = rtvStartHandle;
    device_->CreateRenderTargetView(swapChainResources_[0].Get(), &rtvDesc_, rtvHandles_[0]);

    // 2つ目
    rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    device_->CreateRenderTargetView(swapChainResources_[1].Get(), &rtvDesc_, rtvHandles_[1]);
}
