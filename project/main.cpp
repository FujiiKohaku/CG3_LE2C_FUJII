// ======================= ヘッダー・ライブラリ関連 ==========================
#define _USE_MATH_DEFINES
// 標準ライブラリ//
#include "DebugCamera.h"
#include "DirectXCommon/DirectXCommon.h"
#include "Input.h"
#include "MatrixMath.h"
#include "SoundManager.h"
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

/// CG_02_06
MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
    // 1.中で必要となる変数の宣言
    MaterialData materialData; // 構築するMaterialData
    // 2.ファイルを開く
    std::string line; // ファイルから読んだ１行を格納するもの
    std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
    assert(file.is_open()); // とりあえず開けなかったら止める
    // 3.実際にファイルを読み、MaterialDataを構築していく
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;
        // identifierに応じた処理
        if (identifier == "map_Kd") {
            std::string textureFilename;
            s >> textureFilename;
            // 連結してファイルパスにする
            materialData.textureFilePath = directoryPath + "/" + textureFilename;
        }
    }
    // 4.materialDataを返す
    return materialData;
}
// std::stringは文字列を扱う
// 06_02
ModelData LoadOjFile(const std::string& directoryPath, const std::string& filename)
{

    // 1.中で必要となる変数の宣言
    ModelData modelData; // 構築するModelData
    std::vector<Vector4> positions; // 位置
    std::vector<Vector3> normals; // 法線
    std::vector<Vector2> texcoords; // テクスチャ座標
    std::string line; // ファイルから読んだ一行を格納するもの

    // 2.ファイルを開く
    std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
    assert(file.is_open()); // とりあえず開けなかったら止める

    // 3.実際にファイルを読み,ModelDataを構築していく
    while (std::getline(file, line)) {
        std::string identifiler;
        std::istringstream s(line);
        s >> identifiler; // 先頭の識別子を読む

        // identifierに応じた処理
        if (identifiler == "v") {
            Vector4 position;
            s >> position.x >> position.y >> position.z;
            // 左手座標にする
            position.x *= -1.0f;

            position.w = 1.0f;
            positions.push_back(position);
        } else if (identifiler == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;
            // 上下逆にする

            // texcoord.y *= -1.0f;
            texcoord.y = 1.0f - texcoord.y;
            // CG2_06_02_kusokusosjsusuawihoafwhgiuwhkgfau
            texcoords.push_back(texcoord);
        } else if (identifiler == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            // 左手座標にする
            normal.x *= -1.0f;

            normals.push_back(normal);
        } else if (identifiler == "f") {
            VertexData triangle[3]; // 三つの頂点を保存
            // 面は三角形限定。その他は未対応
            for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
                std::string vertexDefinition;
                s >> vertexDefinition;
                // 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してえIndexを取得する
                std::istringstream v(vertexDefinition);
                uint32_t elementIndices[3];
                for (int32_t element = 0; element < 3; ++element) {
                    std::string index;

                    std::getline(v, index, '/'); // 区切りでインデックスを読んでいく
                    elementIndices[element] = std::stoi(index);
                }
                // 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
                Vector4 position = positions[elementIndices[0] - 1];
                Vector2 texcoord = texcoords[elementIndices[1] - 1];
                Vector3 normal = normals[elementIndices[2] - 1];
                // X軸を反転して左手座標系に

                triangle[faceVertex] = { position, texcoord, normal };
            }
            // 逆順にして格納（2 → 1 → 0）
            modelData.vertices.push_back(triangle[2]);
            modelData.vertices.push_back(triangle[1]);
            modelData.vertices.push_back(triangle[0]);
            //?
        } else if (identifiler == "mtllib") {
            // materialTemplateLibraryファイルの名前を取得する
            std::string materialFilename;
            s >> materialFilename;
            // 基本的にobjファイルと同一階層mtlは存在させるので、ディレクトリ名とファイル名を渡す。
            modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
        }
    }
    // 4.ModelDataを返す
    return modelData;
}

