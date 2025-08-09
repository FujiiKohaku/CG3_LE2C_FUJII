#include "GameSceneManager.h"
#include "Utility.h"
#include <filesystem>

static void EnableDebugLayerIfDebug()
{
#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        debugController->SetEnableGPUBasedValidation(TRUE);
    }
#endif
}

static void SetupInfoQueueIfDebug(ID3D12Device* device)
{
#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
    if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

        D3D12_MESSAGE_ID denyIds[] = {
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
        };
        D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_INFO_QUEUE_FILTER filter {};
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;
        infoQueue->PushStorageFilter(&filter);
    }
#endif
}

void GameSceneManager::Initialize(HINSTANCE hInst, int nCmdShow,
    const wchar_t* title, uint32_t width, uint32_t height)
{
    // ログ・例外ハンドラ
    std::filesystem::create_directory("logs");
    SetUnhandledExceptionFilter(Utility::ExportDump);
    log_.Initialize();
    log_.Log("初期化開始");

    // デバッグレイヤ
    EnableDebugLayerIfDebug();

    // ウィンドウ
    win_.Initialize(hInst, nCmdShow, std::wstring(title), width, height);

    // デバイスマネージャ
    deviceManager_.Initialize(log_.GetStream(), &win_, width, height);

    // InfoQueue
    SetupInfoQueueIfDebug(deviceManager_.GetDevice());

    // DSV・SRVヒープ
    dsvHeap_.Create(deviceManager_.GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
    srvHeap_.Create(deviceManager_.GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

    // 深度ステンシルバッファ
    depthStencillResource_ = CreateDepthStencilTextureResource(deviceManager_.GetDevice(), width, height);
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    deviceManager_.GetDevice()->CreateDepthStencilView(depthStencillResource_.Get(), &dsvDesc, dsvHeap_.GetCPUHandleStart());

    // フェンス
    HRESULT hr = deviceManager_.GetDevice()->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
    assert(SUCCEEDED(hr));
    fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent_ != nullptr);

    // ImGui 初期化
    win_.ImGuiInitialize(deviceManager_.GetDevice(),
        deviceManager_.GetRTVDesc().Format,
        srvHeap_.GetHeap(),
        deviceManager_.GetSwapChainDesc().BufferCount);

    log_.Log("初期化完了");
}
