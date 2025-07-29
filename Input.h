#pragma once
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定

#include "MatrixMath.h"
#include <dinput.h>
#include <wrl.h> // ComPtr 用
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
class Input {
public:
    bool Initialize(HINSTANCE hInstance, HWND hwnd);
    void Update();
    bool IsKeyPressed(BYTE keyCode) const;
    bool IsGamepadButtonPressed(int buttonIndex) const;
    Vector2 GetLeftStick() const;
    bool IsGamepadConnected() const;


private:
    static BOOL CALLBACK EnumGamepadCallback(const DIDEVICEINSTANCE* instance, VOID* context);

    Microsoft::WRL::ComPtr<IDirectInput8> directInput_;
    Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_;
    Microsoft::WRL::ComPtr<IDirectInputDevice8> gamepad_;
    BYTE keys_[256] {};
    DIJOYSTATE gamepadState_ {};
};