////////////////
// main関数/////-------------------------------------------------------------------------------------------------
//  Windwsアプリでの円とリポウント(main関数)

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{

    HRESULT hr;
    D3DResourceLeakChecker leakChecker;

    // 誰も補足しなかった場合(Unhandled),補足する関数を登録
    // main関数はじまってすぐに登録するとよい
    SetUnhandledExceptionFilter(Utility::ExportDump);
    // ログのディレクトリを用意
    std::filesystem::create_directory("logs");
    // main関数の先頭//

#pragma region ログファイルの準備
    //// 現在時刻を取得(UTC時刻)
    // std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    //// ログファイルの名前にコンマ何秒はいらないので削って秒にする
    // std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
    //     nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    //// 日本時間(PCの設定時間)に変換
    // std::chrono::zoned_time loacalTime { std::chrono::current_zone(), nowSeconds };
    //// formatを使って年月日_時分秒の文字列に変換
    // std::string dateString = std::format("{:%Y%m%d_%H%M%S}", loacalTime);
    //// 時刻を使ってファイル名を決定
    // std::string logFilePath = std::string("logs/") + dateString + ".log";
    //// ファイルを作って書き込み準備
    // std::ofstream logStream(logFilePath);
#pragma endregion

    // WinAppのポインタ
    WinApp* winApp = nullptr;

    // WinAppの初期化
    winApp = new WinApp();
    winApp->initialize();

    // DirectXCommonのポインタ
    DirectXCommon* dxCommon = nullptr;
    // DirectXCommonの初期化
    dxCommon = new DirectXCommon();
    dxCommon->Initialize(winApp);

#ifdef _DEBUG

    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
    if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
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

    //  ルート何？03_00
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature {};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0〜1の範囲外をリピート
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipMapを使う
    staticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う

    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);
    // RootParameter作成。複数設定できるので配列。今回は結果１つだけなので長さ１の配列
    // PixelShaderのMaterialとVertexShaderのTransform
    D3D12_ROOT_PARAMETER rootParameters[4] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
    rootParameters[0].Descriptor.ShaderRegister = 0; // レジスタ番号０とバインド
    // ここから[2]
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // Vertexshaderで使う
    rootParameters[1].Descriptor.ShaderRegister = 0; // 得wジスタ番号０を使う
    // ここまで[2]
    // 新しいディスクリプタレンジ03_00
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0; // 0から始まる
    descriptorRange[0].NumDescriptors = 1; // 数は1つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算
    descriptionRootSignature.pParameters = rootParameters; // ルートパラメータ配列へのポインタ
    descriptionRootSignature.NumParameters = _countof(rootParameters); // 配列の長さ

    // 新しいディスクリプタレンジ03_00
    // ここから[3]03_00
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数
    // ここまで[3]//05_03追加しろー
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PxelShaderで使う
    rootParameters[3].Descriptor.ShaderRegister = 1; // レジスタ番号１を使う
    // ==== シリアライズしてバイナリにする（GPUが読める形に変換） ====
    // バイナリになるデータを入れるための箱02_00
    ID3DBlob* signatureBlob = nullptr; // ルートシグネチャ本体
    ID3DBlob* errorBlob = nullptr; // エラー内容が入るかも
    // GPUが読めるようにデータ変換！（バイナリ化）
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

    // もし失敗したら、エラーメッセージを出して止める
    if (FAILED(hr)) {
        Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer())); // エラーをログに出す
        assert(false); // 絶対成功してないと困るので、止める
    }

    // バイナリをもとに生成02_00
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr; // com
    hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));

    // Textureを読んで転送する03_00
    DirectX::ScratchImage mipImages = dxCommon->LoadTexture("resources/uvChecker.png");
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata); // get
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = dxCommon->UploadTextureData(textureResource.Get(), mipImages); //?
    // モデル読み込み
    ModelData modelData = LoadOjFile("resources", "Plane.obj");

    std::cout << "テクスチャファイルパス: " << modelData.material.textureFilePath
              << std::endl;

    if (!std::filesystem::exists(modelData.material.textureFilePath)) {
        std::cerr << "ファイルが存在しません！" << std::endl;
    }

    // 2枚目のTextureを読んで転送するCG2_05_01_page_8
    DirectX::ScratchImage mipImages2 = dxCommon->LoadTexture(modelData.material.textureFilePath);

    const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata2); // get
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource2 = dxCommon->UploadTextureData(textureResource2.Get(), mipImages2);

    // ID3D12Resource* intermediateResource = dxCommon->UploadTextureData(textureResource, mipImages);

#pragma region ディスクリプタサイズを取得する（SRV/RTV/DSV）
    // DescriptorSizeを取得しておくCG2_05_01_page_6

