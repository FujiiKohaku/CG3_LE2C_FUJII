#pragma once
#include <Windows.h>
#include <cassert>
#include <dxcapi.h>
#include <wrl.h>
#pragma comment(lib, "dxcompiler.lib")
class Dxc {
public:
    // DXC各種初期化
    void Initialize();

    // DXC関連インスタンス取得
    IDxcUtils* GetUtils() const { return dxcUtils_.Get(); }
    IDxcCompiler3* GetCompiler() const { return dxcCompiler_.Get(); }
    IDxcIncludeHandler* GetIncludeHandler() const { return includeHandler_.Get(); }

private:
    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
};
