#include "GraphicsDevice.h"
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

void GraphicsDevice::Initialize(HWND hwnd, uint32_t width, uint32_t height)
{

    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
    assert(SUCCEEDED(hr));

    Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter; // �A�_�v�^�p�̕ϐ�
    // �悢���ɃA�_�v�^�𗊂�
    for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; ++i) {

        // �A�_�v�^�[�̏����擾����
        DXGI_ADAPTER_DESC3 adapterDesc {}; // com
        hr = adapter->GetDesc3(&adapterDesc); // comGet
        assert(SUCCEEDED(hr)); // �擾�ł��Ȃ��͈̂�厖
        // �\�t�g�E�F�A�A�_�v�^�łȂ���΍̗p!
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) { // get
            // �̗p�����A�_�v�^�̏������O�ɏo��wstring�̕��Ȃ̂Œ���
            // Utility::Log(logStream, Utility::ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description))); // ��U�R�����g�A�E�g��Ŗ߂��Ă����-

            break;
        }
        adapter.Reset();
    }
    assert(adapter);

    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_12_2,
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0
    };
    const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };

    // �������ɐ����ł��邩�����Ă���
    for (size_t i = 0; i < _countof(levels); ++i) {
        hr = D3D12CreateDevice(adapter.Get(), levels[i], IID_PPV_ARGS(&device_));
        if (SUCCEEDED(hr)) {
            /*Utility::Log(logStream, std::format("FeatureLevel : {}\n", featureLevelStrings[i]));*/ // ��Ŗ߂��Ă�����
            break;
        }
    }
    // �ŏI�I�� device_ �������ł��Ă��Ȃ��ꍇ�� assert
    assert(device_ != nullptr);
    /* Utility::Log(logStream, "Complete create D3D12Device!!!\n");*/ // ��Ŗ߂��Ă�����

    // �R�}���h�L���[�쐬
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc {};
    hr = device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
    assert(SUCCEEDED(hr));

    hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
    assert(SUCCEEDED(hr));

    // �R�}���h���X�g�𐶐�����
    hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
    // �R�}���h���X�g�̐�������肭�����Ȃ������̂ŋN���ł��Ȃ�
    assert(SUCCEEDED(hr));
}