#pragma endregion

    // metaDataを基にSRVの設定03_00
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc {};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

    // metaData2を基にSRVの設定CG2_05_01_page_9
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2 {};
    srvDesc2.Format = metadata2.format;
    srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
    srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

    // SRVを作成するDescriptorHeapの場所を決める//変更CG2_05_01_0page6
    D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->GetCPUDescriptorHandle(dxCommon->GetSRVDescriptorHeap(), dxCommon->GetSRVDescriptorSize(), 1);
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetGPUDescriptorHandle(dxCommon->GetSRVDescriptorHeap(), dxCommon->GetSRVDescriptorSize(), 1);

    // SRVを作成するDescriptorHeapの場所を決めるCG2_05_01_page_9
    D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = dxCommon->GetCPUDescriptorHandle(dxCommon->GetSRVDescriptorHeap(), dxCommon->GetSRVDescriptorSize(), 2);
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = dxCommon->GetGPUDescriptorHandle(dxCommon->GetSRVDescriptorHeap(), dxCommon->GetSRVDescriptorSize(), 2);

    // SRVの生成03_00
    dxCommon->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);
    // 05_01
    dxCommon->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);
    // InputLayout
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    // 05_03
    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc {};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    // BlendStateの設定
    D3D12_BLEND_DESC blendDesc {};
    // 全ての色要素を書き込む
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // RasiterzerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc {};
    // 裏面(時計回り)を表示しない
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    // 三角形の中を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    // Shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon->CompileShader(L"resources/shaders/Object3d.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);

    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon->CompileShader(L"resources/shaders/Object3d.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);

    // PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc {};
    graphicsPipelineStateDesc.pRootSignature = rootSignature.Get(); // RootSignatrue
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() }; // VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() }; // PixelShader
    graphicsPipelineStateDesc.BlendState = blendDesc; // BlensState
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState

    // DepthStencillStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc {};
    // depthの機能を有効かする
    depthStencilDesc.DepthEnable = true;
    // 書き込みします
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    // 比較関数はLessEqual。つまり、近ければ描画される
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    // depthStenncillの設定
    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // 書き込むRTVの情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    // 利用するトポロジ(形状)のタイプ。三角形
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    // どのように画面に色を打ち込むかの設定(気にしなくて良い)
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    ////////////
    // 実際に生成//
    ////////////
    //--------------------------
    // 通常モデル用リソース
    //--------------------------

    //--------------------------------------------------
    // modelDataを使う
    //-------------------------------------------------

    // 頂点リソースを作る
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
    // 頂点バッファービューを作成する
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView {};
    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress(); // リソース先頭のアドレスを使う
    vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size()); // 使用するリソースの頂点のサイズ
    vertexBufferView.StrideInBytes = sizeof(VertexData); // 1頂点あたりのサイズ
    // 頂点リソースにデータを書き込む
    VertexData* vertexData = nullptr;
    vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size()); // 頂点データをリソースにコピー

    //--------------------------
    //  マテリアル
    //--------------------------
    //   マテリアル用のリソースを作る今回はcolor一つ分のサイズを用意する05_03
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = dxCommon->CreateBufferResource(sizeof(Material));
    // マテリアルにデータを書き込む
    Material* materialData = nullptr;
    // 書き込むためのアドレスを取得
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    // 今回は赤を書き込んでみる
    materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData->uvTransform = MatrixMath::MakeIdentity4x4(); // 06_01_UuvTransform行列を単位行列で初期化
    materialData->enableLighting = true;
    ////--------------------------
    //// WVP行列
    ////--------------------------
    // WVPリソースを作る02_02
    Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
    // データを書き込む02_02
    TransformationMatrix* wvpData = nullptr;
    // 書き込むためのアドレスを取得02_02
    wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
    // 単位行列を書き込んでおく02_02
    Matrix4x4 identity = MatrixMath::MakeIdentity4x4();
    // 05_03
    memcpy(&wvpData->WVP, &identity, sizeof(Matrix4x4));
    memcpy(&wvpData->World, &identity, sizeof(Matrix4x4));
    //--------------------------
    //  Sprite用リソース
    //--------------------------
    // sprite用の頂点リソースを作る04_00
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = dxCommon->CreateBufferResource(sizeof(VertexData) * 4);
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
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);
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
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = dxCommon->CreateBufferResource(sizeof(Material));
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
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
    // sprite用のデータを書き込む04_00
    TransformationMatrix* transformationMatrixDataSprite = nullptr;
    // sprite用の書き込むためのアドレスを取得04_00
    transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
    // 単位行列を書き込んでおく04_00//これいったん消しました05_03
    // *transformationMatrixDataSprite = MakeIdentity4x4();

    ////--------------------------
    //// 共通リソース
    ////--------------------------

    // 平行光源用の定数バッファ（CBV）を作成（バッファサイズは構造体に合わせる）05_03
    Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
    // 平行光源用のデータを書き込み
    DirectionalLight* directionalLightData = nullptr;
    directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
    directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白色光
    directionalLightData->direction = MatrixMath::Normalize({ 0.0f, -1.0f, 0.0f }); // 真上から下方向
    directionalLightData->intensity = 1.0f; // 標準の明るさ

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

    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPinelineState = nullptr;
    hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPinelineState));
    assert(SUCCEEDED(hr));

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
    // デバックカメラインスタンス作成
    //=================================
    DebugCamera debugCamera;
    // debugcamera初期化一回だけ
    debugCamera.Initialize(winApp);
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

            // 開発用UIの処理。実際に開発用のUIを出す場合はここをげ０無固有の処理を置き換える02_03
            ImGui::ShowDemoWindow(); // ImGuiの始まりの場所-----------------------------

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
            ImGui::SliderFloat("x", &directionalLightData->direction.x, -10.0f, 10.0f);
            ImGui::SliderFloat("y", &directionalLightData->direction.y, -10.0f, 10.0f);
            ImGui::SliderFloat("z", &directionalLightData->direction.z, -10.0f, 10.0f);
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

            // 数字の０キーが押されていたら
             if (input->IsKeyPressed(DIK_0)) {
                 OutputDebugStringA("Hit 0");
                 soundmanager.SoundPlayWave(bgm);
             }

            //  メイクアフィンマトリックス02_02
             Matrix4x4 worldMatrix = MatrixMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
            // カメラのメイクアフィンマトリックス02_02
             Matrix4x4 cameraMatrix = MatrixMath::MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
            // 逆行列カメラ02_02
             Matrix4x4 viewMatrix = debugCamera.GetViewMatrix();
            // 透視投影行列02_02
             Matrix4x4 projectionMatrix = MatrixMath::MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
            // ワールドビュープロジェクション行列02_02
             Matrix4x4 worldViewProjectionMatrix = MatrixMath::Multiply(worldMatrix, MatrixMath::Multiply(viewMatrix, projectionMatrix));
            // CBVのバッファに書き込む02_02
            // CBVに正しい行列を書き込む
             memcpy(&wvpData->WVP, &worldViewProjectionMatrix, sizeof(Matrix4x4));

            // Sprite用のworldviewProjectionMatrixを作る04_00
             Matrix4x4 worldMatrixSprite = MatrixMath::MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
             Matrix4x4 viewMatrixSprite = MatrixMath::MakeIdentity4x4();
             Matrix4x4 projectionMatrixSprite = MatrixMath::MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
             Matrix4x4 worldViewProjectionMatrixSprite = MatrixMath::Multiply(worldMatrixSprite,
                 MatrixMath::Multiply(viewMatrixSprite, projectionMatrixSprite));
            // 単位行列を書き込んでおく04_00
             transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
             transformationMatrixDataSprite->World = worldMatrixSprite;

            //-------------------------
            // UVTransform用の行列生成
            //-------------------------
             Matrix4x4 uvTransformMatrix = MatrixMath::Matrix4x4MakeScaleMatrix(uvTransformSprite.scale);
             uvTransformMatrix = MatrixMath::Multiply(uvTransformMatrix, MatrixMath::MakeRotateZMatrix(uvTransformSprite.rotate.z));
             uvTransformMatrix = MatrixMath::Multiply(uvTransformMatrix, MatrixMath::MakeTranslateMatrix(uvTransformSprite.translate));
             materialDataSprite->uvTransform = uvTransformMatrix;

            // 描画前の処理//----------------------------------------------------
            dxCommon->PreDraw();
            // 画面のクリア処理

            // RootSignatureを設定。PS0に設定しているけど別途設定が必要
            dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
            dxCommon->GetCommandList()->SetPipelineState(graphicsPinelineState.Get()); // PS0を設定
            dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
            /// 形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えていけばよい
            dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonstarBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);

            //// マテリアルCbufferの場所を設定05_03変更
            dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress()); // ここでmaterialResource使え

            // wvp用のCBufferの場所を設定02_02
            dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
            // 平行光源用のCbufferの場所を設定05_03
            dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

            // 描画！(DRAWCALL/ドローコール)。３頂点で１つのインスタンス。インスタンスについては今後_05_00_OHTER
            dxCommon->GetCommandList()->DrawInstanced(kNumVertices, 1, 0, 0);
            // obj
            dxCommon->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0); // オブジェクトのやつ
            // マテリアルCbufferの場所を設定05_03変更これ書くとUvChackerがちゃんとする
            dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress()); // ここでmaterialResource使え

            // 描画
            dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
            // spriteの描画04_00
            dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
            // IBVを設定
            dxCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite);

            dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
            // UvChecker
            dxCommon->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0); // 左上のやつ

            // 描画の最後です//----------------------------------------------------
            //  実際のcommandListのImGuiの描画コマンドを積む
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

            dxCommon->PostDraw();
        }
    }

    // ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
    // こういうもんである。初期化と逆順に行う/
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // 解放処理CG2_01_03
   // CloseHandle();

    // リリースする場所
    // XAudio解放
    soundmanager.Finalize(&bgm);

    // WindowsAPIの終了処理
    winApp->Finalize();

    // デリート
    /*  delete input;*/
    delete winApp;
    delete dxCommon;
    return 0;

} // 最後のカギかっこ
