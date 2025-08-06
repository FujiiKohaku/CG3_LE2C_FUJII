// ======================= プリプロセッサ ==========================
#define _USE_MATH_DEFINES

// ======================= 標準ライブラリ ==========================
#include <cassert>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// ======================= Windows / DirectX =======================
#include <Windows.h>
#include <d3d12.h>
#include <dbghelp.h>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <wrl.h>

// ======================= DirectXTex ==============================
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

// ======================= ImGui関連 ===============================
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

// ======================= 自作ユーティリティ =======================
#include "BlendStateHelper.h"
#include "BufferHelper.h"
#include "DebugCamera.h"
#include "DescriptorHeapWrapper.h"
#include "DeviceManager.h"
#include "Dxc.h"
#include "Input.h"
#include "InputLayoutHelper.h"
#include "MaterialBuffer.h"
#include "MatrixMath.h"
#include "ModelLoder.h"
#include "RasterizerStateHelper.h"
#include "RenderHelper.h"
#include "RootSignatureHelper.h"
#include "ShaderCompiler.h"
#include "ShaderCompilerDXC.h"
#include "SoundManager.h"
#include "Unknwn.h"
#include "Utility.h"
#include "VertexBuffer.h"
#include "WVPBuffer.h"
#include "WVPManager.h"
#include "WinApp.h"
#include "pipelineBuilder.h"

// ======================= リンカオプション =========================
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")

//------------------
// グローバル定数
//------------------
const int kSubdivision = 16; // 16分割
int kNumVertices = kSubdivision * kSubdivision * 6; // 頂点数
float waveTime = 0.0f;
const int32_t kClientWidth = 1280;
const int32_t kClientHeight = 720;
//////////////---------------------------------------
// 関数の作成///
//////////////

// データを転送するUploadTextureData関数を作る03_00EX
//[[nodiscard]] // 03_00EX
// 球の頂点生成関数_05_00_OTHER新しい書き方
// D3Dリソースリークチェック用のクラス
struct D3DResourceLeakChecker {
    ~D3DResourceLeakChecker()
    {
        Microsoft::WRL::ComPtr<IDXGIDebug1> debug;

        // DXGIのデバッグインターフェースを取得
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
            // DXGI全体のリソースチェック（アプリが作ったリソースがまだ残ってるか確認）
            debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
            debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
            debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
        }
    }
};

// CG2_05_01_page_5
D3D12_CPU_DESCRIPTOR_HANDLE
GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap,
    uint32_t descriptorSize, uint32_t index)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (descriptorSize * index);
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE
GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap,
    uint32_t descriptorSize, uint32_t index)
{
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (descriptorSize * index);
    return handleGPU;
}

////////////////
// main関数/////
///////////////

//  Windwsアプリでの円とリポウント(main関数)

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    D3DResourceLeakChecker leakChecker; // リソースリークチェック用。スコープ終了時に自動でチェックされる

    WinApp win; // ウィンドウ管理クラスのインスタンス（WinMainで必要）

    DeviceManager deviceManager; // DirectXデバイス管理クラスのインスタンス（初期化や解放などを行う）

    HRESULT hr; // 各種DirectX関数の戻り値用。ローカルスコープで十分だが、複数関数で使い回すためここで宣言

    Dxc dxc;

    BlendStateHelper psoDesc;

    RasterizerStateHelper rasterizer;

    PipelineBuilder builder;

    VertexBuffer vertexBuffer;

    RenderHelper render(deviceManager);

    MaterialBuffer materialBuffer;

    WVPBuffer wvpBuffer;
    DescriptorHeapWrapper dsvHeap;
    DescriptorHeapWrapper srvHeap;
    WVPManager wvpManager;

    CoInitializeEx(0, COINIT_MULTITHREADED);

    // 誰も補足しなかった場合(Unhandled),補足する関数を登録
    // main関数はじまってすぐに登録するとよい
    SetUnhandledExceptionFilter(Utility::ExportDump);
    // ログのディレクトリを用意
    std::filesystem::create_directory("logs");
    // main関数の先頭//

    // 現在時刻を取得(UTC時刻)
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    // ログファイルの名前にコンマ何秒はいらないので削って秒にする
    std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
        nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    // 日本時間(PCの設定時間)に変換
    std::chrono::zoned_time loacalTime { std::chrono::current_zone(), nowSeconds };
    // formatを使って年月日_時分秒の文字列に変換
    std::string dateString = std::format("{:%Y%m%d_%H%M%S}", loacalTime);
    // 時刻を使ってファイル名を決定
    std::string logFilePath = std::string("logs/") + dateString + ".log";
    // ファイルを作って書き込み準備
    std::ofstream logStream(logFilePath);
    std::wstring title = L"CG2 EngineGOD"; // ← ここでウィンドウタイトルを定義

    // 出力
    win.Initialize(hInstance, nCmdShow, title, kClientWidth, kClientHeight);

