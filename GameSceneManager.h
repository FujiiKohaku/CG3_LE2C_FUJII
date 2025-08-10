#pragma once
#include "CommonStructs.h"
#include "DebugCamera.h"
#include "DescriptorHeapWrapper.h"
#include "DeviceManager.h"
#include "DirectionalLightBuffer.h"
#include "Dxc.h"
#include "Input.h"
#include "Logger.h"
#include "ModelLoder.h"
#include "RenderHelper.h"
#include "SoundManager.h"
#include "Texture.h"
#include "WinApp.h"
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>

class GameSceneManager {
public:
    void Initialize(HINSTANCE hInst, int nCmdShow,
        const wchar_t* title, uint32_t width, uint32_t height);

    void LoadTextureAndMakeSRV(const char* texturePath, uint32_t srvIndex, Texture& outTex);
    void LoadModelAndMaterialSRV(const char* dir, const char* filename, uint32_t srvIndex, ModelData& outModel, Texture& outTex);

    WinApp& GetWinApp() { return win_; }
    DeviceManager& GetDeviceManager() { return deviceManager_; }
    Logger& GetLogger() { return log_; }
    // 第2陣
    DescriptorHeapWrapper& GetDSVHeap() { return dsvHeap_; }
    DescriptorHeapWrapper& GetSRVHeap() { return srvHeap_; }
    ID3D12Fence* GetFence() { return fence_.Get(); }
    HANDLE GetFenceEvent() { return fenceEvent_; }
    uint64_t& GetFenceValue() { return fenceValue_; }

    // 追加：描画で使うやつ
    ID3D12RootSignature* GetRootSignature() { return rootSignature_.Get(); }
    ID3D12PipelineState* GetPipelineState() { return pipelineState_.Get(); }
    // 追加第3陣
    uint32_t GetDescriptorSizeSRV() const { return descriptorSizeSRV_; }
    uint32_t GetDescriptorSizeRTV() const { return descriptorSizeRTV_; }
    uint32_t GetDescriptorSizeDSV() const { return descriptorSizeDSV_; }

    // SRV/DSV のハンドルを index 指定で取得（便利版）
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUHandle(uint32_t index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUHandle(uint32_t index) const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUHandle(uint32_t index) const;

    // ルートシグネチャ/PSO を外から使えるように
    ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
    ID3D12PipelineState* GetPipelineState() const { return pipelineState_.Get(); }

    Input& GetInput() { return input_; }
    DebugCamera& GetDebugCamera() { return debugCamera_; }
    SoundManager& GetSoundManager() { return soundmanager_; }
    SoundData& GetBGM() { return bgm_; }
    // 描画関連
    //   フレームの頭と尻（ImGui含む）
    void BeginFrame();
    void EndFrame();

    // 描画セットアップと終了（RenderHelper 呼び出しを内包）
    void PreDraw(const float clearColor[4], const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor);
    void PostDraw();
    // RenderHelper へのアクセス
    RenderHelper* GetRender() const { return render_.get(); }

private:
    Logger log_;
    WinApp win_;
    DeviceManager deviceManager_;
    // 第2陣
    DescriptorHeapWrapper dsvHeap_;
    DescriptorHeapWrapper srvHeap_;
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencillResource_;
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    uint64_t fenceValue_ = 0;
    HANDLE fenceEvent_ = nullptr;

    // 追加：DXC / ルートシグネチャ / PSO
    Dxc dxc_;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

    // 追加第3陣

    uint32_t descriptorSizeSRV_ = 0;
    uint32_t descriptorSizeRTV_ = 0;
    uint32_t descriptorSizeDSV_ = 0;

    std::unique_ptr<RenderHelper> render_;

    // 追加
    Input input_;
    DebugCamera debugCamera_;
    SoundManager soundmanager_;
    SoundData bgm_ {};
};
