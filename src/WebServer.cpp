#include "../include/WebServer.h"
#include "../include/third_party/httplib.h"
#include <iostream>
#include <sstream>
#include <random>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#else
#include <cstdlib>
#endif

WebServer::WebServer(int port, const std::string& wwwRoot) 
    : port(port), wwwRoot(wwwRoot) {
    db = Database("data/users.txt", "data/foods.txt", "data/meals.txt");
    
    if (!db.loadFoods()) {
        std::cout << u8"首次运行，初始化数据..." << std::endl;
        db.initializeSampleData();
    }
    db.loadUsers();
    db.loadMeals();
    
    engine.setFoodDatabase(db.getAllFoods());
    
    std::map<int, std::vector<Meal>> allHistory;
    for (const auto& meal : db.getAllMeals()) {
        allHistory[meal.getUserId()].push_back(meal);
    }
    engine.loadHistory(allHistory);
}

std::string WebServer::generateSessionToken() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    for (int i = 0; i < 32; i++) {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}

std::string WebServer::createJsonResponse(bool success, const std::string& message, const std::string& data) {
    std::stringstream ss;
    ss << "{\"success\":" << (success ? "true" : "false")
       << ",\"message\":\"" << message << "\"";
    if (!data.empty()) {
        ss << ",\"data\":" << data;
    }
    ss << "}";
    return ss.str();
}

std::string WebServer::urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value;
            std::istringstream is(str.substr(i + 1, 2));
            if (is >> std::hex >> value) {
                result += static_cast<char>(value);
                i += 2;
            } else {
                result += str[i];
            }
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

void WebServer::reloadEngineHistory() {
    std::map<int, std::vector<Meal>> allHistory;
    for (const auto& meal : db.getAllMeals()) {
        allHistory[meal.getUserId()].push_back(meal);
    }
    engine.loadHistory(allHistory);
}

std::string WebServer::parseJsonString(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return "";
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return "";
    
    pos = json.find("\"", pos);
    if (pos == std::string::npos) return "";
    pos++;
    
    size_t endPos = json.find("\"", pos);
    if (endPos == std::string::npos) return "";
    
    return json.substr(pos, endPos - pos);
}

int WebServer::parseJsonInt(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return 0;
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return 0;
    pos++;
    
    while (pos < json.length() && std::isspace(json[pos])) pos++;
    
    size_t endPos = pos;
    while (endPos < json.length() && (std::isdigit(json[endPos]) || json[endPos] == '-')) {
        endPos++;
    }
    
    if (endPos > pos) {
        return std::stoi(json.substr(pos, endPos - pos));
    }
    return 0;
}

double WebServer::parseJsonDouble(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return 0.0;
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return 0.0;
    pos++;
    
    while (pos < json.length() && std::isspace(json[pos])) pos++;
    
    size_t endPos = pos;
    while (endPos < json.length() && (std::isdigit(json[endPos]) || json[endPos] == '.' || json[endPos] == '-')) {
        endPos++;
    }
    
    if (endPos > pos) {
        return std::stod(json.substr(pos, endPos - pos));
    }
    return 0.0;
}

std::string WebServer::userToJson(const User& user) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << "{"
       << "\"id\":" << user.getId() << ","
       << "\"username\":\"" << user.getUsername() << "\","
       << "\"age\":" << user.getAge() << ","
       << "\"weight\":" << user.getWeight() << ","
       << "\"height\":" << user.getHeight() << ","
       << "\"gender\":\"" << user.getGender() << "\","
       << "\"activityLevel\":\"" << user.getActivityLevel() << "\","
       << "\"dailyCalorieGoal\":" << user.getDailyCalorieGoal() << ","
       << "\"dailyProteinGoal\":" << user.getDailyProteinGoal() << ","
       << "\"dailyCarbsGoal\":" << user.getDailyCarbGoal() << ","
       << "\"dailyFatGoal\":" << user.getDailyFatGoal()
       << "}";
    return ss.str();
}