#ifdef _DEBUG

    Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr; // COM
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        // デバックレイヤーを有効化する
        debugController->EnableDebugLayer();
        // さらに6PU側でもチェックリストを行うようにする
        debugController->SetEnableGPUBasedValidation(TRUE);
    }
#endif // _DEBUG

    deviceManager.Initialize(logStream, &win, kClientWidth, kClientHeight);

#ifdef _DEBUG

    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
    if (SUCCEEDED(deviceManager.GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
        // やばいエラー時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        // エラー時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        // 警告時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
        // 抑制するメッセージのＩＤ
        D3D12_MESSAGE_ID denyIds[] = {
            // windows11でのDXGIデバックレイヤーとDX12デバックレイヤーの相互作用バグによるエラーメッセージ
            // https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
        };
        // 抑制するレベル
        D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_INFO_QUEUE_FILTER filter {};
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;
        // 指定したメッセージの表示wp抑制する
        infoQueue->PushStorageFilter(&filter);
        // 解放
    }

#endif // DEBUG

    //  ----------------------------
    //  DirectX12 初期化ここまで！
    //  ----------------------------

    dsvHeap.Create(deviceManager.GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
    srvHeap.Create(deviceManager.GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

    // 初期値でFenceを作る01_02
    Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr; // com
    uint64_t fenceValue = 0;
    hr = deviceManager.GetDevice()->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&fence));
    assert(SUCCEEDED(hr));

    // FenceのSignalを待つためのイベントを作成する01_02
    HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent != nullptr);

    // DXC初期化
    dxc.Initialize();

    // シェーダーで使うリソースの接続設定（ルートシグネチャ）を生成
    auto rootSignature = RootSignatureHelper::CreateDefaultRootSignature(deviceManager.GetDevice(), logStream);

    ///==============================
    /// テクスチャ読み込み & 転送
    ///==============================

    // --- 1枚目のテクスチャ（固定ファイル）
    DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = CreateTextureResource(deviceManager.GetDevice(), metadata);
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = UploadTextureData(textureResource.Get(), mipImages, deviceManager.GetDevice(), deviceManager.GetCommandList());

    // --- モデル読み込み
    ModelData modelData = LoadObjFile("resources", "Plane.obj"); // model読み込み01

    // --- 2枚目のテクスチャ（モデルのマテリアルから取得）
    DirectX::ScratchImage mipImages2 = LoadTexture(modelData.material.textureFilePath);
    const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = CreateTextureResource(deviceManager.GetDevice(), metadata2);
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource2 = UploadTextureData(textureResource2.Get(), mipImages2, deviceManager.GetDevice(), deviceManager.GetCommandList());

    ///==============================
    /// ディスクリプタサイズ取得
    ///==============================
    const uint32_t descriptorSizeSRV = deviceManager.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    const uint32_t descriptorSizeRTV = deviceManager.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    const uint32_t descriptorSizeDSV = deviceManager.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    ///==============================
    /// SRV 設定
    ///==============================

    // --- 1枚目
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc {};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

    // --- 2枚目
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2 {};
    srvDesc2.Format = metadata2.format;
    srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

    // --- SRVハンドル取得
    D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = GetCPUDescriptorHandle(srvHeap.GetHeap(), descriptorSizeSRV, 1);
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = GetGPUDescriptorHandle(srvHeap.GetHeap(), descriptorSizeSRV, 1);

    D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(srvHeap.GetHeap(), descriptorSizeSRV, 2);
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(srvHeap.GetHeap(), descriptorSizeSRV, 2);

    // --- SRV作成
    deviceManager.GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);
    deviceManager.GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);

    ///==============================
    /// InputLayout 設定
    ///==============================

    // InputLayout を取得
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = InputLayoutHelper::CreatePosTexNormLayout();

    // blendStateのせってい
    psoDesc.CreateWriteAll();
    // rasterizerStateの設定
    rasterizer.CreateDefault();

    // Shaderをコンパイルする//これまだクラス化しない
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(L"Object3d.VS.hlsl", L"vs_6_0", dxc.GetUtils(), dxc.GetCompiler(), dxc.GetIncludeHandler(), logStream);
    assert(vertexShaderBlob != nullptr);

    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(L"Object3d.PS.hlsl", L"ps_6_0", dxc.GetUtils(), dxc.GetCompiler(), dxc.GetIncludeHandler(), logStream);
    assert(pixelShaderBlob != nullptr);

    // PSO生成
    builder.SetRootSignature(rootSignature.Get());
    builder.SetInputLayout(inputLayoutDesc);
    builder.SetVertexShader(vertexShaderBlob.Get());
    builder.SetPixelShader(pixelShaderBlob.Get());
    builder.SetBlendState(psoDesc.CreateWriteAll());
    builder.SetRasterizerState(rasterizer.CreateDefault());

    D3D12_DEPTH_STENCIL_DESC dsDesc {};
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    builder.SetDepthStencilState(dsDesc);

    builder.SetRTVFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
    builder.SetDSVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);
    builder.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = builder.Build(deviceManager.GetDevice());

    //====================
    // 獲物
    //====================

    //--------------------------
    // 通常モデル用リソース
    //--------------------------

    //--------------------------------------------------
    // modelDataを使う
    //--------------------------------------------------

    vertexBuffer.Initialize(deviceManager.GetDevice(), modelData.vertices); // model読み込み02
    //--------------------------
    //  マテリアル
    //--------------------------
    // マテリアルデータの初期化（CPU側）
    // 今回は赤色・単位UV変換・ライティング有効を設定
    Material materialData = {};
    materialData.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白色（R=1, G=1, B=1, A=1）
    materialData.uvTransform = MatrixMath::MakeIdentity4x4(); // UV変換行列は単位行列
    materialData.enableLighting = true; // ライティングON
    // マテリアル用の定数バッファをGPUに作成＆データを転送
    materialBuffer.Create(deviceManager.GetDevice(), materialData);

    //--------------------------
    // WVP行列
    //--------------------------
    wvpBuffer.Create(deviceManager.GetDevice());
    //--------------------------
    // Sprite用リソース
    //--------------------------
    // sprite用の頂点リソースを作る04_00
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = CreateBufferResource(deviceManager.GetDevice(), sizeof(VertexData) * 4);
    // sprite用の頂点リソースにデータを書き込む04_00
    VertexData* vertexDataSprite = nullptr;
    //  書き込むためのアドレスを取得04_00
    vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite)); // 04_00
    // 6頂点を4頂点にする
    vertexDataSprite[0].position = { 0.0f, 360.0f, 0.0f, 1.0f }; // 左下
    vertexDataSprite[0].texcoord = { 0.0f, 1.0f };

    vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f }; // 左上
    vertexDataSprite[1].texcoord = { 0.0f, 0.0f };

    vertexDataSprite[2].position = { 640.0f, 360.0f, 0.0f, 1.0f }; // 右下
    vertexDataSprite[2].texcoord = { 1.0f, 1.0f };

    vertexDataSprite[3].position = { 640.0f, 0.0f, 0.0f, 1.0f }; // 右上
    vertexDataSprite[3].texcoord = { 1.0f, 0.0f };

    // sprite用の頂点バッファビューを作成する04_00
    D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite {};
    // sprite用のリソースの先頭のアドレスから使う04_00
    vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
    // sprite用の使用するリーソースのサイズは頂点6つ分のサイズ04_00//06_00ここ４にしました
    vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
    // sprite用の１頂点当たりのサイズ04_00
    vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

    // 06_00_page6
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = CreateBufferResource(deviceManager.GetDevice(), sizeof(uint32_t) * 6);
    uint32_t* indexDataSprite = nullptr;
    // インデックスリソースにデータを書き込む uint32_t *indexDataSprite =
    // nullptr;
    indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
    indexDataSprite[0] = 0;
    indexDataSprite[1] = 1;
    indexDataSprite[2] = 2;
    indexDataSprite[3] = 1;
    indexDataSprite[4] = 3;
    indexDataSprite[5] = 2;

    // Viewを作成する06_00_page6
    D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite {};
    // リソースの先頭のアドレスから使う
    indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
    // 使用するリソースのサイズはインデックス６つ分のサイズ
    indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
    // インデックスはuint32_tとする
    indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

    // Sprite用のマテリアルリソースを作る05_03
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = CreateBufferResource(deviceManager.GetDevice(), sizeof(Material));
    // Sprite用のマテリアルにデータを書き込む
    Material* materialDataSprite = nullptr;
    // 書き込むためのアドレスを取得
    materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
    // 今回は白を設定
    materialDataSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialDataSprite->uvTransform = MatrixMath::MakeIdentity4x4(); // 06_01//同じ
    materialDataSprite->enableLighting = false; // kokomonstrball?

    // sprite用のTransfomationMatrix用のリソースを作る。Matrix4x4
    // 1つ分のサイズを用意する04_00
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = CreateBufferResource(deviceManager.GetDevice(), sizeof(TransformationMatrix));
    // sprite用のデータを書き込む04_00
    TransformationMatrix* transformationMatrixDataSprite = nullptr;
    // sprite用の書き込むためのアドレスを取得04_00
    transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
    // 単位行列を書き込んでおく04_00//これいったん消しました05_03
    // *transformationMatrixDataSprite = MakeIdentity4x4();

    //--------------------------
    // 共通リソース
    //--------------------------
    // 03_01_Other
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencillResource = CreateDepthStencilTextureResource(deviceManager.GetDevice(), kClientWidth, kClientHeight);
    // DSVの設定
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    // DSVHeapの先端にDSVを作る
    deviceManager.GetDevice()->CreateDepthStencilView(depthStencillResource.Get(), &dsvDesc, dsvHeap.GetCPUHandleStart());

    // 平行光源用の定数バッファ（CBV）を作成（バッファサイズは構造体に合わせる）05_03
    Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = CreateBufferResource(deviceManager.GetDevice(), sizeof(DirectionalLight));
    // 平行光源用のデータを書き込み
    DirectionalLight* directionalLightData = nullptr;
    directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
    directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白色光
    directionalLightData->direction = MatrixMath::Normalize({ 0.0f, -1.0f, 0.0f }); // 真上から下方向
    directionalLightData->intensity = 1.0f; // 標準の明るさ

    //--------------------------
    // その他リソース
    //--------------------------
    //   ビューポート
    D3D12_VIEWPORT viewport {};
    // クライアント領域のサイズと一緒にして画面全体に表示/
    viewport.Width = kClientWidth;
    viewport.Height = kClientHeight;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    // シザー矩形
    D3D12_RECT scissorRect {};
    // 基本的にビューポートと同じ矩形が構成されるようにする
    scissorRect.left = 0;
    scissorRect.right = kClientWidth;
    scissorRect.top = 0;
    scissorRect.bottom = kClientHeight;

    // 変数//
    // spriteトランスフォーム
    Transform transformSprite {
        { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }
    };
    // トランスフォーム
    Transform transform {
        { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }
    };
    // カメラトランスフォーム
    Transform cameraTransform {
        { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -5.0f }
    };
    // UVTransform用の変数を用意
    Transform uvTransformSprite {
        { 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
    };

    // Textureの切り替え
    bool useMonstarBall = true;
    //
    // ImGuiの初期化。詳細はさして重要ではないので解説は省略する。
    // ImGuiの初期化
    win.ImGuiInitialize(deviceManager.GetDevice(), deviceManager.GetRTVDesc().Format, srvHeap.GetHeap(), deviceManager.GetSwapChainDesc().BufferCount);
    // ImGuiの初期化
    // ImGuiの初期化

    MSG msg {};
    //=================================
    // キーボードインスタンス作成
    //=================================
    Input input;
    //=================================
    // キーボード情報の取得開始
    //=================================
    input.Initialize(hInstance, win.GetHwnd());

    //=================================
    // デバックカメラインスタンス作成
    //=================================
    DebugCamera debugCamera;
    // debugcamera初期化一回だけ
    debugCamera.Initialize(hInstance, win.GetHwnd());
    //=================================
    // サウンドマネージャーインスタンス作成
    //=================================
    SoundManager soundmanager;
    // サウンドマネージャー初期化！
    soundmanager.Initialize();
    // サウンドファイルを読み込み（パスはプロジェクトに合わせて調整）
    SoundData bgm = soundmanager.SoundLoadWave("Resources/BGM.wav");

    // ウィンドウの×ボタンが押されるまでループ
    while (msg.message != WM_QUIT) {

        // Windowにメッセージが来てたら最優先で処理させる
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            //=========================== フレーム開始（ImGuiセットアップ） ===========================//
            win.ImGuiBeginFrame();

            //=========================== ImGui ウィンドウ（デバッグUI） ===========================//

            ImGui::Begin("Materialcolor");
            ImGui::SliderFloat3("Scale", &transform.scale.x, 0.1f, 5.0f);
            ImGui::SliderAngle("RotateX", &transform.rotate.x, -180.0f, 180.0f);
            ImGui::SliderAngle("RotateY", &transform.rotate.y, -180.0f, 180.0f);
            ImGui::SliderAngle("RotateZ", &transform.rotate.z, -180.0f, 180.0f);
            ImGui::SliderFloat3("Translate", &transform.translate.x, -5.0f, 5.0f);
            ImGui::Text("useMonstarBall");
            ImGui::Checkbox("useMonstarBall", &useMonstarBall);
            ImGui::Text("Lighting");
            ImGui::SliderFloat("x", &directionalLightData->direction.x, -10.0f, 10.0f);
            ImGui::SliderFloat("y", &directionalLightData->direction.y, -10.0f, 10.0f);
            ImGui::SliderFloat("z", &directionalLightData->direction.z, -10.0f, 10.0f);
            ImGui::Text("UVTransform");
            ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
            ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
            ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
            ImGui::End();

            //=========================== ゲーム処理 ===========================//
            waveTime += 0.05f;
            input.Update();
            debugCamera.Update();
            if (input.IsKeyPressed(DIK_0)) {
                OutputDebugStringA("Hit 0");
                soundmanager.SoundPlayWave(bgm);
            }

            //=========================== 行列計算（モデル・スプライト・UV） ===========================//

            // 描画と移動に必要なWVP行列を関数化(これmodelです)
            wvpManager.Update(transform, debugCamera, kClientWidth, kClientHeight);
            wvpBuffer.Update(wvpManager.GetWVPMatrix(), wvpManager.GetWorldMatrix()); // ここでWVPとWORLDに入れる

            Matrix4x4 worldMatrixSprite = MatrixMath::MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
            Matrix4x4 viewMatrixSprite = MatrixMath::MakeIdentity4x4();
            Matrix4x4 projectionMatrixSprite = MatrixMath::MakeOrthographicMatrix(0.0f, 0.0f, float(kClientWidth), float(kClientHeight), 0.0f, 100.0f);
            Matrix4x4 worldViewProjectionMatrixSprite = MatrixMath::Multiply(worldMatrixSprite, MatrixMath::Multiply(viewMatrixSprite, projectionMatrixSprite));
            transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
            transformationMatrixDataSprite->World = worldMatrixSprite;

            Matrix4x4 uvTransformMatrix = MatrixMath::Matrix4x4MakeScaleMatrix(uvTransformSprite.scale);
            uvTransformMatrix = MatrixMath::Multiply(uvTransformMatrix, MatrixMath::MakeRotateZMatrix(uvTransformSprite.rotate.z));
            uvTransformMatrix = MatrixMath::Multiply(uvTransformMatrix, MatrixMath::MakeTranslateMatrix(uvTransformSprite.translate));
            materialDataSprite->uvTransform = uvTransformMatrix;

            //=========================== 描画準備 ===========================//
            float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
            // ここでmaterialDataを更新する
            materialBuffer.Update(materialData); // マイフレーム更新

            render.PreDraw(clearColor, dsvHeap.GetHeap(), viewport, scissorRect, rootSignature.Get(), pipelineState.Get(), srvHeap.GetHeap());

            //=========================== モデル描画 ===========================//
            render.DrawModel(vertexBuffer.GetView(), static_cast<UINT>(modelData.vertices.size()), wvpBuffer.GetGPUVirtualAddress(), materialBuffer.GetResource()->GetGPUVirtualAddress(), directionalLightResource->GetGPUVirtualAddress(), useMonstarBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);

            //=========================== スプライト描画 ===========================//
            render.DrawSprite(vertexBufferViewSprite, indexBufferViewSprite, transformationMatrixResourceSprite->GetGPUVirtualAddress(), materialResourceSprite->GetGPUVirtualAddress(), textureSrvHandleGPU);

            //=========================== ImGui描画 ===========================//
            win.ImGuiEndFrame(deviceManager.GetCommandList());

            //=========================== リソースバリア & Present ===========================//
            render.PostDraw(fence.Get(), fenceEvent, fenceValue);
        }
    }

    // ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
    // こういうもんである。初期化と逆順に行う/
    win.ImGuiShutdown();

    //  // 解放処理CG2_01_03
    CloseHandle(fenceEvent);

    // リリースする場所
    // XAudio解放
    soundmanager.Finalize(&bgm);

    CoInitialize(nullptr);
    // #endif
    CloseWindow(win.GetHwnd());

    return 0;

} // 最後のカギかっこ
