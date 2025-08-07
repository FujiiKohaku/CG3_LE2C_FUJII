#pragma once
#include <fstream>
#include <string>

class Logger {
public:
    static void Initialize(); // ログファイル作成
    static void Log(const std::string& message);
    static void Log(const std::wstring& message);
    void Log(std::ostream& os, const std::string& message);
    static std::ofstream& GetStream(); // 明示的に出力先にアクセスしたい場合

private:
    static std::ofstream logStream_;
};
