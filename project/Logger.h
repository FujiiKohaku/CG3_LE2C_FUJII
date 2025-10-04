#pragma once
#include <iostream>
#include <string>

class Logger {
public:
    // static メンバ関数として宣言
    static void Log(const std::string& message);
};
