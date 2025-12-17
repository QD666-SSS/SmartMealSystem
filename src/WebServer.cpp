#include "WebServer.h"
#include "Database.h"
#include "RecommendationEngine.h"
#include "User.h"
#include "Food.h"
#include "Meal.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iomanip>

// 跨平台的socket处理
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <cstring>
    #include <arpa/inet.h>
    #define closesocket close
#endif

// Helper functions
static std::string getQueryParam(const std::string& fullPath, const std::string& key) {
    size_t queryPos = fullPath.find('?');
    if (queryPos == std::string::npos) return "";
    
    std::string query = fullPath.substr(queryPos + 1);
    std::stringstream ss(query);
    std::string segment;
    while (std::getline(ss, segment, '&')) {
        size_t eqPos = segment.find('=');
        if (eqPos != std::string::npos) {
            std::string k = segment.substr(0, eqPos);
            if (k == key) {
                return segment.substr(eqPos + 1);
            }
        }
    }
    return "";
}

static std::string escapeJSON(const std::string& s) {
    std::string res;
    for (char c : s) {
        if (c == '"') res += "\\\"";
        else if (c == '\\') res += "\\\\";
        else if (c == '\b') res += "\\b";
        else if (c == '\f') res += "\\f";
        else if (c == '\n') res += "\\n";
        else if (c == '\r') res += "\\r";
        else if (c == '\t') res += "\\t";
        else res += c;
    }
    return res;
}

static std::string getJsonValue(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    
    size_t colon = json.find(":", pos);
    if (colon == std::string::npos) return "";
    
    size_t startValue = json.find("\"", colon);
    if (startValue == std::string::npos) return "";
    
    size_t endValue = json.find("\"", startValue + 1);
    if (endValue == std::string::npos) return "";
    
    return json.substr(startValue + 1, endValue - startValue - 1);
}

SimpleWebServer::SimpleWebServer(Database* db, RecommendationEngine* engine, int port) 
    : db(db), engine(engine), port(port), server_socket(INVALID_SOCKET), running(false) {}

SimpleWebServer::~SimpleWebServer() {
    cleanup();
}

void SimpleWebServer::start() {
    if (initialize()) {
        running = true;
        std::cout << "Web服务器启动在端口 " << port << std::endl;
        run();
    }
}

void SimpleWebServer::stop() {
    running = false;
    closesocket(server_socket);
    cleanup();
}

bool SimpleWebServer::initialize() {
#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cout << "WSAStartup失败: " << result << std::endl;
        return false;
    }
#endif

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cout << "创建socket失败" << std::endl;
        cleanup();
        return false;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cout << "绑定失败" << std::endl;
        closesocket(server_socket);
        cleanup();
        return false;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "监听失败" << std::endl;
        closesocket(server_socket);
        cleanup();
        return false;
    }

    return true;
}

void SimpleWebServer::run() {
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        socket_t client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket != INVALID_SOCKET) {
            std::thread(&SimpleWebServer::handleClient, this, client_socket).detach();
        }
    }
}

void SimpleWebServer::handleClient(socket_t client_socket) {
    char buffer[4096] = {0};
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    
    if (bytes_received > 0) {
        std::string request(buffer, bytes_received);
        std::string response = handleHttpRequest(request);
        send(client_socket, response.c_str(), response.length(), 0);
    }
    
    closesocket(client_socket);
}

std::string SimpleWebServer::handleHttpRequest(const std::string& request) {
    std::istringstream stream(request);
    std::string method, fullPath, version;
    stream >> method >> fullPath >> version;
    
    std::string path = fullPath;
    size_t queryPos = fullPath.find('?');
    if (queryPos != std::string::npos) {
        path = fullPath.substr(0, queryPos);
    }
    
    std::string response;
    
    if (path == "/" || path == "/index.html") {
        response = serveIndexPage();
    } else if (path == "/api/login") {
        response = handleLogin(request);
    } else if (path == "/api/register") {
        response = handleRegister(request);
    } else if (path == "/api/profile") {
        response = handleGetProfile(fullPath);
    } else if (path == "/api/foods") {
        response = handleGetFoods(request);
    } else if (path == "/api/recommendations") {
        response = handleGetRecommendations(fullPath);
    } else if (path == "/api/meals") {
        response = handleGetMeals(fullPath);
    } else if (path == "/api/statistics") {
        response = handleGetStatistics(fullPath);
    } else {
        response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found";
    }
    
    return response;
}

