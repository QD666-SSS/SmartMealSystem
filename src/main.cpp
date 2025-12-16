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
#include <optional>

class MealRecommendationSystem {
private:
    Database db;
    RecommendationEngine engine;
    std::optional<User> currentUser;

public:
    MealRecommendationSystem() : currentUser(std::nullopt) {
        db = Database("data/users.txt", "data/foods.txt", "data/meals.txt");
    }

    void run() {
         if (!db.loadFoods()) {
             std::cout << u8"首次运行，初始化数据..." << std::endl;
             db.initializeSampleData();
         }
        db.loadUsers();
        db.loadMeals();
        
        engine.setFoodDatabase(db.getAllFoods());
        
        showWelcome();
        
        while (true) {
            if (!currentUser.has_value()) {
                if (!loginMenu()) {
                    break;
                }
            } else {
                mainMenu();
            }
        }
        
        std::cout << u8"\n感谢使用智能配餐推荐系统！再见！" << std::endl;
    }

private:
    User& getCurrentUser() {
        return currentUser.value();
    }

    void showWelcome() {
         Utils::clearScreen();
         std::cout << "============================================" << std::endl;
         std::cout << "                                      " << std::endl;
         std::cout << u8"          智能配餐推荐系统 v1.0             " << std::endl;
         std::cout << "                                      " << std::endl;
         std::cout << "     Intelligent Meal Recommendation System " << std::endl;
         std::cout << "                                      " << std::endl;
         std::cout << "============================================" << std::endl;
         std::cout << std::endl;
     }

