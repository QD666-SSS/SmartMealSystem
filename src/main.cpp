#include "../include/User.h"
#include "../include/Food.h"
#include "../include/Meal.h"
#include "../include/Database.h"
#include "../include/RecommendationEngine.h"
#include "../include/Utils.h"
#include <iostream>
#include <string>
#include <limits>
#include <iomanip>
#include <map>

class MealRecommendationSystem {
private:
    Database db;
    RecommendationEngine engine;
    User* currentUser;

public:
    MealRecommendationSystem() : currentUser(nullptr) {
        db = Database("data/users.txt", "data/foods.txt", "data/meals.txt");
    }

    void run() {
        if (!db.loadFoods()) {
            std::cout << "首次运行，初始化数据..." << std::endl;
            db.initializeSampleData();
        }
        db.loadUsers();
        db.loadMeals();
        
        engine.setFoodDatabase(db.getAllFoods());
        
        showWelcome();
        
        while (true) {
            if (currentUser == nullptr) {
                if (!loginMenu()) {
                    break;
                }
            } else {
                mainMenu();
            }
        }
        
        std::cout << "\n感谢使用智能配餐推荐系统！再见！" << std::endl;
    }

private:
    void showWelcome() {
        Utils::clearScreen();
        std::cout << "============================================" << std::endl;
        std::cout << "                                      " << std::endl;
        std::cout << "          智能配餐推荐系统 v1.0             " << std::endl;
        std::cout << "                                      " << std::endl;
        std::cout << "     Intelligent Meal Recommendation System " << std::endl;
        std::cout << "                                      " << std::endl;
        std::cout << "============================================" << std::endl;
        std::cout << std::endl;
    }

    bool loginMenu() {
        std::cout << "\n=================================" << std::endl;
        std::cout << "         欢迎使用                   " << std::endl;
        std::cout << "=================================" << std::endl;
        std::cout << "  1. 用户登录                      " << std::endl;
        std::cout << "  2. 用户注册                      " << std::endl;
        std::cout << "  0. 退出系统                      " << std::endl;
        std::cout << "=================================" << std::endl;
        
        int choice = Utils::getIntInput("\n请选择 (0-2): ");
        
        switch (choice) {
            case 1:
                login();
                break;
            case 2:
                registerUser();
                break;
            case 0:
                return false;
            default:
                std::cout << "无效选择，请重试。" << std::endl;
        }
        return true;
    }

    void login() {
        std::cout << "\n=== 用户登录 ===" << std::endl;
        std::string username = Utils::getStringInput("请输入用户名: ");
        std::string password = Utils::getStringInput("请输入密码: ");
        
        auto users = db.getAllUsers();
        for (auto& user : users) {
            if (user.getUsername() == username && user.getPassword() == password) {
                currentUser = &user;
                std::cout << "\n登录成功！" << std::endl;
                return;
            }
        }
        
        std::cout << "\n用户名或密码错误！" << std::endl;
    }

    void registerUser() {
        std::cout << "\n=== 用户注册 ===" << std::endl;
        
        std::string username;
        while (true) {
            username = Utils::getStringInput("请输入用户名: ");
            if (username.empty()) {
                std::cout << "用户名不能为空！" << std::endl;
                continue;
            }
            
            auto users = db.getAllUsers();
            bool exists = false;
            for (const auto& user : users) {
                if (user.getUsername() == username) {
                    exists = true;
                    break;
                }
            }
            if (exists) {
                std::cout << "用户名已存在，请选择其他用户名！" << std::endl;
            } else {
                break;
            }
        }
        
        std::string password = Utils::getStringInput("请输入密码: ");
        
        int newId = db.getNextUserId();
        User newUser(newId, username, password);
        
        // 设置基本信息
        std::cout << "\n=== 设置基本信息 ===" << std::endl;
        newUser.setAge(Utils::getIntInput("请输入年龄: "));
        newUser.setWeight(Utils::getDoubleInput("请输入体重(kg): "));
        newUser.setHeight(Utils::getDoubleInput("请输入身高(cm): "));
        
        std::cout << "请选择性别: " << std::endl;
        std::cout << "1. 男性" << std::endl;
        std::cout << "2. 女性" << std::endl;
        int genderChoice = Utils::getIntInput("选择 (1-2): ");
        newUser.setGender(genderChoice == 1 ? "male" : "female");
        
        std::cout << "\n请选择活动水平: " << std::endl;
        std::cout << "1. 久坐 (很少运动)" << std::endl;
        std::cout << "2. 轻度活动 (轻度运动1-3天/周)" << std::endl;
        std::cout << "3. 中度活动 (中度运动3-5天/周)" << std::endl;
        std::cout << "4. 高强度活动 (高强度运动6-7天/周)" << std::endl;
        std::cout << "5. 极高强度 (每天高强度运动)" << std::endl;
        int activityChoice = Utils::getIntInput("选择 (1-5): ");
        std::string activityLevels[] = {"sedentary", "light", "moderate", "active", "very_active"};
        newUser.setActivityLevel(activityLevels[activityChoice - 1]);
        
        // 计算营养目标
        newUser.calculateNutritionGoals();
        
        db.saveUser(newUser);
        
        std::cout << "\n注册成功！" << std::endl;
        currentUser = &db.getAllUsers().back();
    }

