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
        
        std::map<int, std::vector<Meal>> allHistory;
        for (const auto& meal : db.getAllMeals()) {
            allHistory[meal.getUserId()].push_back(meal);
        }
        engine.loadHistory(allHistory);
        
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
         std::cout << u8"============================================" << std::endl;
         std::cout << u8"                                      " << std::endl;
         std::cout << u8"          智能配餐推荐系统 v2.0             " << std::endl;
         std::cout << u8"                                      " << std::endl;
         std::cout << u8"     Intelligent Meal Recommendation System " << std::endl;
         std::cout << u8"                                      " << std::endl;
         std::cout << u8"============================================" << std::endl;
         std::cout << std::endl;
     }

    bool loginMenu() {
         std::cout << u8"\n=================================" << std::endl;
         std::cout << u8"         欢迎使用                   " << std::endl;
         std::cout << u8"=================================" << std::endl;
         std::cout << u8"  1. 用户登录                      " << std::endl;
         std::cout << u8"  2. 用户注册                      " << std::endl;
         std::cout << u8"  0. 退出系统                      " << std::endl;
         std::cout << u8"=================================" << std::endl;

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
         std::cout << u8"\n=================================" << std::endl;
         std::cout << u8"         主菜单                   " << std::endl;
         std::cout << u8"=================================" << std::endl;
         std::cout << u8"  欢迎回来，" << getCurrentUser().getUsername() << u8"!" << std::endl;
         std::cout << u8"=================================" << std::endl;
         std::cout << u8"  1. 查看个人信息               " << std::endl;
         std::cout << u8"  2. 查看食物数据库             " << std::endl;
         std::cout << u8"  3. 生成餐单推荐               " << std::endl;
         std::cout << u8"  4. 管理历史餐单               " << std::endl;
         std::cout << u8"  5. 口味偏好管理               " << std::endl;
         std::cout << u8"  6. 营养统计                   " << std::endl;
         std::cout << u8"  0. 退出登录                   " << std::endl;
         std::cout << u8"=================================" << std::endl;

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
        
        if (choice >= 1 && choice <= static_cast<int>(categoryList.size())) {
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

         std::cout << u8"\n=== 推荐餐单详情 ===" << std::endl;
         for (const auto& meal : recommendation) {
             meal.displayMeal();
         }

         engine.displayRecommendationStats(recommendation);

         std::cout << u8"\n是否保存此推荐？ (y/n): ";
         std::string saveChoice;
         std::cin >> saveChoice;

         if (saveChoice == "y" || saveChoice == "Y") {
             for (auto& meal : recommendation) {
                 meal.setId(db.getNextMealId());
                 meal.setUserId(getCurrentUser().getId());
                 db.saveMeal(meal);
                 engine.addToHistory(getCurrentUser().getId(), meal);
             }
             std::cout << u8"推荐餐单已保存！" << std::endl;
         }
     }

    void manageMealHistory() {
         auto meals = db.getMealsByUser(getCurrentUser().getId());

         if (meals.empty()) {
             std::cout << u8"\n暂无历史餐单记录。" << std::endl;
             return;
         }

         while (true) {
             std::cout << u8"\n=== 历史餐单管理 ===" << std::endl;
             std::cout << u8"1. 查看所有历史餐单" << std::endl;
             std::cout << u8"2. 查看特定日期的餐单" << std::endl;
             std::cout << u8"3. 删除餐单" << std::endl;
             std::cout << u8"4. 编辑餐单" << std::endl;
             std::cout << u8"5. 对比营养目标" << std::endl;
             std::cout << u8"0. 返回主菜单" << std::endl;

             int choice = Utils::getIntInput(u8"选择 (0-5): ");

             switch (choice) {
                 case 1:
                     viewAllMealHistory();
                     break;
                 case 2:
                     viewMealsByDate();
                     break;
                 case 3:
                     deleteMealFromHistory();
                     break;
                 case 4:
                     editMealFromHistory();
                     break;
                 case 5:
                     compareWithGoals();
                     break;
                 case 0:
                     return;
                 default:
                     std::cout << u8"无效选择！" << std::endl;
             }
         }
     }

     void viewAllMealHistory() {
         auto meals = db.getMealsByUser(getCurrentUser().getId());

         std::cout << u8"\n=== 所有历史餐单 ===" << std::endl;

         std::map<std::string, std::vector<Meal>> mealsByDate;
         for (const auto& meal : meals) {
             mealsByDate[meal.getDate()].push_back(meal);
         }

         std::cout << u8"找到 " << meals.size() << u8" 个餐单记录" << std::endl;

         for (const auto& dateGroup : mealsByDate) {
             std::cout << u8"\n=== 日期: " << dateGroup.first << u8" ===" << std::endl;
             
             double dayCalories = 0, dayProtein = 0, dayCarbs = 0, dayFat = 0;
             for (const auto& meal : dateGroup.second) {
                 meal.displayMeal();
                 dayCalories += meal.getTotalCalories();
                 dayProtein += meal.getTotalProtein();
                 dayCarbs += meal.getTotalCarbs();
                 dayFat += meal.getTotalFat();
             }
             
             std::cout << u8"\n当日总计: " 
                       << std::fixed << std::setprecision(1)
                       << dayCalories << u8" kcal, "
                       << dayProtein << u8" g 蛋白质, "
                       << dayCarbs << u8" g 碳水, "
                       << dayFat << u8" g 脂肪" << std::endl;
             
             std::cout << u8"目标完成度: "
                       << (dayCalories / getCurrentUser().getDailyCalorieGoal() * 100) << u8"% 热量, "
                       << (dayProtein / getCurrentUser().getDailyProteinGoal() * 100) << u8"% 蛋白质"
                       << std::endl;
         }
     }

     void viewMealsByDate() {
         std::string date = Utils::getStringInput(u8"请输入日期 (格式: YYYY-MM-DD): ");
         auto meals = db.getMealsByDate(date);

         if (meals.empty()) {
             std::cout << u8"\n该日期没有餐单记录。" << std::endl;
             return;
         }

         std::cout << u8"\n=== " << date << u8" 的餐单 ===" << std::endl;
         
         double dayCalories = 0, dayProtein = 0, dayCarbs = 0, dayFat = 0;
         for (const auto& meal : meals) {
             if (meal.getUserId() == getCurrentUser().getId()) {
                 meal.displayMeal();
                 dayCalories += meal.getTotalCalories();
                 dayProtein += meal.getTotalProtein();
                 dayCarbs += meal.getTotalCarbs();
                 dayFat += meal.getTotalFat();
             }
         }
         
         std::cout << u8"\n=== 营养汇总 ===" << std::endl;
         std::cout << u8"总热量: " << std::fixed << std::setprecision(1) 
                   << dayCalories << u8" / " << getCurrentUser().getDailyCalorieGoal() 
                   << u8" kcal (" << (dayCalories / getCurrentUser().getDailyCalorieGoal() * 100) 
                   << u8"%)" << std::endl;
         std::cout << u8"蛋白质: " << dayProtein << u8" / " << getCurrentUser().getDailyProteinGoal() 
                   << u8" g (" << (dayProtein / getCurrentUser().getDailyProteinGoal() * 100) 
                   << u8"%)" << std::endl;
         std::cout << u8"碳水化合物: " << dayCarbs << u8" / " << getCurrentUser().getDailyCarbGoal() 
                   << u8" g (" << (dayCarbs / getCurrentUser().getDailyCarbGoal() * 100) 
                   << u8"%)" << std::endl;
         std::cout << u8"脂肪: " << dayFat << u8" / " << getCurrentUser().getDailyFatGoal() 
                   << u8" g (" << (dayFat / getCurrentUser().getDailyFatGoal() * 100) 
                   << u8"%)" << std::endl;
     }

     void deleteMealFromHistory() {
         auto meals = db.getMealsByUser(getCurrentUser().getId());
         
         if (meals.empty()) {
             std::cout << u8"\n暂无餐单可删除。" << std::endl;
             return;
         }

         std::cout << u8"\n=== 删除餐单 ===" << std::endl;
         
         for (size_t i = 0; i < meals.size(); ++i) {
             std::cout << (i + 1) << ". ";
             std::cout << u8"[" << meals[i].getDate() << u8"] " 
                       << meals[i].getMealType() << u8" - " 
                       << meals[i].getTotalCalories() << u8" kcal" << std::endl;
         }

         int choice = Utils::getIntInput(u8"\n选择要删除的餐单 (0取消): ");
         
         if (choice > 0 && choice <= static_cast<int>(meals.size())) {
             std::cout << u8"确认删除该餐单？ (y/n): ";
             std::string confirm;
             std::cin >> confirm;
             
             if (confirm == "y" || confirm == "Y") {
                 if (db.deleteMeal(meals[choice - 1].getId())) {
                     std::cout << u8"餐单已删除！" << std::endl;
                 } else {
                     std::cout << u8"删除失败！" << std::endl;
                 }
             }
         }
     }

     void editMealFromHistory() {
         auto meals = db.getMealsByUser(getCurrentUser().getId());
         
         if (meals.empty()) {
             std::cout << u8"\n暂无餐单可编辑。" << std::endl;
             return;
         }

         std::cout << u8"\n=== 编辑餐单 ===" << std::endl;
         
         for (size_t i = 0; i < meals.size(); ++i) {
             std::cout << (i + 1) << ". ";
             std::cout << u8"[" << meals[i].getDate() << u8"] " 
                       << meals[i].getMealType() << std::endl;
         }

         int choice = Utils::getIntInput(u8"\n选择要编辑的餐单 (0取消): ");
         
         if (choice > 0 && choice <= static_cast<int>(meals.size())) {
             Meal selectedMeal = meals[choice - 1];
             selectedMeal.displayMeal();
             
             std::cout << u8"\n编辑选项:" << std::endl;
             std::cout << u8"1. 添加食物" << std::endl;
             std::cout << u8"2. 移除食物" << std::endl;
             std::cout << u8"3. 替换食物" << std::endl;
             std::cout << u8"0. 取消" << std::endl;
             
             int editChoice = Utils::getIntInput(u8"选择 (0-3): ");
             
             switch (editChoice) {
                 case 1: {
                     auto allFoods = db.getAllFoods();
                     std::cout << u8"\n可用食物列表:" << std::endl;
                     for (size_t i = 0; i < allFoods.size(); ++i) {
                         std::cout << (i + 1) << ". " << allFoods[i].getName() 
                                   << u8" - " << allFoods[i].getCalories() << u8" kcal" << std::endl;
                     }
                     
                     int foodChoice = Utils::getIntInput(u8"选择要添加的食物 (0取消): ");
                     if (foodChoice > 0 && foodChoice <= static_cast<int>(allFoods.size())) {
                         selectedMeal.addFood(allFoods[foodChoice - 1]);
                         if (db.updateMeal(selectedMeal)) {
                             std::cout << u8"食物已添加！" << std::endl;
                         }
                     }
                     break;
                 }
                 case 2: {
                     auto foods = selectedMeal.getFoods();
                     if (foods.empty()) {
                         std::cout << u8"餐单中没有食物。" << std::endl;
                         break;
                     }
                     
                     std::cout << u8"\n餐单中的食物:" << std::endl;
                     for (size_t i = 0; i < foods.size(); ++i) {
                         std::cout << (i + 1) << ". " << foods[i].getName() << std::endl;
                     }
                     
                     int foodChoice = Utils::getIntInput(u8"选择要移除的食物 (0取消): ");
                     if (foodChoice > 0 && foodChoice <= static_cast<int>(foods.size())) {
                         selectedMeal.removeFood(foods[foodChoice - 1].getId());
                         if (db.updateMeal(selectedMeal)) {
                             std::cout << u8"食物已移除！" << std::endl;
                         }
                     }
                     break;
                 }
                 case 3: {
                     auto foods = selectedMeal.getFoods();
                     if (foods.empty()) {
                         std::cout << u8"餐单中没有食物。" << std::endl;
                         break;
                     }
                     
                     std::cout << u8"\n餐单中的食物:" << std::endl;
                     for (size_t i = 0; i < foods.size(); ++i) {
                         std::cout << (i + 1) << ". " << foods[i].getName() << std::endl;
                     }
                     
                     int oldFoodChoice = Utils::getIntInput(u8"选择要替换的食物 (0取消): ");
                     if (oldFoodChoice > 0 && oldFoodChoice <= static_cast<int>(foods.size())) {
                         auto alternatives = engine.getAlternativeFoods(foods[oldFoodChoice - 1], getCurrentUser(), 5);
                         
                         if (alternatives.empty()) {
                             std::cout << u8"没有找到合适的替代食物。" << std::endl;
                             break;
                         }
                         
                         std::cout << u8"\n推荐的替代食物:" << std::endl;
                         for (size_t i = 0; i < alternatives.size(); ++i) {
                             std::cout << (i + 1) << ". " << alternatives[i].getName() 
                                       << u8" - " << alternatives[i].getCalories() << u8" kcal" << std::endl;
                         }
                         
                         int newFoodChoice = Utils::getIntInput(u8"选择新食物 (0取消): ");
                         if (newFoodChoice > 0 && newFoodChoice <= static_cast<int>(alternatives.size())) {
                             selectedMeal.removeFood(foods[oldFoodChoice - 1].getId());
                             selectedMeal.addFood(alternatives[newFoodChoice - 1]);
                             if (db.updateMeal(selectedMeal)) {
                                 std::cout << u8"食物已替换！" << std::endl;
                             }
                         }
                     }
                     break;
                 }
             }
         }
     }

     void compareWithGoals() {
         std::string date = Utils::getStringInput(u8"请输入日期 (格式: YYYY-MM-DD, 留空查看所有): ");
         
         auto meals = db.getMealsByUser(getCurrentUser().getId());
         
         if (date.empty()) {
             std::cout << u8"\n=== 营养目标对比分析 (全部历史) ===" << std::endl;
             
             double totalCal = 0, totalProt = 0, totalCarb = 0, totalFat = 0;
             int dayCount = 0;
             std::set<std::string> uniqueDates;
             
             for (const auto& meal : meals) {
                 uniqueDates.insert(meal.getDate());
                 totalCal += meal.getTotalCalories();
                 totalProt += meal.getTotalProtein();
                 totalCarb += meal.getTotalCarbs();
                 totalFat += meal.getTotalFat();
             }
             
             dayCount = uniqueDates.size();
             if (dayCount == 0) {
                 std::cout << u8"暂无数据。" << std::endl;
                 return;
             }
             
             double avgCal = totalCal / dayCount;
             double avgProt = totalProt / dayCount;
             double avgCarb = totalCarb / dayCount;
             double avgFat = totalFat / dayCount;
             
             std::cout << u8"\n记录天数: " << dayCount << std::endl;
             std::cout << u8"\n平均每日摄入 vs 目标:" << std::endl;
             displayGoalComparison(u8"热量", avgCal, getCurrentUser().getDailyCalorieGoal(), u8"kcal");
             displayGoalComparison(u8"蛋白质", avgProt, getCurrentUser().getDailyProteinGoal(), u8"g");
             displayGoalComparison(u8"碳水化合物", avgCarb, getCurrentUser().getDailyCarbGoal(), u8"g");
             displayGoalComparison(u8"脂肪", avgFat, getCurrentUser().getDailyFatGoal(), u8"g");
         } else {
             std::cout << u8"\n=== 营养目标对比分析 (" << date << u8") ===" << std::endl;
             
             double totalCal = 0, totalProt = 0, totalCarb = 0, totalFat = 0;
             bool found = false;
             
             for (const auto& meal : meals) {
                 if (meal.getDate() == date) {
                     found = true;
                     totalCal += meal.getTotalCalories();
                     totalProt += meal.getTotalProtein();
                     totalCarb += meal.getTotalCarbs();
                     totalFat += meal.getTotalFat();
                 }
             }
             
             if (!found) {
                 std::cout << u8"该日期没有餐单记录。" << std::endl;
                 return;
             }
             
             std::cout << u8"\n当日摄入 vs 目标:" << std::endl;
             displayGoalComparison(u8"热量", totalCal, getCurrentUser().getDailyCalorieGoal(), u8"kcal");
             displayGoalComparison(u8"蛋白质", totalProt, getCurrentUser().getDailyProteinGoal(), u8"g");
             displayGoalComparison(u8"碳水化合物", totalCarb, getCurrentUser().getDailyCarbGoal(), u8"g");
             displayGoalComparison(u8"脂肪", totalFat, getCurrentUser().getDailyFatGoal(), u8"g");
         }
     }

     void displayGoalComparison(const std::string& nutrient, double actual, double goal, const std::string& unit) {
         double percentage = (actual / goal) * 100.0;
         std::cout << std::fixed << std::setprecision(1);
         std::cout << u8"  " << nutrient << u8": " 
                   << actual << u8" / " << goal << u8" " << unit
                   << u8" (" << percentage << u8"%)";
         
         if (percentage < 90) {
             std::cout << u8" [未达标]";
         } else if (percentage > 110) {
             std::cout << u8" [超标]";
         } else {
             std::cout << u8" [良好]";
         }
         std::cout << std::endl;
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
                 getCurrentUser().removePreferredTag(tag);
                 break;
             }
             case 5: {
                 std::string tag = Utils::getStringInput(u8"请输入要移除的避免标签: ");
                 getCurrentUser().removeAvoidedTag(tag);
                 break;
             }
             case 6: {
                 std::string allergen = Utils::getStringInput(u8"请输入要移除的过敏源: ");
                 getCurrentUser().removeAllergen(allergen);
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