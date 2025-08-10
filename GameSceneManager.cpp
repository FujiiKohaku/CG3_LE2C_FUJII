#include "GameSceneManager.h"
#include "Utility.h"

// 第3陣：シェーダ／PSO周り
#include "BlendStateHelper.h"
#include "InputLayoutHelper.h"
#include "PipelineBuilder.h"
#include "RasterizerStateHelper.h"
#include "RootSignatureHelper.h"
#include "ShaderCompiler.h"
#include "ShaderCompilerDXC.h"

#include <cassert>
#include <filesystem>

// ===================== 内部ヘルパ =====================

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

        // ノイズ抑制（環境依存の警告を隠す）
        D3D12_MESSAGE_ID denyIds[] = {
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
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

// ===================== 本体 =====================

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

    // ===== ディスクリプタヒープ =====
    dsvHeap_.Create(deviceManager_.GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
    srvHeap_.Create(deviceManager_.GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

    // ===== 深度ステンシル =====
    depthStencillResource_ = CreateDepthStencilTextureResource(deviceManager_.GetDevice(), width, height);
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc {};
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        deviceManager_.GetDevice()->CreateDepthStencilView(
            depthStencillResource_.Get(), &dsvDesc, dsvHeap_.GetCPUHandleStart());
    }

    // ===== フェンス =====
    {
        HRESULT hr = deviceManager_.GetDevice()->CreateFence(
            fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
        assert(SUCCEEDED(hr));
        fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
        assert(fenceEvent_ != nullptr);
    }

    // ===== ImGui =====
    win_.ImGuiInitialize(
        deviceManager_.GetDevice(),
        deviceManager_.GetRTVDesc().Format,
        srvHeap_.GetHeap(),
        deviceManager_.GetSwapChainDesc().BufferCount);

    // ===== ディスクリプタサイズ（ここで一度だけ取得） =====
    descriptorSizeSRV_ = deviceManager_.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    descriptorSizeRTV_ = deviceManager_.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    descriptorSizeDSV_ = deviceManager_.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    // ===== 第3陣：DXC / ルートシグネチャ / PSO =====
    dxc_.Initialize();

    // ルートシグネチャ
    rootSignature_ = RootSignatureHelper::CreateDefaultRootSignature(deviceManager_.GetDevice(), log_);
    assert(rootSignature_ != nullptr);

    // シェーダコンパイル
    Microsoft::WRL::ComPtr<IDxcBlob> vsBlob = CompileShader(
        L"Object3d.VS.hlsl", L"vs_6_0",
        dxc_.GetUtils(), dxc_.GetCompiler(), dxc_.GetIncludeHandler(), log_);
    assert(vsBlob != nullptr);

    Microsoft::WRL::ComPtr<IDxcBlob> psBlob = CompileShader(
        L"Object3d.PS.hlsl", L"ps_6_0",
        dxc_.GetUtils(), dxc_.GetCompiler(), dxc_.GetIncludeHandler(), log_);
    assert(psBlob != nullptr);

    // PSO 構築
    {
        PipelineBuilder builder;
        BlendStateHelper blend;
        blend.CreateWriteAll();
        RasterizerStateHelper rast;
        rast.CreateDefault();

        D3D12_DEPTH_STENCIL_DESC dsDesc {};
        dsDesc.DepthEnable = true;
        dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

        builder.SetRootSignature(rootSignature_.Get());
        builder.SetInputLayout(InputLayoutHelper::CreatePosTexNormLayout());
        builder.SetVertexShader(vsBlob.Get());
        builder.SetPixelShader(psBlob.Get());
        builder.SetBlendState(blend.CreateWriteAll());
        builder.SetRasterizerState(rast.CreateDefault());
        builder.SetDepthStencilState(dsDesc);
        builder.SetRTVFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
        builder.SetDSVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);
        builder.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

        pipelineState_ = builder.Build(deviceManager_.GetDevice());
        assert(pipelineState_ != nullptr);
    }

    render_ = std::make_unique<RenderHelper>(deviceManager_);
    log_.Log("初期化完了");

    // ★ここで初期化
    input_.Initialize(hInst, win_.GetHwnd());
    debugCamera_.Initialize(hInst, win_.GetHwnd());
    soundmanager_.Initialize();
}

// ===================== 便利ハンドル関数 =====================

D3D12_CPU_DESCRIPTOR_HANDLE GameSceneManager::GetSRVCPUHandle(uint32_t index) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE h = srvHeap_.GetHeap()->GetCPUDescriptorHandleForHeapStart();
    h.ptr += static_cast<SIZE_T>(descriptorSizeSRV_) * index;
    return h;
}

D3D12_GPU_DESCRIPTOR_HANDLE GameSceneManager::GetSRVGPUHandle(uint32_t index) const
{
    D3D12_GPU_DESCRIPTOR_HANDLE h = srvHeap_.GetHeap()->GetGPUDescriptorHandleForHeapStart();
    h.ptr += static_cast<UINT64>(descriptorSizeSRV_) * index;
    return h;
}

D3D12_CPU_DESCRIPTOR_HANDLE GameSceneManager::GetDSVCPUHandle(uint32_t index) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE h = dsvHeap_.GetHeap()->GetCPUDescriptorHandleForHeapStart();
    h.ptr += static_cast<SIZE_T>(descriptorSizeDSV_) * index;
    return h;
}

void GameSceneManager::BeginFrame()
{
    win_.ImGuiBeginFrame();
}

void GameSceneManager::EndFrame()
{
    win_.ImGuiEndFrame(deviceManager_.GetCommandList());
}

void GameSceneManager::PreDraw(const float clearColor[4], const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor)
{
    render_->PreDraw(clearColor,
        dsvHeap_.GetHeap(),
        viewport,
        scissor,
        rootSignature_.Get(),
        pipelineState_.Get(),
        srvHeap_.GetHeap());
}

void GameSceneManager::PostDraw()
{
    render_->PostDraw(fence_.Get(), fenceEvent_, fenceValue_);
}

void GameSceneManager::LoadTextureAndMakeSRV(const char* texturePath, uint32_t srvIndex, Texture& outTex)
{
    // 読み込み（アップロード含む）
    outTex.LoadFromFile(deviceManager_.GetDevice(), deviceManager_.GetCommandList(), texturePath);

    // SRVを指定スロットへ
    D3D12_CPU_DESCRIPTOR_HANDLE cpu = GetSRVCPUHandle(srvIndex);
    D3D12_GPU_DESCRIPTOR_HANDLE gpu = GetSRVGPUHandle(srvIndex);
    outTex.CreateSRV(deviceManager_.GetDevice(), cpu, gpu);
}

void GameSceneManager::LoadModelAndMaterialSRV(const char* dir, const char* filename, uint32_t srvIndex,
    ModelData& outModel, Texture& outTex)
{
    // モデル読み込み（material.textureFilePath が埋まる前提）
    outModel = LoadObjFile(dir, filename);

    // マテリアルのテクスチャを読み込んでSRV作成
    outTex.LoadFromFile(deviceManager_.GetDevice(), deviceManager_.GetCommandList(),
        outModel.material.textureFilePath.c_str());

    D3D12_CPU_DESCRIPTOR_HANDLE cpu = GetSRVCPUHandle(srvIndex);
    D3D12_GPU_DESCRIPTOR_HANDLE gpu = GetSRVGPUHandle(srvIndex);
    outTex.CreateSRV(deviceManager_.GetDevice(), cpu, gpu);
}