    bool loginMenu() {
         std::cout << "\n=================================" << std::endl;
         std::cout << u8"         欢迎使用                   " << std::endl;
         std::cout << "=================================" << std::endl;
         std::cout << u8"  1. 用户登录                      " << std::endl;
         std::cout << u8"  2. 用户注册                      " << std::endl;
         std::cout << u8"  0. 退出系统                      " << std::endl;
         std::cout << "=================================" << std::endl;

         int choice = Utils::getIntInput(u8"\n请选择 (0-2): ");
        
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
                std::cout << u8"无效选择，请重试。" << std::endl;
            }
            return true;
            }

            void login() {
            std::cout << u8"\n=== 用户登录 ===" << std::endl;
            std::string username = Utils::getStringInput(u8"请输入用户名: ");
            std::string password = Utils::getStringInput(u8"请输入密码: ");
        
        auto users = db.getAllUsers();
        for (const auto& user : users) {
            if (user.getUsername() == username && user.getPassword() == password) {
                currentUser = user;
                std::cout << u8"\n登录成功！" << std::endl;
                return;
            }
        }

        std::cout << u8"\n用户名或密码错误！" << std::endl;
    }

    void registerUser() {
         std::cout << u8"\n=== 用户注册 ===" << std::endl;

         std::string username;
         while (true) {
             username = Utils::getStringInput(u8"请输入用户名: ");
             if (username.empty()) {
                 std::cout << u8"用户名不能为空！" << std::endl;
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
                 std::cout << u8"用户名已存在，请选择其他用户名！" << std::endl;
             } else {
                 break;
             }
         }

         std::string password = Utils::getStringInput(u8"请输入密码: ");
        
        int newId = db.getNextUserId();
        User newUser(newId, username, password);
        
        std::cout << u8"\n=== 设置基本信息 ===" << std::endl;
         newUser.setAge(Utils::getIntInput(u8"请输入年龄: "));
         newUser.setWeight(Utils::getDoubleInput(u8"请输入体重(kg): "));
         newUser.setHeight(Utils::getDoubleInput(u8"请输入身高(cm): "));

         std::cout << u8"请选择性别: " << std::endl;
         std::cout << u8"1. 男性" << std::endl;
         std::cout << u8"2. 女性" << std::endl;
         int genderChoice;
         do {
             genderChoice = Utils::getIntInput(u8"选择 (1-2): ");
             if (genderChoice < 1 || genderChoice > 2) {
                 std::cout << u8"无效选择，请输入1或2。" << std::endl;
             }
         } while (genderChoice < 1 || genderChoice > 2);
         newUser.setGender(genderChoice == 1 ? "male" : "female");

         std::cout << u8"\n请选择活动水平: " << std::endl;
         std::cout << u8"1. 久坐 (很少运动)" << std::endl;
         std::cout << u8"2. 轻度活动 (轻度运动1-3天/周)" << std::endl;
         std::cout << u8"3. 中度活动 (中度运动3-5天/周)" << std::endl;
         std::cout << u8"4. 高强度活动 (高强度运动6-7天/周)" << std::endl;
         std::cout << u8"5. 极高强度 (每天高强度运动)" << std::endl;
         int activityChoice;
         do {
             activityChoice = Utils::getIntInput(u8"选择 (1-5): ");
             if (activityChoice < 1 || activityChoice > 5) {
                 std::cout << u8"无效选择，请输入1-5之间的数字。" << std::endl;
             }
         } while (activityChoice < 1 || activityChoice > 5);
         std::string activityLevels[] = {"sedentary", "light", "moderate", "active", "very_active"};
         newUser.setActivityLevel(activityLevels[activityChoice - 1]);

         newUser.calculateNutritionGoals();

         db.saveUser(newUser);

         std::cout << u8"\n注册成功！" << std::endl;
        currentUser = newUser;
    }

    void mainMenu() {
         Utils::clearScreen();
         std::cout << "\n=================================" << std::endl;
         std::cout << u8"         主菜单                   " << std::endl;
         std::cout << "=================================" << std::endl;
         std::cout << u8"  欢迎回来，" << getCurrentUser().getUsername() << u8"!" << std::endl;
         std::cout << "=================================" << std::endl;
         std::cout << u8"  1. 查看个人信息               " << std::endl;
         std::cout << u8"  2. 查看食物数据库             " << std::endl;
         std::cout << u8"  3. 生成餐单推荐               " << std::endl;
         std::cout << u8"  4. 管理历史餐单               " << std::endl;
         std::cout << u8"  5. 口味偏好管理               " << std::endl;
         std::cout << u8"  6. 营养统计                   " << std::endl;
         std::cout << u8"  0. 退出登录                   " << std::endl;
         std::cout << "=================================" << std::endl;

         int choice = Utils::getIntInput(u8"\n请选择 (0-6): ");
        
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
                currentUser = std::nullopt;
                return;
            default:
                std::cout << u8"无效选择，请重试。" << std::endl;
            }

            Utils::pause();
            }

            void showUserProfile() {
            getCurrentUser().displayProfile();

            std::cout << u8"\n是否需要修改资料？ (y/n): ";
            std::string choice;
            std::cin >> choice;

            if (choice == "y" || choice == "Y") {
             std::cout << u8"\n=== 修改个人信息 ===" << std::endl;
             getCurrentUser().setAge(Utils::getIntInput(u8"请输入年龄: "));
             getCurrentUser().setWeight(Utils::getDoubleInput(u8"请输入体重(kg): "));
             getCurrentUser().setHeight(Utils::getDoubleInput(u8"请输入身高(cm): "));

             std::cout << u8"请选择性别: " << std::endl;
             std::cout << u8"1. 男性" << std::endl;
             std::cout << u8"2. 女性" << std::endl;
             int genderChoice;
             do {
                 genderChoice = Utils::getIntInput(u8"选择 (1-2): ");
                 if (genderChoice < 1 || genderChoice > 2) {
                     std::cout << u8"无效选择，请输入1或2。" << std::endl;
                 }
             } while (genderChoice < 1 || genderChoice > 2);
             getCurrentUser().setGender(genderChoice == 1 ? "male" : "female");

             std::cout << u8"\n请选择活动水平: " << std::endl;
             std::cout << u8"1. 久坐 (很少运动)" << std::endl;
             std::cout << u8"2. 轻度活动 (轻度运动1-3天/周)" << std::endl;
             std::cout << u8"3. 中度活动 (中度运动3-5天/周)" << std::endl;
             std::cout << u8"4. 高强度活动 (高强度运动6-7天/周)" << std::endl;
             std::cout << u8"5. 极高强度 (每天高强度运动)" << std::endl;
             int activityChoice;
             do {
                 activityChoice = Utils::getIntInput(u8"选择 (1-5): ");
                 if (activityChoice < 1 || activityChoice > 5) {
                     std::cout << u8"无效选择，请输入1-5之间的数字。" << std::endl;
                 }
             } while (activityChoice < 1 || activityChoice > 5);
             std::string activityLevels[] = {"sedentary", "light", "moderate", "active", "very_active"};
             getCurrentUser().setActivityLevel(activityLevels[activityChoice - 1]);

             getCurrentUser().calculateNutritionGoals();
             db.updateUser(getCurrentUser());

             std::cout << u8"\n资料更新成功！" << std::endl;
            }
    }

    void showFoodDatabase() {
         auto foods = db.getAllFoods();

         std::cout << u8"\n=== 食物数据库 ===" << std::endl;
         std::cout << u8"总计 " << foods.size() << u8" 种食物" << std::endl;

         while (true) {
             std::cout << u8"\n选择操作: " << std::endl;
             std::cout << u8"1. 浏览所有食物" << std::endl;
             std::cout << u8"2. 按类别查看" << std::endl;
             std::cout << u8"3. 按标签搜索" << std::endl;
             std::cout << u8"0. 返回主菜单" << std::endl;

             int choice = Utils::getIntInput(u8"选择 (0-3): ");
            
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
                    std::cout << u8"无效选择！" << std::endl;
            }
        }
    }

    void showFoodsByCategory() {
        auto foods = db.getAllFoods();
        std::set<std::string> categories;
        
        for (const auto& food : foods) {
            categories.insert(food.getCategory());
        }
        
        std::cout << u8"\n=== 按类别查看 ===" << std::endl;
        std::vector<std::string> categoryList(categories.begin(), categories.end());
        
        for (size_t i = 0; i < categoryList.size(); ++i) {
            std::cout << (i+1) << ". " << categoryList[i] << std::endl;
        }
        
        int choice = Utils::getIntInput(u8"选择类别 (1-" + std::to_string(categoryList.size()) + u8"): ");
        
        if (choice >= 1 && choice <= categoryList.size()) {
             std::string selectedCategory = categoryList[choice-1];
             std::cout << u8"\n=== " << selectedCategory << u8" ===" << std::endl;
            
            for (const auto& food : foods) {
                if (food.getCategory() == selectedCategory) {
                    food.displayInfo();
                }
            }
        }
    }

    void searchFoodsByTags() {
         std::cout << u8"\n=== 按标签搜索 ===" << std::endl;
         std::string tag = Utils::getStringInput(u8"请输入要搜索的标签: ");

         auto foods = db.getAllFoods();
         std::vector<Food> matchingFoods;

         for (const auto& food : foods) {
             if (food.hasTag(tag)) {
                 matchingFoods.push_back(food);
             }
         }

         if (matchingFoods.empty()) {
             std::cout << u8"未找到包含该标签的食物。" << std::endl;
         } else {
             std::cout << u8"\n找到 " << matchingFoods.size() << u8" 种匹配的食物:" << std::endl;
             for (const auto& food : matchingFoods) {
                 food.displayInfo();
             }
         }
     }

    void generateMealRecommendation() {
         std::cout << u8"\n=== 生成餐单推荐 ===" << std::endl;

         std::string date = Utils::getStringInput(u8"请输入日期 (格式: YYYY-MM-DD): ");

         auto recommendation = engine.recommendDailyMeals(getCurrentUser(), date);

         std::cout << u8"\n推荐餐单已生成！" << std::endl;
         std::cout << u8"是否保存此推荐？ (y/n): ";
         std::string saveChoice;
         std::cin >> saveChoice;

         if (saveChoice == "y" || saveChoice == "Y") {
             for (auto& meal : recommendation) {
                 meal.setUserId(getCurrentUser().getId());
                 db.saveMeal(meal);
             }
             std::cout << u8"推荐餐单已保存！" << std::endl;
         }

         engine.displayRecommendationStats(recommendation);
     }

    void manageMealHistory() {
         auto meals = db.getMealsByUser(getCurrentUser().getId());

         if (meals.empty()) {
             std::cout << u8"\n暂无历史餐单记录。" << std::endl;
             return;
         }

         std::cout << u8"\n=== 历史餐单管理 ===" << std::endl;

         std::map<std::string, std::vector<Meal>> mealsByDate;
         for (const auto& meal : meals) {
             mealsByDate[meal.getDate()].push_back(meal);
         }

         std::cout << u8"找到 " << meals.size() << u8" 个餐单记录，按日期分组显示:" << std::endl;

         for (const auto& dateGroup : mealsByDate) {
             std::cout << u8"\n=== " << dateGroup.first << u8" ===" << std::endl;
             for (const auto& meal : dateGroup.second) {
                 meal.displayMeal();
             }
         }
     }

    void managePreferences() {
         std::cout << u8"\n=== 口味偏好管理 ===" << std::endl;

         std::cout << u8"当前偏好:" << std::endl;
         std::cout << u8"喜欢的标签: ";
         auto preferredTags = getCurrentUser().getPreferredTags();
         for (const auto& tag : preferredTags) {
             std::cout << tag << " ";
         }
         std::cout << std::endl;

         std::cout << u8"避免的标签: ";
         auto avoidedTags = getCurrentUser().getAvoidedTags();
         for (const auto& tag : avoidedTags) {
             std::cout << tag << " ";
         }
         std::cout << std::endl;

         std::cout << u8"过敏源: ";
         auto allergens = getCurrentUser().getAllergens();
         for (const auto& allergen : allergens) {
             std::cout << allergen << " ";
         }
         std::cout << std::endl;

         std::cout << u8"\n选择操作:" << std::endl;
         std::cout << u8"1. 添加喜欢的标签" << std::endl;
         std::cout << u8"2. 添加避免的标签" << std::endl;
         std::cout << u8"3. 添加过敏源" << std::endl;
         std::cout << u8"4. 移除喜欢的标签" << std::endl;
         std::cout << u8"5. 移除避免的标签" << std::endl;
         std::cout << u8"6. 移除过敏源" << std::endl;
         std::cout << u8"0. 返回" << std::endl;

         int choice = Utils::getIntInput(u8"选择 (0-6): ");

         switch (choice) {
             case 1: {
                 std::string tag = Utils::getStringInput(u8"请输入喜欢的标签: ");
                 getCurrentUser().addPreferredTag(tag);
                 break;
             }
             case 2: {
                 std::string tag = Utils::getStringInput(u8"请输入要避免的标签: ");
                 getCurrentUser().addAvoidedTag(tag);
                 break;
             }
             case 3: {
                 std::string allergen = Utils::getStringInput(u8"请输入过敏源: ");
                 getCurrentUser().addAllergen(allergen);
                 break;
             }
             case 4: {
                 std::string tag = Utils::getStringInput(u8"请输入要移除的喜欢标签: ");
                 break;
             }
             case 5: {
                 std::string tag = Utils::getStringInput(u8"请输入要移除的避免标签: ");
                 break;
             }
             case 6: {
                 std::string allergen = Utils::getStringInput(u8"请输入要移除的过敏源: ");
                 break;
             }
             case 0:
                 return;
             default:
                 std::cout << u8"无效选择！" << std::endl;
         }

         db.updateUser(getCurrentUser());
         std::cout << u8"偏好设置已更新！" << std::endl;
     }

    void showNutritionStats() {
         auto meals = db.getMealsByUser(getCurrentUser().getId());

         if (meals.empty()) {
             std::cout << u8"\n暂无餐单记录用于统计。" << std::endl;
             return;
         }

         std::cout << u8"\n=== 营养统计 ===" << std::endl;
        
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

            std::cout << u8"总餐单数: " << mealCount << std::endl;
            std::cout << u8"平均每餐营养:" << std::endl;
            std::cout << u8"  热量: " << std::fixed << std::setprecision(1) << avgCalories << u8" kcal" << std::endl;
            std::cout << u8"  蛋白质: " << avgProtein << u8" g" << std::endl;
            std::cout << u8"  碳水化合物: " << avgCarbs << u8" g" << std::endl;
            std::cout << u8"  脂肪: " << avgFat << u8" g" << std::endl;

            std::cout << u8"\n营养目标完成情况:" << std::endl;
            double calorieCompletion = (avgCalories / getCurrentUser().getDailyCalorieGoal()) * 100;
            double proteinCompletion = (avgProtein / getCurrentUser().getDailyProteinGoal()) * 100;

            std::cout << u8"  热量完成度: " << std::fixed << std::setprecision(1) << calorieCompletion << u8"%" << std::endl;
            std::cout << u8"  蛋白质完成度: " << std::fixed << std::setprecision(1) << proteinCompletion << u8"%" << std::endl;
        }
    }
};

int main() {
    MealRecommendationSystem system;
    system.run();
    return 0;
}