std::string WebServer::foodToJson(const Food& food) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << "{"
       << "\"id\":" << food.getId() << ","
       << "\"name\":\"" << food.getName() << "\","
       << "\"category\":\"" << food.getCategory() << "\","
       << "\"calories\":" << food.getCalories() << ","
       << "\"protein\":" << food.getProtein() << ","
       << "\"carbs\":" << food.getCarbohydrates() << ","
       << "\"fat\":" << food.getFat() << ","
       << "\"tags\":[";
    
    const auto& tags = food.getTags();
    bool first = true;
    for (const auto& tag : tags) {
        if (!first) ss << ",";
        ss << "\"" << tag << "\"";
        first = false;
    }
    ss << "]}";
    return ss.str();
}

std::string WebServer::mealToJson(const Meal& meal) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << "{"
       << "\"id\":" << meal.getId() << ","
       << "\"userId\":" << meal.getUserId() << ","
       << "\"mealType\":\"" << meal.getMealType() << "\","
       << "\"date\":\"" << meal.getDate() << "\","
       << "\"totalCalories\":" << meal.getTotalCalories() << ","
       << "\"totalProtein\":" << meal.getTotalProtein() << ","
       << "\"totalCarbs\":" << meal.getTotalCarbs() << ","
       << "\"totalFat\":" << meal.getTotalFat() << ","
       << "\"foods\":[";
    
    const auto& foods = meal.getFoods();
    bool first = true;
    for (const auto& food : foods) {
        if (!first) ss << ",";
        ss << foodToJson(food);
        first = false;
    }
    ss << "]}";
    return ss.str();
}

std::string WebServer::foodsArrayToJson(const std::vector<Food>& foods) {
    std::stringstream ss;
    ss << "[";
    bool first = true;
    for (const auto& food : foods) {
        if (!first) ss << ",";
        ss << foodToJson(food);
        first = false;
    }
    ss << "]";
    return ss.str();
}

std::string WebServer::mealsArrayToJson(const std::vector<Meal>& meals) {
    std::stringstream ss;
    ss << "[";
    bool first = true;
    for (const auto& meal : meals) {
        if (!first) ss << ",";
        ss << mealToJson(meal);
        first = false;
    }
    ss << "]";
    return ss.str();
}

void WebServer::openBrowser(const std::string& url) {
#ifdef _WIN32
    ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif __APPLE__
    std::string cmd = "open " + url;
    system(cmd.c_str());
#else
    std::string cmd = "xdg-open " + url + " 2>/dev/null &";
    system(cmd.c_str());
#endif
}