    void mainMenu() {
        Utils::clearScreen();
        std::cout << "\n=================================" << std::endl;
        std::cout << "         主菜单                   " << std::endl;
        std::cout << "=================================" << std::endl;
        std::cout << "  欢迎回来，" << currentUser->getUsername() << "!" << std::endl;
        std::cout << "=================================" << std::endl;
        std::cout << "  1. 查看个人信息               " << std::endl;
        std::cout << "  2. 查看食物数据库             " << std::endl;
        std::cout << "  3. 生成餐单推荐               " << std::endl;
        std::cout << "  4. 管理历史餐单               " << std::endl;
        std::cout << "  5. 口味偏好管理               " << std::endl;
        std::cout << "  6. 营养统计                   " << std::endl;
        std::cout << "  0. 退出登录                   " << std::endl;
        std::cout << "=================================" << std::endl;
        
        int choice = Utils::getIntInput("\n请选择 (0-6): ");
        
        switch (choice) {
            case 1:
                showUserProfile();
                break;
            case 2:
                showFoodDatabase();
                break;
            case 3:
                generateMealRecommendation();
                break;
            case 4:
                manageMealHistory();
                break;
            case 5:
                managePreferences();
                break;
            case 6:
                showNutritionStats();
                break;
            case 0:
                currentUser = nullptr;
                return;
            default:
                std::cout << "无效选择，请重试。" << std::endl;
        }
        
        Utils::pause();
    }

    void showUserProfile() {
        currentUser->displayProfile();
        
        std::cout << "\n是否需要修改资料？ (y/n): ";
        std::string choice;
        std::cin >> choice;
        
        if (choice == "y" || choice == "Y") {
            std::cout << "\n=== 修改个人信息 ===" << std::endl;
            currentUser->setAge(Utils::getIntInput("请输入年龄: "));
            currentUser->setWeight(Utils::getDoubleInput("请输入体重(kg): "));
            currentUser->setHeight(Utils::getDoubleInput("请输入身高(cm): "));
            
            std::cout << "请选择性别: " << std::endl;
            std::cout << "1. 男性" << std::endl;
            std::cout << "2. 女性" << std::endl;
            int genderChoice = Utils::getIntInput("选择 (1-2): ");
            currentUser->setGender(genderChoice == 1 ? "male" : "female");
            
            std::cout << "\n请选择活动水平: " << std::endl;
            std::cout << "1. 久坐 (很少运动)" << std::endl;
            std::cout << "2. 轻度活动 (轻度运动1-3天/周)" << std::endl;
            std::cout << "3. 中度活动 (中度运动3-5天/周)" << std::endl;
            std::cout << "4. 高强度活动 (高强度运动6-7天/周)" << std::endl;
            std::cout << "5. 极高强度 (每天高强度运动)" << std::endl;
            int activityChoice = Utils::getIntInput("选择 (1-5): ");
            std::string activityLevels[] = {"sedentary", "light", "moderate", "active", "very_active"};
            currentUser->setActivityLevel(activityLevels[activityChoice - 1]);
            
            currentUser->calculateNutritionGoals();
            db.updateUser(*currentUser);
            
            std::cout << "\n资料更新成功！" << std::endl;
        }
    }

