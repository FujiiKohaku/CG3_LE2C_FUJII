#include "Logger.h"
#include <Windows.h> // OutputDebugStringA

// static メンバ関数の実装
void Logger::Log(const std::string& message)
{
    // コンソールに出力
    std::cout << message << std::endl;

    // Visual Studio の出力ウィンドウに出力
    OutputDebugStringA((message + "\n").c_str());
}
