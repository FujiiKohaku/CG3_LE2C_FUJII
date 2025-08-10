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
#include "DirectionalLightBuffer.h"
#include "Dxc.h"
#include "GameSceneManager.h"
#include "IndexBuffer.h"
#include "Input.h"
#include "InputLayoutHelper.h"
#include "Logger.h"
#include "MaterialBuffer.h"
#include "MatrixMath.h"
#include "ModelLoder.h"
#include "RasterizerStateHelper.h"
#include "RenderHelper.h"
#include "RootSignatureHelper.h"
#include "ShaderCompiler.h"
#include "ShaderCompilerDXC.h"
#include "SoundManager.h"
#include "Texture.h"
#include "UVTransformManager.h"
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

////////////////
// main関数/////
///////////////

//  Windwsアプリでの円とリポウント(main関数)

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    D3DResourceLeakChecker leakChecker; // リソースリークチェック用。スコープ終了時に自動でチェックされる

    GameSceneManager gameSceneManager;

    auto& deviceManager = gameSceneManager.GetDeviceManager();
    auto& win = gameSceneManager.GetWinApp(); // ← 以後はこれを使う
    auto& log = gameSceneManager.GetLogger(); // ← 以後はこれを使う

    HRESULT hr; // 各種DirectX関数の戻り値用。ローカルスコープで十分だが、複数関数で使い回すためここで宣言

    VertexBuffer vertexBuffer;

    MaterialBuffer materialBuffer;

    // 3Dオブジェクト用
    WVPBuffer wvpBufferObject;
    WVPManager wvpManagerObject;

    // 2Dスプライト用
    WVPBuffer wvpBufferSprite;
    WVPManager wvpManagerSprite;

    UVTransformManager uvManager;

    DirectionalLightBuffer directionalLightBuffer;

    MaterialBuffer spriteMaterial;

    gameSceneManager.Initialize(GetModuleHandle(nullptr), SW_SHOWDEFAULT, L"CG2 EngineGOD", kClientWidth, kClientHeight);

    CoInitializeEx(0, COINIT_MULTITHREADED);

    //  ----------------------------
    //  DirectX12 初期化ここまで！
    //  ----------------------------
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
    ///==============================
    /// ディスクリプタサイズ取得（最初にやると整理しやすい）
    ///==============================
    const uint32_t descriptorSizeSRV = gameSceneManager.GetDescriptorSizeSRV();
    const uint32_t descriptorSizeRTV = gameSceneManager.GetDescriptorSizeRTV();
    const uint32_t descriptorSizeDSV = gameSceneManager.GetDescriptorSizeDSV();
    ///==============================
    /// モデル読み込み
    ///==============================
    ModelData modelData;
    Texture texture2;
    gameSceneManager.LoadModelAndMaterialSRV("resources", "Plane.obj", 2, modelData, texture2);

    // スプライト用テクスチャ
    Texture texture;
    gameSceneManager.LoadTextureAndMakeSRV("resources/uvChecker.png", 1, texture);

    ///==============================
    /// SRV 作成
    ///==============================

    // --- 1枚目のSRV
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = gameSceneManager.GetSRVCPUHandle(1);
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = gameSceneManager.GetSRVGPUHandle(1);
    texture.CreateSRV(deviceManager.GetDevice(), cpuHandle, gpuHandle);

    // --- 2枚目のSRV
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle2 = gameSceneManager.GetSRVCPUHandle(2);
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle2 = gameSceneManager.GetSRVGPUHandle(2);
    texture2.CreateSRV(deviceManager.GetDevice(), cpuHandle2, gpuHandle2);

    ///==============================
    /// InputLayout 設定
    ///==============================

    ///==============================
    /// 3D（通常モデル）リソース
    ///==============================

    // 頂点データ（modelDataを使う）
    vertexBuffer.Initialize(deviceManager.GetDevice(), modelData.vertices); // model読み込み02

    // マテリアル（白・UV単位・ライティングON）
    Material materialData = {};
    materialData.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData.uvTransform = MatrixMath::MakeIdentity4x4();
    materialData.enableLighting = true;
    // マテリアル用定数バッファ作成＆初期データ転送
    materialBuffer.Create(deviceManager.GetDevice(), materialData);

    // WVP行列（3D用）
    wvpBufferObject.Create(deviceManager.GetDevice());

    ///==============================
    /// 2D（スプライト）リソース
    ///==============================

    // スプライト用頂点（矩形）
    std::vector<VertexData> spriteVertices(4);
    // 左下
    spriteVertices[0].position = { 0.0f, 360.0f, 0.0f, 1.0f };
    spriteVertices[0].texcoord = { 0.0f, 1.0f };
    // 左上
    spriteVertices[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
    spriteVertices[1].texcoord = { 0.0f, 0.0f };
    // 右下
    spriteVertices[2].position = { 640.0f, 360.0f, 0.0f, 1.0f };
    spriteVertices[2].texcoord = { 1.0f, 1.0f };
    // 右上
    spriteVertices[3].position = { 640.0f, 0.0f, 0.0f, 1.0f };
    spriteVertices[3].texcoord = { 1.0f, 0.0f };

    // スプライト用VB
    VertexBuffer spriteVertexBuffer;
    spriteVertexBuffer.Initialize(deviceManager.GetDevice(), spriteVertices);

    // スプライト用IB（2三角形）
    std::vector<uint32_t> spriteIndices = {
        0, 1, 2, // 左下→左上→右下
        1, 3, 2 // 左上→右上→右下
    };
    IndexBuffer indexBufferSprite;
    indexBufferSprite.Initialize(deviceManager.GetDevice(), spriteIndices);

    // スプライト用マテリアル（白・UV単位・ライティングOFF）
    Material materialDataSprite = {};
    materialDataSprite.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialDataSprite.uvTransform = MatrixMath::MakeIdentity4x4();
    materialDataSprite.enableLighting = false;
    spriteMaterial.Create(deviceManager.GetDevice(), materialDataSprite);

    // スプライト用Transform定数バッファ（必要に応じて使用）
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = CreateBufferResource(deviceManager.GetDevice(), sizeof(TransformationMatrix));
    TransformationMatrix* transformationMatrixDataSprite = nullptr;
    transformationMatrixResourceSprite->Map( 0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));

    // WVP行列（スプライト用）
    wvpBufferSprite.Create(deviceManager.GetDevice());

    ///==============================
    /// 共通リソース
    ///==============================

    // 平行光源バッファ
    directionalLightBuffer.Initialize(deviceManager.GetDevice());


    // サウンドファイルを読み込み（パスはプロジェクトに合わせて調整）
    SoundData bgm = gameSceneManager.GetSoundManager().SoundLoadWave("Resources/BGM.wav");

    MSG msg {};
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
            ImGui::Text("Lighting");
            ImGui::SliderFloat("x", &directionalLightBuffer.GetData()->direction.x, -10.0f, 10.0f);
            ImGui::SliderFloat("y", &directionalLightBuffer.GetData()->direction.y, -10.0f, 10.0f);
            ImGui::SliderFloat("z", &directionalLightBuffer.GetData()->direction.z, -10.0f, 10.0f);
            ImGui::Text("UVTransform");
            ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
            ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
            ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
            ImGui::End();

            //=========================== ゲーム処理 ===========================//
            waveTime += 0.05f;
            gameSceneManager.GetInput().Update();
            gameSceneManager.GetDebugCamera().Update();
            if (gameSceneManager.GetInput().IsKeyPressed(DIK_0)) {
                OutputDebugStringA("Hit 0");
                gameSceneManager.GetSoundManager().SoundPlayWave(bgm);
            }

            //=========================== 行列計算（モデル・スプライト・UV） ===========================//

            // 3D
            wvpManagerObject.Update(transform, gameSceneManager.GetDebugCamera(), (float)kClientWidth, (float)kClientHeight);
            wvpBufferObject.Update(wvpManagerObject.GetWVPMatrix(), wvpManagerObject.GetWorldMatrix());

            // 2D（スプライト）
            wvpManagerSprite.Update(transformSprite, (float)kClientWidth, (float)kClientHeight);
            wvpBufferSprite.Update(wvpManagerSprite.GetWVPMatrix(), wvpManagerSprite.GetWorldMatrix());

            // UV → マテリアル
            uvManager.Update(uvTransformSprite);
            materialDataSprite.uvTransform = uvManager.GetMatrix();

            //=========================== 描画準備 ===========================//
            float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
            // ここでmaterialDataを更新する
            materialBuffer.Update(materialData); // マイフレーム更新
            spriteMaterial.Update(materialDataSprite); // スプライトのマテリアル更新

            gameSceneManager.PreDraw(clearColor, gameSceneManager.GetViewportRef(), gameSceneManager.GetScissorRectRef());

            //=========================== モデル描画 ===========================//
            gameSceneManager.GetRender()->DrawModel(vertexBuffer.GetView(), static_cast<UINT>(modelData.vertices.size()), wvpBufferObject.GetGPUVirtualAddress(), materialBuffer.GetResource()->GetGPUVirtualAddress(), directionalLightBuffer.GetGPUVirtualAddress(), texture2.GetGpuHandle());

            //=========================== スプライト描画 ===========================//
            gameSceneManager.GetRender()->DrawSprite(spriteVertexBuffer.GetView(), indexBufferSprite.GetView(), wvpBufferSprite.GetGPUVirtualAddress(), spriteMaterial.GetResource()->GetGPUVirtualAddress(), texture.GetGpuHandle());

            //=========================== ImGui描画 ===========================//
            gameSceneManager.EndFrame();

            //=========================== リソースバリア & Present ===========================//
            gameSceneManager.PostDraw();
        }
    }

    // ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
    // こういうもんである。初期化と逆順に行う/
    win.ImGuiShutdown();

    //  // 解放処理CG2_01_03
    CloseHandle(gameSceneManager.GetFenceEvent());

    // リリースする場所
    // XAudio解放
    gameSceneManager.GetSoundManager().Finalize(&gameSceneManager.GetBGM());

    CoInitialize(nullptr);
    // #endif
    CloseWindow(win.GetHwnd());

    return 0;

} // 最後のカギかっこ
