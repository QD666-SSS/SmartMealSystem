#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

class Utils {
public:
    static void clearScreen();
    static void pause();
    static std::string trim(const std::string& str);
    static void displayProgressBar(double percentage, int width = 30);
    static std::string getStringInput(const std::string& prompt);
    static int getIntInput(const std::string& prompt);
    static double getDoubleInput(const std::string& prompt);
    static bool isValidEmail(const std::string& email);
    static std::string formatDate(const std::string& date);
    static std::string toLowerCase(const std::string& str);
};

#endif