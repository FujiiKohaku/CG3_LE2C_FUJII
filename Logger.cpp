#include "Logger.h"
#include <Windows.h>
#include <chrono>
#include <filesystem>
#include <format>

std::ofstream Logger::logStream_;

void Logger::Init()
{
    // ディレクトリ作成
    std::filesystem::create_directory("logs");

    // 現在時刻（秒単位）
    auto now = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
    std::chrono::zoned_time localTime { std::chrono::current_zone(), now };
    std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);

    // ファイルオープン
    std::string logFilePath = "logs/" + dateString + ".log";
    logStream_.open(logFilePath);
}

void Logger::Log(const std::string& message)
{
    if (logStream_.is_open()) {
        logStream_ << message << std::endl;
    }
    OutputDebugStringA((message + "\n").c_str());
}
