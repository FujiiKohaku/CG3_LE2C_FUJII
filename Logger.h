#pragma once
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <Windows.h>
class Logger {
public:
    // コンストラクタでログファイルを自動生成
    Logger();
    ~Logger();

    // 任意の ostream にログを出力
    void Log(std::ostream& os, const std::string& message);

    // ログファイルとOutputDebugStringにも同時にログを出す
    void Log(const std::string& message);

    // ログファイルのパス取得
    const std::string& GetFilePath() const { return logFilePath_; }

    // 必要ならログファイルストリームを外部に返すgetter
    std::ofstream& GetLogStream() { return logStream_; }

private:
    std::ofstream logStream_;
    std::string logFilePath_; // ログファイルのパス
};
// ここでグローバルインスタンスを宣言
extern Logger logger;