    void showFoodDatabase() {
        auto foods = db.getAllFoods();
        
        std::cout << "\n=== 食物数据库 ===" << std::endl;
        std::cout << "总计 " << foods.size() << " 种食物" << std::endl;
        
        while (true) {
            std::cout << "\n选择操作: " << std::endl;
            std::cout << "1. 浏览所有食物" << std::endl;
            std::cout << "2. 按类别查看" << std::endl;
            std::cout << "3. 按标签搜索" << std::endl;
            std::cout << "0. 返回主菜单" << std::endl;
            
            int choice = Utils::getIntInput("选择 (0-3): ");
            
            switch (choice) {
                case 1:
                    for (const auto& food : foods) {
                        food.displayInfo();
                    }
                    break;
                case 2:
                    showFoodsByCategory();
                    break;
                case 3:
                    searchFoodsByTags();
                    break;
                case 0:
                    return;
                default:
                    std::cout << "无效选择！" << std::endl;
            }
        }
    }

    void showFoodsByCategory() {
        auto foods = db.getAllFoods();
        std::set<std::string> categories;
        
        for (const auto& food : foods) {
            categories.insert(food.getCategory());
        }
        
        std::cout << "\n=== 按类别查看 ===" << std::endl;
        std::vector<std::string> categoryList(categories.begin(), categories.end());
        
        for (size_t i = 0; i < categoryList.size(); ++i) {
            std::cout << (i+1) << ". " << categoryList[i] << std::endl;
        }
        
        int choice = Utils::getIntInput("选择类别 (1-" + std::to_string(categoryList.size()) + "): ");
        
        if (choice >= 1 && choice <= categoryList.size()) {
            std::string selectedCategory = categoryList[choice-1];
            std::cout << "\n=== " << selectedCategory << " ===" << std::endl;
            
            for (const auto& food : foods) {
                if (food.getCategory() == selectedCategory) {
                    food.displayInfo();
                }
            }
        }
    }

    void searchFoodsByTags() {
        std::cout << "\n=== 按标签搜索 ===" << std::endl;
        std::string tag = Utils::getStringInput("请输入要搜索的标签: ");
        
        auto foods = db.getAllFoods();
        std::vector<Food> matchingFoods;
        
        for (const auto& food : foods) {
            if (food.hasTag(tag)) {
                matchingFoods.push_back(food);
            }
        }
        
        if (matchingFoods.empty()) {
            std::cout << "未找到包含该标签的食物。" << std::endl;
        } else {
            std::cout << "\n找到 " << matchingFoods.size() << " 种匹配的食物:" << std::endl;
            for (const auto& food : matchingFoods) {
                food.displayInfo();
            }
        }
    }

    void generateMealRecommendation() {
        std::cout << "\n=== 生成餐单推荐 ===" << std::endl;
        
        std::string date = Utils::getStringInput("请输入日期 (格式: YYYY-MM-DD): ");
        
        auto recommendation = engine.recommendDailyMeals(*currentUser, date);
        
        std::cout << "\n推荐餐单已生成！" << std::endl;
        std::cout << "是否保存此推荐？ (y/n): ";
        std::string saveChoice;
        std::cin >> saveChoice;
        
        if (saveChoice == "y" || saveChoice == "Y") {
            for (auto& meal : recommendation) {
                meal.setUserId(currentUser->getId());
                db.saveMeal(meal);
            }
            std::cout << "推荐餐单已保存！" << std::endl;
        }
        
        engine.displayRecommendationStats(recommendation);
    }

    void manageMealHistory() {
        auto meals = db.getMealsByUser(currentUser->getId());
        
        if (meals.empty()) {
            std::cout << "\n暂无历史餐单记录。" << std::endl;
            return;
        }
        
        std::cout << "\n=== 历史餐单管理 ===" << std::endl;
        
        // 按日期分组显示
        std::map<std::string, std::vector<Meal>> mealsByDate;
        for (const auto& meal : meals) {
            mealsByDate[meal.getDate()].push_back(meal);
        }
        
        std::cout << "找到 " << meals.size() << " 个餐单记录，按日期分组显示:" << std::endl;
        
        for (const auto& dateGroup : mealsByDate) {
            std::cout << "\n=== " << dateGroup.first << " ===" << std::endl;
            for (const auto& meal : dateGroup.second) {
                meal.displayMeal();
            }
        }
    }

