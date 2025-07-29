#pragma once
#include <Windows.h>
#include <dxcapi.h>
#include <wrl.h>
#pragma comment(lib, "dxcompiler.lib")

class ShaderCompilerDXC {
public:
    ShaderCompilerDXC();
    ~ShaderCompilerDXC();

    void Initialize(); // 初期化

    // アクセサ
    Microsoft::WRL::ComPtr<IDxcUtils> GetUtils() const { return dxcUtils_.Get(); }
    Microsoft::WRL::ComPtr<IDxcCompiler3> GetCompiler() const { return dxcCompiler_.Get(); }
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> GetIncludeHandler() const { return includeHandler_.Get(); }

private:
    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
};
