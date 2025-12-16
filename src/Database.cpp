#include "../include/Database.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <optional>

Database::Database() : usersFile("data/users.txt"), 
                       foodsFile("data/foods.txt"),
                       mealsFile("data/meals.txt") {}

Database::Database(const std::string& usersFile, const std::string& foodsFile, const std::string& mealsFile)
    : usersFile(usersFile), foodsFile(foodsFile), mealsFile(mealsFile) {}

std::vector<std::string> Database::split(const std::string& str, char delimiter) const {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::set<std::string> Database::parseTagString(const std::string& tagStr) const {
    std::set<std::string> tags;
    if (tagStr.empty()) return tags;
    
    auto tokens = split(tagStr, ',');
    for (const auto& token : tokens) {
        if (!token.empty()) {
            tags.insert(token);
        }
    }
    return tags;
}

bool Database::loadFoods() {
    std::ifstream file(foodsFile);
    if (!file.is_open()) {
        return false;
    }
    
    foods.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        auto tokens = split(line, '|');
        if (tokens.size() >= 9) {
            try {
                int id = std::stoi(tokens[0]);
                std::string name = tokens[1];
                double calories = std::stod(tokens[2]);
                double protein = std::stod(tokens[3]);
                double carbs = std::stod(tokens[4]);
                double fat = std::stod(tokens[5]);
                double fiber = std::stod(tokens[6]);
                std::string tagsStr = tokens[7];
                std::string category = tokens[8];
                
                auto tags = parseTagString(tagsStr);
                
                Food food(id, name, calories, protein, carbs, fat, fiber, tags, category);
                foods.push_back(food);
            } catch (const std::exception& e) {
                std::cout << "Error parsing food line: " << line << std::endl;
            }
        }
    }
    
    file.close();
    return !foods.empty();
}

bool Database::saveFoods() {
    std::ofstream file(foodsFile);
    if (!file.is_open()) {
        return false;
    }
    
    for (const auto& food : foods) {
        file << food.toString() << std::endl;
    }
    
    file.close();
    return true;
}

bool Database::loadUsers() {
    std::ifstream file(usersFile);
    if (!file.is_open()) {
        return false;
    }
    
    users.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        auto tokens = split(line, '|');
        if (tokens.size() >= 12) {
            try {
                int id = std::stoi(tokens[0]);
                std::string username = tokens[1];
                std::string password = tokens[2];
                int age = std::stoi(tokens[3]);
                double weight = std::stod(tokens[4]);
                double height = std::stod(tokens[5]);
                std::string gender = tokens[6];
                std::string activityLevel = tokens[7];
                double dailyCalorieGoal = std::stod(tokens[8]);
                double dailyProteinGoal = std::stod(tokens[9]);
                double dailyCarbGoal = std::stod(tokens[10]);
                double dailyFatGoal = std::stod(tokens[11]);
                
                auto preferredTags = parseTagString(tokens[12]);
                auto avoidedTags = parseTagString(tokens[13]);
                auto allergens = parseTagString(tokens[14]);
                
                User user(id, username, password);
                user.setAge(age);
                user.setWeight(weight);
                user.setHeight(height);
                user.setGender(gender);
                user.setActivityLevel(activityLevel);
                user.setDailyCalorieGoal(dailyCalorieGoal);
                user.setDailyProteinGoal(dailyProteinGoal);
                user.setDailyCarbGoal(dailyCarbGoal);
                user.setDailyFatGoal(dailyFatGoal);
                
                for (const auto& tag : preferredTags) {
                    user.addPreferredTag(tag);
                }
                for (const auto& tag : avoidedTags) {
                    user.addAvoidedTag(tag);
                }
                for (const auto& allergen : allergens) {
                    user.addAllergen(allergen);
                }
                
                users.push_back(user);
            } catch (const std::exception& e) {
                std::cout << "Error parsing user line: " << line << std::endl;
            }
        }
    }
    
    file.close();
    return true;
}

