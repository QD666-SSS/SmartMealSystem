#include "../include/Utils.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <cctype>
#include <iomanip>

void Utils::clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

void Utils::pause() {
    std::cout << "\n按回车键继续...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

int Utils::getIntInput(const std::string& prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "无效输入，请输入一个整数。" << std::endl;
        } else {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
    }
}

double Utils::getDoubleInput(const std::string& prompt) {
    double value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "无效输入，请输入一个数字。" << std::endl;
        } else {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
    }
}

std::string Utils::getStringInput(const std::string& prompt) {
    std::string value;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, value);
        
        if (value.empty()) {
            std::cout << "输入不能为空，请重试。" << std::endl;
        } else {
            return value;
        }
    }
}

std::string Utils::trim(const std::string& str) {
    std::string result = str;
    result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](int ch) {
        return !std::isspace(ch);
    }));
    result.erase(std::find_if(result.rbegin(), result.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), result.end());
    return result;
}

bool Utils::isValidEmail(const std::string& email) {
    if (email.empty()) return false;
    
    size_t atPos = email.find('@');
    size_t dotPos = email.rfind('.');
    
    return (atPos != std::string::npos && 
            dotPos != std::string::npos && 
            atPos < dotPos &&
            atPos > 0 && 
            dotPos < email.length() - 1);
}

void Utils::displayProgressBar(double percentage, int width) {
    int filled = static_cast<int>(percentage * width / 100);
    int empty = width - filled;
    
    std::cout << "[";
    for (int i = 0; i < filled; ++i) {
        std::cout << "=";
    }
    for (int i = 0; i < empty; ++i) {
        std::cout << " ";
    }
    std::cout << "] " << std::fixed << std::setprecision(1) << percentage << "%";
}

std::string Utils::formatDate(const std::string& date) {
    if (date.length() == 10 && date[4] == '-' && date[7] == '-') {
        return date.substr(0, 4) + "年" + 
               date.substr(5, 2) + "月" + 
               date.substr(8, 2) + "日";
    }
    return date;
}

std::string Utils::toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}