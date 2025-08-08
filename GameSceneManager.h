#pragma once
#include "DescriptorHeapWrapper.h"
#include "DeviceManager.h"
#include "DirectionalLightBuffer.h"
#include "Dxc.h"
#include "Logger.h"
#include "WinApp.h"
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>

class GameSceneManager {
public:
    void Initialize(HINSTANCE hInst, int nCmdShow,
        const wchar_t* title, uint32_t width, uint32_t height);

private:
};
