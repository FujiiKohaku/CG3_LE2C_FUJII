// ======================= ヘッダー・ライブラリ関連 ==========================
#define _USE_MATH_DEFINES
// 標準ライブラリ//
#include "D3DResourceLeakChecker.h"
#include "DebugCamera.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "MatrixMath.h"
#include "Object3D.h"
#include "Object3dManager.h"
#include "SoundManager.h"
#include "Sprite.h"
#include "SpriteManager.h"
#include "TextureManager.h"
#include "Unknwn.h"
#include "Utility.h"
#include "Winapp/WinApp.h"
#include <cassert>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <wrl.h>
// Windows・DirectX関連
#include <Windows.h> //ウィンドウAPIで消す
#include <d3d12.h>
#include <dbghelp.h>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
// 外部ライブラリ//
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
// リンカオプション
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")
// ======================= ImGui用ウィンドウプロシージャ =====================
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);
// ======================= 基本構造体 =====================

// 変換情報
struct Transform {
    Vector3 scale;
    Vector3 rotate;
    Vector3 translate;
};
// 頂点、マテリアル関連
struct VertexData {
    Vector4 position;
    Vector2 texcoord;
    Vector3 normal;
};
struct Material {
    Vector4 color;
    int32_t enableLighting;
    float padding[3];
    Matrix4x4 uvTransform;
};
struct TransformationMatrix {
    Matrix4x4 WVP;
    Matrix4x4 World;
};
struct DirectionalLight {
    Vector4 color;
    Vector3 direction;
    float intensity;
};
// パーティクル等
struct Fragment {
    Vector3 position;
    Vector3 velocity;
    Vector3 rotation;
    Vector3 rotationSpeed;
    float alpha;
    bool active;
};

// モデルデータ
struct MaterialData {
    std::string textureFilePath;
};

struct ModelData {
    std::vector<VertexData> vertices;
    MaterialData material;
};

//------------------
// グローバル定数
//------------------
const int kSubdivision = 16; // 16分割
int kNumVertices = kSubdivision * kSubdivision * 6; // 頂点数
float waveTime = 0.0f;

;
//////////////
// 関数の作成///
//////////////

// 球の頂点生成関数_05_00_OTHER新しい書き方
void GenerateSphereVertices(VertexData* vertices, int kSubdivision,
    float radius)
{
    // 経度(360)
    const float kLonEvery = static_cast<float>(M_PI * 2.0f) / kSubdivision;
    // 緯度(180)
    const float kLatEvery = static_cast<float>(M_PI) / kSubdivision;

    for (int latIndex = 0; latIndex < kSubdivision; ++latIndex) {
        float lat = -static_cast<float>(M_PI) / 2.0f + kLatEvery * latIndex;
        float nextLat = lat + kLatEvery;

        for (int lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
            float lon = kLonEvery * lonIndex;
            float nextLon = lon + kLonEvery;

            // verA
            VertexData vertA;
            vertA.position = { cosf(lat) * cosf(lon), sinf(lat), cosf(lat) * sinf(lon),
                1.0f };
            vertA.texcoord = { static_cast<float>(lonIndex) / kSubdivision,
                1.0f - static_cast<float>(latIndex) / kSubdivision };
            vertA.normal = { vertA.position.x, vertA.position.y, vertA.position.z };

            // verB
            VertexData vertB;
            vertB.position = { cosf(nextLat) * cosf(lon), sinf(nextLat),
                cosf(nextLat) * sinf(lon), 1.0f };
            vertB.texcoord = { static_cast<float>(lonIndex) / kSubdivision,
                1.0f - static_cast<float>(latIndex + 1) / kSubdivision };
            vertB.normal = { vertB.position.x, vertB.position.y, vertB.position.z };

            // vertC
            VertexData vertC;
            vertC.position = { cosf(lat) * cosf(nextLon), sinf(lat),
                cosf(lat) * sinf(nextLon), 1.0f };
            vertC.texcoord = { static_cast<float>(lonIndex + 1) / kSubdivision,
                1.0f - static_cast<float>(latIndex) / kSubdivision };
            vertC.normal = { vertC.position.x, vertC.position.y, vertC.position.z };

            // vertD
            VertexData vertD;
            vertD.position = { cosf(nextLat) * cosf(nextLon), sinf(nextLat),
                cosf(nextLat) * sinf(nextLon), 1.0f };
            vertD.texcoord = { static_cast<float>(lonIndex + 1) / kSubdivision,
                1.0f - static_cast<float>(latIndex + 1) / kSubdivision };
            vertD.normal = { vertD.position.x, vertD.position.y, vertD.position.z };

            // 初期位置//
            uint32_t startIndex = (latIndex * kSubdivision + lonIndex) * 6;

            vertices[startIndex + 0] = vertA;
            vertices[startIndex + 1] = vertB;
            vertices[startIndex + 2] = vertC;
            vertices[startIndex + 3] = vertC;
            vertices[startIndex + 4] = vertD;
            vertices[startIndex + 5] = vertB;
        }
    }
}

