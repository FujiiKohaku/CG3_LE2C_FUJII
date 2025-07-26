#include "Logger.h"
// グローバルインスタンスの実体
Logger logger;

Logger::Logger()
{
    // ログのディレクトリを用意
    std::filesystem::create_directory("logs");

    // 現在時刻を取得(UTC)
    auto now = std::chrono::system_clock::now();
    auto nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);

    // 日本時間に変換
    std::chrono::zoned_time localTime { std::chrono::current_zone(), nowSeconds };

    // 時刻を文字列化
    std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);

    // ファイル名決定
    logFilePath_ = "logs/" + dateString + ".log";

    // ファイルを開く
    logStream_.open(logFilePath_, std::ios::out);
}

Logger::~Logger()
{
    if (logStream_.is_open()) {
        logStream_.close();
    }
}

void Logger::Log(std::ostream& os, const std::string& message)
{
    os << message << '\n'; // フラッシュは呼び出し側で必要なら
}

void Logger::Log(const std::string& message)
{
    if (logStream_.is_open()) {
        logStream_ << message << '\n';
    }
    OutputDebugStringA((message + "\n").c_str());
}