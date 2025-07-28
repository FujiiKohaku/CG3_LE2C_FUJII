#include "ShaderCompiler.h"
#include "Utility.h" // Utility::Log用
#include <cassert>
#include <format> // C++20以降、std::formatを使う場合
#pragma comment(lib, "dxcompiler.lib")

Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
    const std::wstring& filepath,
    const wchar_t* profile,
    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils,
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler,
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler,
    std::ostream& os)
{
    // 元の関数内容まんま
    Utility::Log(os, Utility::ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filepath, profile)));
    Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
    HRESULT hr = dxcUtils->LoadFile(filepath.c_str(), nullptr, &shaderSource);
    assert(SUCCEEDED(hr));

    DxcBuffer shaderSourceBuffer;
    shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
    shaderSourceBuffer.Size = shaderSource->GetBufferSize();
    shaderSourceBuffer.Encoding = DXC_CP_UTF8;

    LPCWSTR arguments[] = {
        filepath.c_str(),
        L"-E",
        L"main",
        L"-T",
        profile,
        L"-Zi",
        L"-Qembed_debug",
        L"-Od",
        L"-Zpr"
    };

    Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
    hr = dxcCompiler->Compile(
        &shaderSourceBuffer,
        arguments,
        _countof(arguments),
        includeHandler.Get(),
        IID_PPV_ARGS(&shaderResult));
    assert(SUCCEEDED(hr));

    IDxcBlobUtf8* shaderError = nullptr;
    shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
    if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
        Utility::Log(os, shaderError->GetStringPointer());
        assert(false);
    }

    Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
    hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
    assert(SUCCEEDED(hr));

    Utility::Log(os, Utility::ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filepath, profile)));

    return shaderBlob;
}
