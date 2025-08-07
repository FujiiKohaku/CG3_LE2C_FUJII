#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include <string>
#include <wrl.h>

class Logger;

Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
    const std::wstring& filepath,
    const wchar_t* profile,
    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils,
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler,
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler,
    Logger& logger); // ← ostream → Logger に変更