bool Database::saveUser(const User& user) {
    // 检查用户是否已存在
    for (size_t i = 0; i < users.size(); ++i) {
        if (users[i].getId() == user.getId()) {
            users[i] = user;
            return saveUsers();
        }
    }
    
    // 添加新用户
    users.push_back(user);
    return saveUsers();
}

bool Database::updateUser(const User& user) {
    return saveUser(user);
}

bool Database::saveUsers() {
    std::ofstream file(usersFile);
    if (!file.is_open()) {
        return false;
    }
    
    for (const auto& user : users) {
        file << user.toString() << std::endl;
    }
    
    file.close();
    return true;
}

bool Database::loadMeals() {
    std::ifstream file(mealsFile);
    if (!file.is_open()) {
        return false;
    }
    
    meals.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        auto tokens = split(line, '|');
        if (tokens.size() >= 10) {
            try {
                int id = std::stoi(tokens[0]);
                int userId = std::stoi(tokens[1]);
                std::string date = tokens[2];
                std::string mealType = tokens[3];
                double totalCalories = std::stod(tokens[4]);
                double totalProtein = std::stod(tokens[5]);
                double totalCarbs = std::stod(tokens[6]);
                double totalFat = std::stod(tokens[7]);
                bool isRecommended = (tokens[8] == "1");
                
                Meal meal(id, userId, date, mealType);
                meal.setIsRecommended(isRecommended);
                
                // 解析食物ID列表
                if (tokens.size() > 9) {
                    auto foodIds = split(tokens[9], ',');
                    for (const auto& foodIdStr : foodIds) {
                        if (!foodIdStr.empty()) {
                            int foodId = std::stoi(foodIdStr);
                            auto food = getFoodById(foodId);
                            if (food) {
                                meal.addFood(*food);
                            }
                        }
                    }
                }
                
                meals.push_back(meal);
            } catch (const std::exception& e) {
                std::cout << "Error parsing meal line: " << line << std::endl;
            }
        }
    }
    
    file.close();
    return true;
}

bool Database::saveMeal(const Meal& meal) {
    // 检查餐单是否已存在
    for (size_t i = 0; i < meals.size(); ++i) {
        if (meals[i].getId() == meal.getId()) {
            meals[i] = meal;
            return saveMeals();
        }
    }
    
    // 添加新餐单
    meals.push_back(meal);
    return saveMeals();
}

bool Database::saveMeals() {
    std::ofstream file(mealsFile);
    if (!file.is_open()) {
        return false;
    }
    
    for (const auto& meal : meals) {
        file << meal.toString() << std::endl;
    }
    
    file.close();
    return true;
}

std::vector<Food> Database::getAllFoods() const {
    return foods;
}

std::vector<User> Database::getAllUsers() const {
    return users;
}

std::vector<Meal> Database::getAllMeals() const {
    return meals;
}

std::vector<Meal> Database::getMealsByUser(int userId) const {
    std::vector<Meal> result;
    for (const auto& meal : meals) {
        if (meal.getUserId() == userId) {
            result.push_back(meal);
        }
    }
    return result;
}

std::vector<Meal> Database::getMealsByDate(const std::string& date) const {
    std::vector<Meal> result;
    for (const auto& meal : meals) {
        if (meal.getDate() == date) {
            result.push_back(meal);
        }
    }
    return result;
}

std::optional<Food> Database::getFoodById(int id) const {
    for (const auto& food : foods) {
        if (food.getId() == id) {
            return food;
        }
    }
    return std::nullopt;
}

std::optional<User> Database::getUserById(int id) const {
    for (const auto& user : users) {
        if (user.getId() == id) {
            return user;
        }
    }
    return std::nullopt;
}

int Database::getNextFoodId() const {
    int maxId = 0;
    for (const auto& food : foods) {
        if (food.getId() > maxId) {
            maxId = food.getId();
        }
    }
    return maxId + 1;
}

int Database::getNextUserId() const {
    int maxId = 0;
    for (const auto& user : users) {
        if (user.getId() > maxId) {
            maxId = user.getId();
        }
    }
    return maxId + 1;
}

