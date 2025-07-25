#pragma once
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <format>
#include <wrl.h>
#include "Utility.h"
class GraphicsDevice {
public:
    void Initialize(HWND hwnd, uint32_t width, uint32_t height);

    // �Q�b�^�[��
    ID3D12Device* GetDevice() const { return device_.Get(); }
    ID3D12CommandQueue* GetCommandQueue() const { return commandQueue_.Get(); }
    ID3D12CommandAllocator* GetCommandAllocator() const { return commandAllocator_.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }

private:
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_; // DXGI�t�@�N�g���[
    Microsoft::WRL::ComPtr<ID3D12Device> device_; // D3D12�f�o�C�X
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_; // �R�}���h�L���[
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_; // �R�}���h�A���P�[�^�[
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_; // �R�}���h���X�g
};