void WebServer::start() {
    httplib::Server svr;
    
    svr.set_mount_point("/", wwwRoot.c_str());
    
    svr.Post("/api/login", [this](const httplib::Request& req, httplib::Response& res) {
        std::string username = parseJsonString(req.body, "username");
        std::string password = parseJsonString(req.body, "password");
        
        auto users = db.getAllUsers();
        for (const auto& user : users) {
            if (user.getUsername() == username && user.getPassword() == password) {
                std::string token = generateSessionToken();
                sessions[token] = user;
                
                std::string data = "{\"token\":\"" + token + "\",\"user\":" + userToJson(user) + "}";
                res.set_content(createJsonResponse(true, u8"登录成功", data), "application/json; charset=utf-8");
                return;
            }
        }
        
        res.set_content(createJsonResponse(false, u8"用户名或密码错误"), "application/json; charset=utf-8");
    });
    
    svr.Post("/api/register", [this](const httplib::Request& req, httplib::Response& res) {
        std::string username = parseJsonString(req.body, "username");
        std::string password = parseJsonString(req.body, "password");
        int age = parseJsonInt(req.body, "age");
        double weight = parseJsonDouble(req.body, "weight");
        double height = parseJsonDouble(req.body, "height");
        std::string gender = parseJsonString(req.body, "gender");
        std::string activityLevel = parseJsonString(req.body, "activityLevel");
        
        auto users = db.getAllUsers();
        for (const auto& user : users) {
            if (user.getUsername() == username) {
                res.set_content(createJsonResponse(false, u8"用户名已存在"), "application/json; charset=utf-8");
                return;
            }
        }
        
        int newId = db.getNextUserId();
        User newUser(newId, username, password);
        newUser.setAge(age);
        newUser.setWeight(weight);
        newUser.setHeight(height);
        newUser.setGender(gender);
        newUser.setActivityLevel(activityLevel);
        newUser.calculateNutritionGoals();
        
        db.saveUser(newUser);
        
        std::string token = generateSessionToken();
        sessions[token] = newUser;
        
        std::string data = "{\"token\":\"" + token + "\",\"user\":" + userToJson(newUser) + "}";
        res.set_content(createJsonResponse(true, u8"注册成功", data), "application/json; charset=utf-8");
    });
    
    svr.Get("/api/user/profile", [this](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.get_header_value("Authorization");
        if (token.find("Bearer ") == 0) {
            token = token.substr(7);
        }
        
        if (sessions.find(token) == sessions.end()) {
            res.set_content(createJsonResponse(false, u8"未登录或会话已过期"), "application/json; charset=utf-8");
            return;
        }
        
        User& user = sessions[token];
        res.set_content(createJsonResponse(true, "OK", userToJson(user)), "application/json; charset=utf-8");
    });
    
    svr.Put("/api/user/profile", [this](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.get_header_value("Authorization");
        if (token.find("Bearer ") == 0) {
            token = token.substr(7);
        }
        
        if (sessions.find(token) == sessions.end()) {
            res.set_content(createJsonResponse(false, u8"未登录或会话已过期"), "application/json; charset=utf-8");
            return;
        }
        
        User& user = sessions[token];
        
        int age = parseJsonInt(req.body, "age");
        double weight = parseJsonDouble(req.body, "weight");
        double height = parseJsonDouble(req.body, "height");
        std::string gender = parseJsonString(req.body, "gender");
        std::string activityLevel = parseJsonString(req.body, "activityLevel");
        
        if (age > 0) user.setAge(age);
        if (weight > 0) user.setWeight(weight);
        if (height > 0) user.setHeight(height);
        if (!gender.empty()) user.setGender(gender);
        if (!activityLevel.empty()) user.setActivityLevel(activityLevel);
        
        user.calculateNutritionGoals();
        db.updateUser(user);
        
        res.set_content(createJsonResponse(true, u8"更新成功", userToJson(user)), "application/json; charset=utf-8");
    });
    
    svr.Get("/api/foods", [this](const httplib::Request& req, httplib::Response& res) {
        auto foods = db.getAllFoods();
        res.set_content(createJsonResponse(true, "OK", foodsArrayToJson(foods)), "application/json; charset=utf-8");
    });
    
    svr.Get("/api/meals/history", [this](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.get_header_value("Authorization");
        if (token.find("Bearer ") == 0) {
            token = token.substr(7);
        }
        
        if (sessions.find(token) == sessions.end()) {
            res.set_content(createJsonResponse(false, u8"未登录或会话已过期"), "application/json; charset=utf-8");
            return;
        }
        
        User& user = sessions[token];
        auto meals = db.getMealsByUser(user.getId());
        res.set_content(createJsonResponse(true, "OK", mealsArrayToJson(meals)), "application/json; charset=utf-8");
    });
    
    svr.Post("/api/meals/recommend", [this](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.get_header_value("Authorization");
        if (token.find("Bearer ") == 0) {
            token = token.substr(7);
        }
        
        if (sessions.find(token) == sessions.end()) {
            res.set_content(createJsonResponse(false, u8"未登录或会话已过期"), "application/json; charset=utf-8");
            return;
        }
        
        User& user = sessions[token];
        std::string date = parseJsonString(req.body, "date");
        
        auto recommendation = engine.recommendDailyMeals(user, date);
        res.set_content(createJsonResponse(true, u8"推荐生成成功", mealsArrayToJson(recommendation)), "application/json; charset=utf-8");
    });
    
    svr.Post("/api/meals/save", [this](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.get_header_value("Authorization");
        if (token.find("Bearer ") == 0) {
            token = token.substr(7);
        }
        
        if (sessions.find(token) == sessions.end()) {
            res.set_content(createJsonResponse(false, u8"未登录或会话已过期"), "application/json; charset=utf-8");
            return;
        }
        
        User& user = sessions[token];
        std::string date = parseJsonString(req.body, "date");
        if (date.empty()) {
            res.set_content(createJsonResponse(false, u8"日期不能为空"), "application/json; charset=utf-8");
            return;
        }

        bool hasSameDayRecommended = false;
        {
            auto existingMeals = db.getMealsByUser(user.getId());
            for (const auto& meal : existingMeals) {
                if (meal.getDate() == date && meal.getIsRecommended()) {
                    hasSameDayRecommended = true;
                    break;
                }
            }
        }

        if (hasSameDayRecommended) {
            if (!db.deleteMealsByUserAndDate(user.getId(), date, true)) {
                res.set_content(createJsonResponse(false, u8"替换失败：无法删除旧记录"), "application/json; charset=utf-8");
                return;
            }
            reloadEngineHistory();
        }

        auto recommendation = engine.recommendDailyMeals(user, date);
        for (auto& meal : recommendation) {
            meal.setId(db.getNextMealId());
            meal.setUserId(user.getId());
            if (!db.saveMeal(meal)) {
                res.set_content(createJsonResponse(false, u8"保存失败"), "application/json; charset=utf-8");
                return;
            }
            engine.addToHistory(user.getId(), meal);
        }

        std::string message = hasSameDayRecommended ? u8"同日旧推荐已替换并保存成功" : u8"餐单保存成功";
        res.set_content(createJsonResponse(true, message), "application/json; charset=utf-8");
    });
    
    svr.Delete(R"(/api/meals/date/([\d\-]+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.get_header_value("Authorization");
        if (token.find("Bearer ") == 0) {
            token = token.substr(7);
        }

        if (sessions.find(token) == sessions.end()) {
            res.set_content(createJsonResponse(false, u8"未登录或会话已过期"), "application/json; charset=utf-8");
            return;
        }

        User& user = sessions[token];
        std::string date = urlDecode(req.matches[1]);
        if (date.empty()) {
            res.set_content(createJsonResponse(false, u8"日期不能为空"), "application/json; charset=utf-8");
            return;
        }

        if (!db.deleteMealsByUserAndDate(user.getId(), date)) {
            res.set_content(createJsonResponse(false, u8"删除失败"), "application/json; charset=utf-8");
            return;
        }
        reloadEngineHistory();

        res.set_content(createJsonResponse(true, u8"删除成功"), "application/json; charset=utf-8");
    });

    svr.Delete(R"(/api/meals/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.get_header_value("Authorization");
        if (token.find("Bearer ") == 0) {
            token = token.substr(7);
        }
        
        if (sessions.find(token) == sessions.end()) {
            res.set_content(createJsonResponse(false, u8"未登录或会话已过期"), "application/json; charset=utf-8");
            return;
        }

        User& user = sessions[token];
        int mealId = std::stoi(req.matches[1]);

        auto mealOpt = db.getMealById(mealId);
        if (!mealOpt || mealOpt->getUserId() != user.getId()) {
            res.set_content(createJsonResponse(false, u8"无权删除该餐单"), "application/json; charset=utf-8");
            return;
        }

        if (!db.deleteMeal(mealId)) {
            res.set_content(createJsonResponse(false, u8"删除失败"), "application/json; charset=utf-8");
            return;
        }

        reloadEngineHistory();
        res.set_content(createJsonResponse(true, u8"删除成功"), "application/json; charset=utf-8");
    });
    
    std::cout << u8"============================================" << std::endl;
    std::cout << u8"  智能配餐推荐系统 Web 服务器               " << std::endl;
    std::cout << u8"============================================" << std::endl;
    std::cout << u8"服务器启动在: http://localhost:" << port << std::endl;
    std::cout << u8"正在自动打开浏览器..." << std::endl;
    std::cout << u8"按 Ctrl+C 停止服务器" << std::endl;
    std::cout << u8"============================================" << std::endl;
    
    std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        openBrowser("http://localhost:" + std::to_string(port));
    }).detach();
    
    svr.listen("0.0.0.0", port);
}
