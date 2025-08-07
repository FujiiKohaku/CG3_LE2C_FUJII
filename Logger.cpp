#include "Logger.h"
#include <Windows.h>
#include <chrono>
#include <filesystem>
#include <format>
#include <iostream>

std::ofstream Logger::logStream_;

void Logger::Initialize()
{
    std::filesystem::create_directory("logs");

    auto now = std::chrono::system_clock::now();
    auto nowSec = std::chrono::time_point_cast<std::chrono::seconds>(now);
    std::chrono::zoned_time localTime { std::chrono::current_zone(), nowSec };
    std::string filename = std::format("{:%Y%m%d_%H%M%S}.log", localTime);

    logStream_.open("logs/" + filename);
}

void Logger::Log(const std::string& message)
{
    if (logStream_.is_open()) {
        logStream_ << message << std::endl;
    }
    std::cout << message << std::endl;
    OutputDebugStringA(message.c_str());
}

void Logger::Log(const std::wstring& message)
{
    OutputDebugStringW(message.c_str());
}

void Logger::Log(std::ostream& os, const std::string& message)
{
    os << message << std::endl;
    OutputDebugStringA(message.c_str());
}

std::ofstream& Logger::GetStream()
{
    return logStream_;
}