std::string SimpleWebServer::serveIndexPage() {
    std::string html;
    std::vector<std::string> possiblePaths = {
        "index.html",
        "src/index.html",
        "./index.html",
        "../src/index.html",
        "../../src/index.html",
        "../../../src/index.html",
        "bin/Debug/index.html",
        "bin/Release/index.html",
        "../../../bin/Debug/index.html",
        "../../../bin/Release/index.html"
    };
    
    for (const auto& path : possiblePaths) {
        std::ifstream file(path);
        if (file.is_open()) {
            std::ostringstream ss;
            ss << file.rdbuf();
            html = ss.str();
            file.close();
            break;
        }
    }
    
    if (html.empty()) {
        html = "<html><body><h1>Error: Could not load index.html</h1></body></html>";
    }
    
    return "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: " + 
           std::to_string(html.length()) + "\r\n\r\n" + html;
}

std::string SimpleWebServer::handleLogin(const std::string& request) {
    size_t bodyPos = request.find("\r\n\r\n");
    if (bodyPos == std::string::npos) return "HTTP/1.1 400 Bad Request\r\n\r\n";
    std::string body = request.substr(bodyPos + 4);
    
    std::string username = getJsonValue(body, "username");
    std::string password = getJsonValue(body, "password");
    
    auto users = db->getAllUsers();
    User* foundUser = nullptr;
    for (auto& user : users) {
        if (user.getUsername() == username && user.getPassword() == password) {
            foundUser = &user;
            break;
        }
    }
    
    std::string response_body;
    if (foundUser) {
        response_body = "{\"success\":true,\"user\":{\"id\":" + std::to_string(foundUser->getId()) + 
                        ",\"username\":\"" + escapeJSON(foundUser->getUsername()) + "\"" +
                        ",\"age\":" + std::to_string(foundUser->getAge()) +
                        ",\"gender\":\"" + escapeJSON(foundUser->getGender()) + "\"" +
                        ",\"height\":" + std::to_string(foundUser->getHeight()) +
                        ",\"weight\":" + std::to_string(foundUser->getWeight()) +
                        ",\"dailyCalorieGoal\":" + std::to_string(foundUser->getDailyCalorieGoal()) +
                        ",\"dailyProteinGoal\":" + std::to_string(foundUser->getDailyProteinGoal()) +
                        ",\"dailyCarbGoal\":" + std::to_string(foundUser->getDailyCarbGoal()) +
                        ",\"dailyFatGoal\":" + std::to_string(foundUser->getDailyFatGoal()) +
                        "},\"message\":\"Login successful\"}";
    } else {
        response_body = "{\"success\":false,\"message\":\"Invalid username or password\"}";
    }
    
    return "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " + 
           std::to_string(response_body.length()) + "\r\n\r\n" + response_body;
}

std::string SimpleWebServer::handleRegister(const std::string& request) {
    size_t bodyPos = request.find("\r\n\r\n");
    if (bodyPos == std::string::npos) return "HTTP/1.1 400 Bad Request\r\n\r\n";
    std::string body = request.substr(bodyPos + 4);
    
    std::string username = getJsonValue(body, "username");
    std::string password = getJsonValue(body, "password");
    
    if (username.empty() || password.empty()) {
        std::string response_body = "{\"success\":false,\"message\":\"Username and password required\"}";
        return "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " + 
               std::to_string(response_body.length()) + "\r\n\r\n" + response_body;
    }
    
    auto users = db->getAllUsers();
    for (const auto& user : users) {
        if (user.getUsername() == username) {
            std::string response_body = "{\"success\":false,\"message\":\"Username already exists\"}";
            return "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " + 
                   std::to_string(response_body.length()) + "\r\n\r\n" + response_body;
        }
    }
    
    User newUser;
    newUser.setId(db->getNextUserId());
    newUser.setUsername(username);
    newUser.setPassword(password);
    // Default values
    newUser.setAge(25);
    newUser.setGender("Other");
    newUser.setHeight(170);
    newUser.setWeight(65);
    newUser.calculateNutritionGoals();
    
    db->saveUser(newUser);
    
    std::string response_body = "{\"success\":true,\"message\":\"Registration successful\"}";
    return "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " + 
           std::to_string(response_body.length()) + "\r\n\r\n" + response_body;
}

