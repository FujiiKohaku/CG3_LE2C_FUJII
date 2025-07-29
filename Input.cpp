#include "Input.h"
#include <cassert>

//=============================================================================
// 初期化
//=============================================================================
bool Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
    HRESULT result;

    // DirectInput全体の初期化（1つだけでよい）
    result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
        reinterpret_cast<void**>(directInput_.ReleaseAndGetAddressOf()), nullptr);
    assert(SUCCEEDED(result));

    //-------------------------
    // キーボードの初期化
    //-------------------------
    result = directInput_->CreateDevice(GUID_SysKeyboard, keyboard_.ReleaseAndGetAddressOf(), nullptr);
    assert(SUCCEEDED(result));

    result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
    assert(SUCCEEDED(result));

    result = keyboard_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
    assert(SUCCEEDED(result));

    //-------------------------
    // ゲームパッドの初期化（1台）
    //-------------------------
    result = directInput_->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumGamepadCallback, this, DIEDFL_ATTACHEDONLY);
    assert(SUCCEEDED(result));

    if (gamepad_) {
        result = gamepad_->SetDataFormat(&c_dfDIJoystick);
        assert(SUCCEEDED(result));

        result = gamepad_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
        assert(SUCCEEDED(result));
    }

    return true;
}

//=============================================================================
// 更新（キーボード／ゲームパッド）
//=============================================================================
void Input::Update()
{
    //-------------------------
    // キーボードの状態取得
    //-------------------------
    HRESULT result = keyboard_->Acquire();
    if (FAILED(result))
        return;

    result = keyboard_->GetDeviceState(sizeof(keys_), keys_);
    if (FAILED(result))
        keyboard_->Acquire();

    //-------------------------
    // ゲームパッドの状態取得
    //-------------------------
    if (gamepad_) {
        result = gamepad_->Acquire();
        if (SUCCEEDED(result)) {
            result = gamepad_->GetDeviceState(sizeof(DIJOYSTATE), &gamepadState_);
            if (FAILED(result))
                gamepad_->Acquire();
        }
    }
}

//=============================================================================
// キー入力判定
//=============================================================================
bool Input::IsKeyPressed(BYTE keyCode) const
{
    return keys_[keyCode] & 0x80;
}

//=============================================================================
// ゲームパッドボタン入力判定
//=============================================================================
bool Input::IsGamepadButtonPressed(int buttonIndex) const
{
    if (!gamepad_)
        return false;
    if (buttonIndex < 0 || buttonIndex >= 128)
        return false;

    return (gamepadState_.rgbButtons[buttonIndex] & 0x80) != 0;
}
Vector2 Input::GetLeftStick() const
{
    // DirectInput の値は 0〜65535 など環境依存。：
    const float center = 32767.5f;

    float x = (float)(gamepadState_.lX - center) / center; // -1 ～ +1
    float y = (float)(gamepadState_.lY - center) / center; // -1 ～ +1

    return { x, y };
}

bool Input::IsGamepadConnected() const
{
    return gamepad_ != nullptr;
}

//=============================================================================
// ゲームパッド列挙コールバック（最初の1台だけ取得）
//=============================================================================
BOOL CALLBACK Input::EnumGamepadCallback(const DIDEVICEINSTANCE* instance, VOID* context)
{
    Input* input = reinterpret_cast<Input*>(context);
    if (SUCCEEDED(input->directInput_->CreateDevice(instance->guidInstance,
            input->gamepad_.ReleaseAndGetAddressOf(), nullptr))) {
        return DIENUM_STOP;
    }
    return DIENUM_CONTINUE;
}