    void managePreferences() {
        std::cout << "\n=== 口味偏好管理 ===" << std::endl;
        
        std::cout << "当前偏好:" << std::endl;
        std::cout << "喜欢的标签: ";
        auto preferredTags = currentUser->getPreferredTags();
        for (const auto& tag : preferredTags) {
            std::cout << tag << " ";
        }
        std::cout << std::endl;
        
        std::cout << "避免的标签: ";
        auto avoidedTags = currentUser->getAvoidedTags();
        for (const auto& tag : avoidedTags) {
            std::cout << tag << " ";
        }
        std::cout << std::endl;
        
        std::cout << "过敏源: ";
        auto allergens = currentUser->getAllergens();
        for (const auto& allergen : allergens) {
            std::cout << allergen << " ";
        }
        std::cout << std::endl;
        
        std::cout << "\n选择操作:" << std::endl;
        std::cout << "1. 添加喜欢的标签" << std::endl;
        std::cout << "2. 添加避免的标签" << std::endl;
        std::cout << "3. 添加过敏源" << std::endl;
        std::cout << "4. 移除喜欢的标签" << std::endl;
        std::cout << "5. 移除避免的标签" << std::endl;
        std::cout << "6. 移除过敏源" << std::endl;
        std::cout << "0. 返回" << std::endl;
        
        int choice = Utils::getIntInput("选择 (0-6): ");
        
        switch (choice) {
            case 1: {
                std::string tag = Utils::getStringInput("请输入喜欢的标签: ");
                currentUser->addPreferredTag(tag);
                break;
            }
            case 2: {
                std::string tag = Utils::getStringInput("请输入要避免的标签: ");
                currentUser->addAvoidedTag(tag);
                break;
            }
            case 3: {
                std::string allergen = Utils::getStringInput("请输入过敏源: ");
                currentUser->addAllergen(allergen);
                break;
            }
            case 4: {
                std::string tag = Utils::getStringInput("请输入要移除的喜欢标签: ");
                // 这里需要实现移除方法
                break;
            }
            case 5: {
                std::string tag = Utils::getStringInput("请输入要移除的避免标签: ");
                // 这里需要实现移除方法
                break;
            }
            case 6: {
                std::string allergen = Utils::getStringInput("请输入要移除的过敏源: ");
                // 这里需要实现移除方法
                break;
            }
            case 0:
                return;
            default:
                std::cout << "无效选择！" << std::endl;
        }
        
        db.updateUser(*currentUser);
        std::cout << "偏好设置已更新！" << std::endl;
    }

    void showNutritionStats() {
        auto meals = db.getMealsByUser(currentUser->getId());
        
        if (meals.empty()) {
            std::cout << "\n暂无餐单记录用于统计。" << std::endl;
            return;
        }
        
        std::cout << "\n=== 营养统计 ===" << std::endl;
        
        std::map<std::string, std::pair<double, int>> stats;
        double totalCalories = 0, totalProtein = 0, totalCarbs = 0, totalFat = 0;
        int mealCount = 0;
        
        for (const auto& meal : meals) {
            totalCalories += meal.getTotalCalories();
            totalProtein += meal.getTotalProtein();
            totalCarbs += meal.getTotalCarbs();
            totalFat += meal.getTotalFat();
            mealCount++;
        }
        
        if (mealCount > 0) {
            double avgCalories = totalCalories / mealCount;
            double avgProtein = totalProtein / mealCount;
            double avgCarbs = totalCarbs / mealCount;
            double avgFat = totalFat / mealCount;
            
            std::cout << "总餐单数: " << mealCount << std::endl;
            std::cout << "平均每餐营养:" << std::endl;
            std::cout << "  热量: " << std::fixed << std::setprecision(1) << avgCalories << " kcal" << std::endl;
            std::cout << "  蛋白质: " << avgProtein << " g" << std::endl;
            std::cout << "  碳水化合物: " << avgCarbs << " g" << std::endl;
            std::cout << "  脂肪: " << avgFat << " g" << std::endl;
            
            std::cout << "\n营养目标完成情况:" << std::endl;
            double calorieCompletion = (avgCalories / currentUser->getDailyCalorieGoal()) * 100;
            double proteinCompletion = (avgProtein / currentUser->getDailyProteinGoal()) * 100;
            
            std::cout << "  热量完成度: " << std::fixed << std::setprecision(1) << calorieCompletion << "%" << std::endl;
            std::cout << "  蛋白质完成度: " << std::fixed << std::setprecision(1) << proteinCompletion << "%" << std::endl;
        }
    }
};

int main() {
    MealRecommendationSystem system;
    system.run();
    return 0;
}