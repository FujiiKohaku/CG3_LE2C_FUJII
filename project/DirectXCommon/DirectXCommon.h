#pragma once
#include "Logger.h"
#include "StringUtility.h"
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <wrl.h> // ComPtr �p
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
class DirectXCommon {
public:
    // ������
    void Initialize();

private:
    // DXGI�t�@�N�g���[�̐���
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
    // �f�o�C�X
    Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;

    // �R�}���h�L���[
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
    // �R�}���h�A���P�[�^
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
    // �R�}���h���X�g
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;

    // �X���b�v�`�F�C��
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
    // �f�o�C�X�������֐�
    void InitializeDevice();
    // �R�}���h������
    void InitializeCommand();

    void InitializeSwapChain();
};
