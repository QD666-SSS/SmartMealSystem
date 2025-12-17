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
     std::cout << u8"\n按回车键继续...";
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
            std::cout << u8"无效输入，请输入一个整数。" << std::endl;
        } else if (value < -1000000 || value > 1000000) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << u8"输入值超出合理范围，请重新输入。" << std::endl;
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
            std::cout << u8"无效输入，请输入一个数字。" << std::endl;
        } else if (value < 0 || value > 1000000) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << u8"输入值超出合理范围，请重新输入。" << std::endl;
        } else {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
    }
}

std::string Utils::getStringInput(const std::string& prompt) {
    std::string value;
    std::cout << prompt;
    std::getline(std::cin, value);
    
    while (value.empty()) {
        std::cout << u8"输入不能为空，请重试。" << std::endl;
        std::cout << prompt;
        std::getline(std::cin, value);
    }
    
    return value;
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
        return date.substr(0, 4) + u8"年" +
               date.substr(5, 2) + u8"月" +
               date.substr(8, 2) + u8"日";
    }
    return date;
    }

std::string Utils::toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}