int Database::getNextMealId() const {
    int maxId = 0;
    for (const auto& meal : meals) {
        if (meal.getId() > maxId) {
            maxId = meal.getId();
        }
    }
    return maxId + 1;
}

void Database::initializeSampleData() {
    foods.clear();
    
    foods.push_back(Food(1, "白米饭", 116, 2.6, 25.6, 0.3, 0.3, {"清淡"}, "主食"));
    foods.push_back(Food(2, "全麦面包", 246, 8.5, 45.3, 3.5, 6.8, {"清淡"}, "主食"));
    foods.push_back(Food(3, "燕麦粥", 68, 2.4, 12.0, 1.4, 1.7, {"清淡"}, "主食"));
    foods.push_back(Food(4, "红薯", 86, 1.6, 20.1, 0.2, 3.0, {"甜"}, "主食"));
    
    foods.push_back(Food(5, "鸡胸肉", 133, 24.6, 2.5, 5.0, 0.0, {"清淡"}, "肉类"));
    foods.push_back(Food(6, "三文鱼", 206, 22.5, 0.0, 13.4, 0.0, {"鲜"}, "肉类"));
    foods.push_back(Food(7, "牛肉", 250, 26.3, 0.0, 15.8, 0.0, {"鲜"}, "肉类"));
    foods.push_back(Food(8, "猪里脊", 143, 20.3, 1.3, 7.9, 0.0, {"鲜"}, "肉类"));
    
    foods.push_back(Food(9, "西兰花", 34, 4.1, 6.6, 0.4, 3.7, {"清淡"}, "蔬菜"));
    foods.push_back(Food(10, "菠菜", 23, 2.9, 3.6, 0.3, 2.2, {"清淡"}, "蔬菜"));
    foods.push_back(Food(11, "胡萝卜", 41, 0.9, 9.6, 0.2, 2.8, {"甜"}, "蔬菜"));
    foods.push_back(Food(12, "番茄", 18, 0.9, 3.9, 0.2, 1.2, {"酸"}, "蔬菜"));
    
    foods.push_back(Food(13, "豆腐", 76, 8.1, 4.2, 3.7, 0.4, {"清淡"}, "豆制品"));
    foods.push_back(Food(14, "豆浆", 31, 3.0, 1.1, 1.6, 1.1, {"清淡"}, "豆制品"));
    
    foods.push_back(Food(15, "鸡蛋", 147, 12.6, 1.3, 10.6, 0.0, {"清淡"}, "蛋类"));
    foods.push_back(Food(16, "鸭蛋", 180, 12.6, 3.1, 13.0, 0.0, {"咸"}, "蛋类"));
    
    foods.push_back(Food(17, "苹果", 52, 0.3, 13.8, 0.2, 2.4, {"甜", "酸"}, "水果"));
    foods.push_back(Food(18, "香蕉", 89, 1.1, 22.8, 0.3, 2.6, {"甜"}, "水果"));
    foods.push_back(Food(19, "橙子", 47, 0.9, 11.8, 0.1, 2.4, {"酸", "甜"}, "水果"));
    foods.push_back(Food(20, "葡萄", 62, 0.6, 15.9, 0.2, 0.9, {"甜"}, "水果"));
    
    foods.push_back(Food(21, "牛奶", 42, 3.4, 5.0, 1.0, 0.0, {"清淡"}, "奶制品"));
    foods.push_back(Food(22, "酸奶", 59, 10.0, 3.6, 0.4, 0.0, {"酸", "甜"}, "奶制品"));
    foods.push_back(Food(23, "奶酪", 113, 7.0, 1.0, 9.0, 0.0, {"清淡"}, "奶制品"));
    
    foods.push_back(Food(24, "核桃", 654, 15.2, 13.7, 65.2, 6.7, {"香"}, "坚果"));
    foods.push_back(Food(25, "花生", 567, 25.8, 16.1, 49.2, 8.5, {"香"}, "坚果"));
    
    saveFoods();
}