#include "WebServer.h"
#include "User.h"
#include "Food.h"
#include "Meal.h"
#include "Database.h"
#include "RecommendationEngine.h"
#include <iostream>
#include <thread>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

// 全局数据库实例
std::unique_ptr<Database> g_db;
std::unique_ptr<RecommendationEngine> g_engine;

class WebMealRecommendationServer {
private:
    int port;
    Database& db;
    RecommendationEngine& engine;

public:
    WebMealRecommendationServer(int port, Database& database, RecommendationEngine& engine) 
        : port(port), db(database), engine(engine) {}

    void start() {
        std::cout << "============================================" << std::endl;
        std::cout << "                                      " << std::endl;
        std::cout << "          智能配餐推荐系统 Web版            " << std::endl;
        std::cout << "                                      " << std::endl;
        std::cout << "     Intelligent Meal Recommendation System " << std::endl;
        std::cout << "                                      " << std::endl;
        std::cout << "============================================" << std::endl;
        std::cout << std::endl;
        
        std::cout << "数据加载完成！" << std::endl;
        std::cout << "正在启动Web服务器..." << std::endl;
        std::cout << "请在浏览器中访问: http://localhost:8080" << std::endl;
        std::cout << "按 Ctrl+C 停止服务器" << std::endl;
        std::cout << std::endl;
        
        // 创建并启动Web服务器
        SimpleWebServer* server = CreateWebServer(&db, &engine, port);
        server->start();
        DeleteWebServer(server);
    }
};

int main() {
    try {
        // 确保数据目录存在
        if (!fs::exists("data")) {
            fs::create_directory("data");
        }
        
        // 初始化数据库
        auto db_ptr = std::make_unique<Database>("data/users.txt", "data/foods.txt", "data/meals.txt");
        
        if (!db_ptr->loadFoods()) {
            std::cout << "首次运行，初始化数据..." << std::endl;
            db_ptr->initializeSampleData();
        }
        db_ptr->loadUsers();
        db_ptr->loadMeals();
        
        // 初始化推荐引擎
        auto engine_ptr = std::make_unique<RecommendationEngine>();
        engine_ptr->setFoodDatabase(db_ptr->getAllFoods());
        
        std::map<int, std::vector<Meal>> allHistory;
        for (const auto& meal : db_ptr->getAllMeals()) {
            allHistory[meal.getUserId()].push_back(meal);
        }
        engine_ptr->loadHistory(allHistory);
        
        // 启动Web服务器
        WebMealRecommendationServer server(8080, *db_ptr, *engine_ptr);
        server.start();
        
    } catch (const std::exception& e) {
        std::cerr << "服务器启动失败: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}