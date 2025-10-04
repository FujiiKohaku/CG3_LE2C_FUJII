#include "DirectXCommon.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <format>
#include <wrl.h>
void DirectXCommon::Initialize()
{
    // �����ɏ����������������Ă���
}

#pragma region �f�o�C�X������

// �f�o�C�X�������֐�
void DirectXCommon::InitializeDevice()
{
    HRESULT hr;
    // �f�o�b�O���C���[��ON�ɂ���
#ifdef _DEBUG

    Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr; // COM
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        // �f�o�b�N���C���[��L��������
        debugController->EnableDebugLayer();
        // �����6PU���ł��`�F�b�N���X�g���s���悤�ɂ���
        debugController->SetEnableGPUBasedValidation(TRUE);
    }
#endif // _DEBUG
    // DXGI�t�@�N�g���[�̐���
    hr = CreateDXGIFactory(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
    // �������̍��{�I�ȕ����ŃG���[���o���ꍇ�̓v���O�������Ԉ���Ă��邩�A�ǂ��ɂ��ł��Ȃ��ꍇ�������̂�assert�ɂ��Ă���
    assert(SUCCEEDED(hr));
    IDXGIAdapter4* useAdapter = nullptr; // com
    // �悢���ɃA�_�v�^�𗊂�
    for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {

        // �A�_�v�^�[�̏����擾����
        DXGI_ADAPTER_DESC3 adapterDesc {}; // com
        hr = useAdapter->GetDesc3(&adapterDesc); // comGet
        assert(SUCCEEDED(hr)); // �擾�ł��Ȃ��͈̂�厖
        // �\�t�g�E�F�A�A�_�v�^�łȂ���΍̗p!
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) { // get
            // �̗p�����A�_�v�^�̏������O�ɏo��wstring�̕��Ȃ̂Œ���
            Logger::Log(StringUtility::ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description))); // get
            break;
        }
        useAdapter = nullptr; // �\�t�g�E�F�A�A�_�v�^�̏ꍇ�͌��Ȃ��������Ƃɂ���
    }

    // �K�؂ȃA�_�v�^��������Ȃ������̂ŋN���ł��Ȃ�
    assert(useAdapter != nullptr);

    // ������x���ƃ��O�o�͗p�̕�����
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
    };

    const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };
    // �������ɐ����ł��邩�����Ă���
    for (size_t i = 0; i < _countof(featureLevels); ++i) {
        // �̗p�����A�_�v�^�[�Ńf�o�C�X�𐶐�
        hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
        // �w�肵��������x���Ńf�o�C�X�͐����ł������m�F
        if (SUCCEEDED(hr)) {
            // �����ł����̂Ń��O�o�͂��s���ă��[�v�𔲂���

            Logger::Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
            break;
        }
    }
    // �f�o�C�X�̐�������肭�����Ȃ������̂ŋN���ł��Ȃ�
    assert(device != nullptr);
    Logger::Log("Complete create D3D12Device!!!\n"); // �����������̃��O���o��
}
#pragma endregion

#pragma region �R�}���h������
void DirectXCommon::InitializeCommand()
{
    HRESULT hr;

    // �R�}���h�L���[�𐶐�����
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc {};
    hr = device->CreateCommandQueue(&commandQueueDesc,
        IID_PPV_ARGS(&commandQueue));
    // �R�}���h�L���[�̐�������肭�����Ȃ������̂ŋN���ł��Ȃ�
    assert(SUCCEEDED(hr));

    // �R�}���h�A���P�[�^�[�𐶐�����
    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    // �R�}���h�L���[�A���P�[�^�[�̐���������肭�����Ȃ������̂ŋN���ł��Ȃ�
    assert(SUCCEEDED(hr));

    // �R�}���h���X�g�𐶐�����
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
    // �R�}���h���X�g�̐�������肭�����Ȃ������̂ŋN���ł��Ȃ�
    assert(SUCCEEDED(hr));
}
#pragma endregion

void DirectXCommon::InitializeSwapChain()
{
    HRESULT hr;

    // �X���b�v�`�F�[���𐶐�����
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc {};
    swapChainDesc.Width = WinApp::kClientWidth; // ��ʂ̕��B�E�B���h�E�̃N���C�A���g�̈�𓯂����̂�ɂ��Ă���
    swapChainDesc.Height = WinApp::kClientHeight; // ��ʂ̍����B�E�B���h�E�̃N���C�A���g�̈�𓯂����̂ɂ��Ă���
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // �F�̌`��
    swapChainDesc.SampleDesc.Count = 1; // �}���`�T���v�����Ȃ�
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // �`��̃^�[�Q�b�g�Ƃ��Ă�悤����
    swapChainDesc.BufferCount = 2; // �_�u���o�b�t�@
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // ���j�^�[�Ɉڂ�����,���g��f��
    // �R�}���h�L���[,�E�B���h�E�o���h���A�ݒ��n���Đ�������
    hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf())); // com.Get,OF
    assert(SUCCEEDED(hr));
}
