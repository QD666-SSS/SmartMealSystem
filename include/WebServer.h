#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "Database.h"
#include "RecommendationEngine.h"
#include "User.h"
#include <string>
#include <memory>
#include <map>

class WebServer {
private:
    Database db;
    RecommendationEngine engine;
    std::map<std::string, User> sessions;
    int port;
    std::string wwwRoot;

    std::string generateSessionToken();
    std::string createJsonResponse(bool success, const std::string& message, const std::string& data = "");
    std::string userToJson(const User& user);
    std::string foodToJson(const Food& food);
    std::string mealToJson(const Meal& meal);
    std::string foodsArrayToJson(const std::vector<Food>& foods);
    std::string mealsArrayToJson(const std::vector<Meal>& meals);
    std::string parseJsonString(const std::string& json, const std::string& key);
    int parseJsonInt(const std::string& json, const std::string& key);
    double parseJsonDouble(const std::string& json, const std::string& key);
    std::string urlDecode(const std::string& str);

    void reloadEngineHistory();

public:
    WebServer(int port = 8000, const std::string& wwwRoot = "www");
    void start();
    void openBrowser(const std::string& url);
};

#endif