////////////////
// main関数/////-------------------------------------------------------------------------------------------------
//  Windwsアプリでの円とリポウント(main関数)

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{

    // HRESULT hr;
    D3DResourceLeakChecker leakChecker;

    // 誰も補足しなかった場合(Unhandled),補足する関数を登録
    // main関数はじまってすぐに登録するとよい
    SetUnhandledExceptionFilter(Utility::ExportDump);
    // ログのディレクトリを用意
    std::filesystem::create_directory("logs");
    // main関数の先頭//

#pragma region ログファイルの準備

#pragma endregion

    // ==========================
    // ① WinApp・DirectX初期化
    // ==========================
    WinApp* winApp = new WinApp();
    winApp->initialize();

    DirectXCommon* dxCommon = new DirectXCommon();
    dxCommon->Initialize(winApp);

    // ==========================
    // ② TextureManager初期化
    // ==========================
    TextureManager::GetInstance()->Initialize(dxCommon);
    TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");
    // SpriteManagerのポインタ
    SpriteManager* spriteManager = nullptr;
    // スプライト共通部の初期化
    spriteManager = new SpriteManager();
    spriteManager->Initialize(dxCommon);

    // spriteのポインタ
    Sprite* sprite = nullptr;

    // スプライト個人の初期化
    std::vector<Sprite*> sprites;
    for (uint32_t i = 0; i < 5; i++) {
        Sprite* sprite = new Sprite();
        sprite->Initialize(spriteManager, "resources/uvChecker.png");
        Vector2 pos = { 100.0f * i, 100.0f };
        sprite->SetPosition(pos);
        sprites.push_back(sprite);
    }

    Object3dManager* object3dManager = nullptr;
    // 3Dオブジェクト共通部の初期化
    object3dManager = new Object3dManager();
    object3dManager->Initialize(dxCommon);

    //=================================
    // デバックカメラインスタンス作成
    //=================================
    DebugCamera debugCamera;
    // debugcamera初期化一回だけ
    debugCamera.Initialize(winApp);

    Object3d object3d;
    object3d.Initialize(object3dManager, debugCamera);

#ifdef _DEBUG

    Microsoft::WRL::ComPtr<ID3D12InfoQueue>
        infoQueue = nullptr;
    if (SUCCEEDED(dxCommon->GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
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
        /*  infoQueue->Release();*/
    }

#endif // DEBUG

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

    MSG msg {};
    //=================================
    // キーボードインスタンス作成
    //=================================
    Input* input = nullptr;
    input = new Input();
    //=================================
    // キーボード情報の取得開始
    //=================================
    input->Initialize(winApp);

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
        if (winApp->ProcessMessage()) {
            break;
        } else {

            // ここがframeの先頭02_03
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            //===================================

            // 開発用UIの処理。実際に開発用のUIを出す場合はここをげ０無固有の処理を置き換える02_03
            // ImGui::ShowDemoWindow(); // ImGuiの始まりの場所-----------------------------

            ImGui::Begin("Materialcolor");
            ImGui::SliderFloat3("Scale", &transform.scale.x, 0.1f, 5.0f);
            ImGui::SliderAngle("RotateX", &transform.rotate.x, -180.0f, 180.0f);
            ImGui::SliderAngle("RotateY", &transform.rotate.y, -180.0f, 180.0f);
            ImGui::SliderAngle("RotateZ", &transform.rotate.z, -180.0f, 180.0f);
            ImGui::SliderFloat3("Translate", &transform.translate.x, -5.0f, 5.0f);

            /* ImGui::ColorEdit4("Color", &(*materialData).x);*/
            ImGui::Text("useMonstarBall");
            ImGui::Checkbox("useMonstarBall", &useMonstarBall);
            ImGui::Text("LIgthng");
            /*           ImGui::SliderFloat("x", &directionalLightData->direction.x, -10.0f, 10.0f);
                       ImGui::SliderFloat("y", &directionalLightData->direction.y, -10.0f, 10.0f);
                       ImGui::SliderFloat("z", &directionalLightData->direction.z, -10.0f, 10.0f);*/
            ImGui::Text("UVTransform");
            ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
            ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
            ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);

            ImGui::End();

            // ImGuiの内部コマンドを生成する02_03
            ImGui::Render(); // ImGui終わりの場所。描画の前02_03--------------------------

            //===================================
            //  ゲームの処理02_02
            //===================================
            //  02_02
            waveTime += 0.05f;
            // インプットの更新
            input->Update();
            // デバッグカメラの更新
            debugCamera.Update();
            for (Sprite* sprite : sprites) {
                sprite->Update();
            }

            object3d.Update();



            dxCommon->PreDraw();
            

            // 3Dオブジェクトの描画準備
            object3dManager->PreDraw();
            // 画面のクリア処理

            object3d.Draw();

            // spriteの描画準備
            spriteManager->PreDraw();
            /* for (Sprite* sprite : sprites) {
                 sprite->Draw();
             }*/
            // 描画の最後です//----------------------------------------------------
            //  実際のcommandListのImGuiの描画コマンドを積む
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());
            dxCommon->PostDraw();
            Logger::Log("CommandList state check before Close()");
        }
    }

    // ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
    // こういうもんである。初期化と逆順に行う/
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // 解放処理CG2_01_03
    // CloseHandle();
    // ディスクリプタとSRVの設計見直す

    // リリースする場所
    // XAudio解放
    soundmanager.Finalize(&bgm);

    delete object3dManager;
    // === 1. SpriteやModelなど「描画で使うオブジェクト」を削除 ===
    for (auto sprite : sprites) {
        delete sprite;
    }
    sprites.clear();
    delete spriteManager;

    // === 2. TextureManager（GPUリソース管理者）を最後に破棄 ===
    TextureManager::GetInstance()->Finalize();

    // === 3. DirectX系の破棄はTextureより後 ===
    delete dxCommon;

    // === 4. 最後にWinAppを終了 ===
    winApp->Finalize();
    delete winApp;

    return 0;

} // 最後のカギかっこ