std::string SimpleWebServer::handleGetProfile(const std::string& request) {
    std::string userIdStr = getQueryParam(request, "userId");
    if (userIdStr.empty()) return "HTTP/1.1 400 Bad Request\r\n\r\n";
    
    int userId = std::stoi(userIdStr);
    auto userOpt = db->getUserById(userId);
    
    if (!userOpt) {
        return "HTTP/1.1 404 Not Found\r\n\r\n";
    }
    
    User user = *userOpt;
    std::string response_body = "{\"success\":true,\"user\":{\"id\":" + std::to_string(user.getId()) + 
                        ",\"username\":\"" + escapeJSON(user.getUsername()) + "\"" +
                        ",\"age\":" + std::to_string(user.getAge()) +
                        ",\"gender\":\"" + escapeJSON(user.getGender()) + "\"" +
                        ",\"height\":" + std::to_string(user.getHeight()) +
                        ",\"weight\":" + std::to_string(user.getWeight()) +
                        ",\"dailyCalorieGoal\":" + std::to_string(user.getDailyCalorieGoal()) +
                        ",\"dailyProteinGoal\":" + std::to_string(user.getDailyProteinGoal()) +
                        ",\"dailyCarbGoal\":" + std::to_string(user.getDailyCarbGoal()) +
                        ",\"dailyFatGoal\":" + std::to_string(user.getDailyFatGoal()) +
                        "}}";
                        
    return "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " + 
           std::to_string(response_body.length()) + "\r\n\r\n" + response_body;
}

std::string SimpleWebServer::handleGetFoods(const std::string& request) {
    auto foods = db->getAllFoods();
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < foods.size(); ++i) {
        const auto& food = foods[i];
        ss << "{\"id\":" << food.getId() 
           << ",\"name\":\"" << escapeJSON(food.getName()) << "\""
           << ",\"calories\":" << food.getCalories()
           << ",\"protein\":" << food.getProtein()
           << ",\"carbs\":" << food.getCarbohydrates()
           << ",\"fat\":" << food.getFat()
           << ",\"tags\":[";
           
        auto tags = food.getTags();
        int t = 0;
        for (const auto& tag : tags) {
            if (t > 0) ss << ",";
            ss << "\"" << escapeJSON(tag) << "\"";
            t++;
        }
        ss << "]}";
        
        if (i < foods.size() - 1) ss << ",";
    }
    ss << "]";
    
    std::string response_body = ss.str();
    return "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " + 
           std::to_string(response_body.length()) + "\r\n\r\n" + response_body;
}

std::string SimpleWebServer::handleGetRecommendations(const std::string& request) {
    std::string userIdStr = getQueryParam(request, "userId");
    if (userIdStr.empty()) return "HTTP/1.1 400 Bad Request\r\n\r\n";
    
    int userId = std::stoi(userIdStr);
    auto userOpt = db->getUserById(userId);
    
    if (!userOpt) return "HTTP/1.1 404 Not Found\r\n\r\n";
    
    std::time_t t = std::time(nullptr);
    char dateBuf[128];
    std::strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", std::localtime(&t));
    
    auto meals = engine->recommendDailyMeals(*userOpt, dateBuf);
    
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < meals.size(); ++i) {
        const auto& meal = meals[i];
        
        std::string mealName;
        auto foods = meal.getFoods();
        for(size_t j=0; j<foods.size(); ++j) {
            if(j>0) mealName += " + ";
            mealName += foods[j].getName();
        }

        ss << "{\"id\":" << meal.getId() 
           << ",\"name\":\"" << escapeJSON(mealName) << "\""
           << ",\"type\":\"" << escapeJSON(meal.getMealType()) << "\""
           << ",\"calories\":" << meal.getTotalCalories()
           << ",\"items\":[";
           
        for (size_t j = 0; j < foods.size(); ++j) {
            if (j > 0) ss << ",";
            ss << "\"" << escapeJSON(foods[j].getName()) << "\"";
        }
        ss << "]}";
        
        if (i < meals.size() - 1) ss << ",";
    }
    ss << "]";
    
    std::string response_body = ss.str();
    return "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " + 
           std::to_string(response_body.length()) + "\r\n\r\n" + response_body;
}

