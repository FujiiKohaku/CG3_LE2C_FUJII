#include "DeviceManager.h"

void DeviceManager::Initialize(std::ofstream& logStream)
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
}