std::string SimpleWebServer::handleGetMeals(const std::string& request) {
    std::string userIdStr = getQueryParam(request, "userId");
    if (userIdStr.empty()) return "HTTP/1.1 400 Bad Request\r\n\r\n";
    
    int userId = std::stoi(userIdStr);
    auto meals = db->getMealsByUser(userId);
    
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < meals.size(); ++i) {
        const auto& meal = meals[i];
        
        std::string mealName;
        auto foods = meal.getFoods();
        for(size_t j=0; j<foods.size(); ++j) {
            if(j>0) mealName += " + ";
            mealName += foods[j].getName();
        }
        
        ss << "{\"id\":" << meal.getId() 
           << ",\"name\":\"" << escapeJSON(mealName) << "\""
           << ",\"type\":\"" << escapeJSON(meal.getMealType()) << "\""
           << ",\"date\":\"" << escapeJSON(meal.getDate()) << "\""
           << ",\"calories\":" << meal.getTotalCalories()
           << ",\"protein\":" << meal.getTotalProtein()
           << ",\"carbs\":" << meal.getTotalCarbs()
           << ",\"fat\":" << meal.getTotalFat()
           << "}";
        
        if (i < meals.size() - 1) ss << ",";
    }
    ss << "]";
    
    std::string response_body = ss.str();
    return "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " + 
           std::to_string(response_body.length()) + "\r\n\r\n" + response_body;
}

std::string SimpleWebServer::handleGetStatistics(const std::string& request) {
    std::string userIdStr = getQueryParam(request, "userId");
    if (userIdStr.empty()) return "HTTP/1.1 400 Bad Request\r\n\r\n";
    
    int userId = std::stoi(userIdStr);
    auto meals = db->getMealsByUser(userId);
    
    double avgCal = 0, avgProt = 0, avgCarb = 0, avgFat = 0;
    if (!meals.empty()) {
        double totalCal = 0, totalProt = 0, totalCarb = 0, totalFat = 0;
        for (const auto& meal : meals) {
            totalCal += meal.getTotalCalories();
            totalProt += meal.getTotalProtein();
            totalCarb += meal.getTotalCarbs();
            totalFat += meal.getTotalFat();
        }
        // Assuming meals are stored individually, we might want to group by day. 
        // For simplicity, let's just average per meal or if we assume 3 meals a day, we can estimate daily.
        // Actually the frontend expects daily average.
        // Let's count unique days.
        std::set<std::string> days;
        for(const auto& meal : meals) days.insert(meal.getDate());
        
        int dayCount = days.size();
        if (dayCount == 0) dayCount = 1;
        
        avgCal = totalCal / dayCount;
        avgProt = totalProt / dayCount;
        avgCarb = totalCarb / dayCount;
        avgFat = totalFat / dayCount;
    }
    
    std::stringstream ss;
    ss << "{\"avgCalories\":" << avgCal
       << ",\"avgProtein\":" << avgProt
       << ",\"avgCarbs\":" << avgCarb
       << ",\"avgFat\":" << avgFat
       << "}";
       
    std::string response_body = ss.str();
    return "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " + 
           std::to_string(response_body.length()) + "\r\n\r\n" + response_body;
}

void SimpleWebServer::cleanup() {
    if (server_socket != INVALID_SOCKET) {
        closesocket(server_socket);
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

SimpleWebServer* CreateWebServer(Database* db, RecommendationEngine* engine, int port) {
    return new SimpleWebServer(db, engine, port);
}

void DeleteWebServer(SimpleWebServer* server) {
    delete server;
}
