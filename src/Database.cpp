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
                std::cout << "Error parsing food line: " << line << " - " << e.what() << std::endl;
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
                
                std::set<std::string> preferredTags;
                std::set<std::string> avoidedTags;
                std::set<std::string> allergens;
                
                if (tokens.size() > 12) {
                    preferredTags = parseTagString(tokens[12]);
                }
                if (tokens.size() > 13) {
                    avoidedTags = parseTagString(tokens[13]);
                }
                if (tokens.size() > 14) {
                    allergens = parseTagString(tokens[14]);
                }
                
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
                std::cout << "Error parsing user line: " << line << " - " << e.what() << std::endl;
            }
        }
    }
    
    file.close();
    return true;
}

bool Database::saveUser(const User& user) {
    for (size_t i = 0; i < users.size(); ++i) {
        if (users[i].getId() == user.getId()) {
            users[i] = user;
            return saveUsers();
        }
    }
    
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
                (void)std::stod(tokens[4]);  // totalCalories - recalculated from foods
                (void)std::stod(tokens[5]);  // totalProtein - recalculated from foods
                (void)std::stod(tokens[6]);  // totalCarbs - recalculated from foods
                (void)std::stod(tokens[7]);  // totalFat - recalculated from foods
                bool isRecommended = (tokens[8] == "1");
                
                Meal meal(id, userId, date, mealType);
                meal.setIsRecommended(isRecommended);
                
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
                std::cout << "Error parsing meal line: " << line << " - " << e.what() << std::endl;
            }
        }
    }
    
    file.close();
    return true;
}

bool Database::saveMeal(const Meal& meal) {
    for (size_t i = 0; i < meals.size(); ++i) {
        if (meals[i].getId() == meal.getId()) {
            meals[i] = meal;
            return saveMeals();
        }
    }
    
    meals.push_back(meal);
    return saveMeals();
}

bool Database::updateMeal(const Meal& meal) {
    for (size_t i = 0; i < meals.size(); ++i) {
        if (meals[i].getId() == meal.getId()) {
            meals[i] = meal;
            return saveMeals();
        }
    }
    return false;
}

bool Database::deleteMeal(int mealId) {
    for (size_t i = 0; i < meals.size(); ++i) {
        if (meals[i].getId() == mealId) {
            meals.erase(meals.begin() + i);
            return saveMeals();
        }
    }
    return false;
}

bool Database::deleteMealsByUserAndDate(int userId, const std::string& date, bool recommendedOnly) {
    size_t oldSize = meals.size();

    meals.erase(
        std::remove_if(meals.begin(), meals.end(),
            [&](const Meal& meal) {
                if (meal.getUserId() != userId) return false;
                if (meal.getDate() != date) return false;
                if (recommendedOnly && !meal.getIsRecommended()) return false;
                return true;
            }),
        meals.end());

    if (meals.size() == oldSize) {
        return true;
    }

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

std::optional<Meal> Database::getMealById(int id) const {
    for (const auto& meal : meals) {
        if (meal.getId() == id) {
            return meal;
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
    
    foods.push_back(Food(1, u8"白米饭", 116, 2.6, 25.6, 0.3, 0.3, {u8"清淡"}, u8"主食"));
    foods.push_back(Food(2, u8"全麦面包", 246, 8.5, 45.3, 3.5, 6.8, {u8"清淡"}, u8"主食"));
    foods.push_back(Food(3, u8"燕麦粥", 68, 2.4, 12.0, 1.4, 1.7, {u8"清淡"}, u8"主食"));
    foods.push_back(Food(4, u8"红薯", 86, 1.6, 20.1, 0.2, 3.0, {u8"甜"}, u8"主食"));
    
    foods.push_back(Food(5, u8"鸡胸肉", 133, 24.6, 2.5, 5.0, 0.0, {u8"清淡"}, u8"肉类"));
    foods.push_back(Food(6, u8"三文鱼", 206, 22.5, 0.0, 13.4, 0.0, {u8"鲜"}, u8"肉类"));
    foods.push_back(Food(7, u8"牛肉", 250, 26.3, 0.0, 15.8, 0.0, {u8"鲜"}, u8"肉类"));
    foods.push_back(Food(8, u8"猪里脊", 143, 20.3, 1.3, 7.9, 0.0, {u8"鲜"}, u8"肉类"));
    
    foods.push_back(Food(9, u8"西兰花", 34, 4.1, 6.6, 0.4, 3.7, {u8"清淡"}, u8"蔬菜"));
    foods.push_back(Food(10, u8"菠菜", 23, 2.9, 3.6, 0.3, 2.2, {u8"清淡"}, u8"蔬菜"));
    foods.push_back(Food(11, u8"胡萝卜", 41, 0.9, 9.6, 0.2, 2.8, {u8"甜"}, u8"蔬菜"));
    foods.push_back(Food(12, u8"番茄", 18, 0.9, 3.9, 0.2, 1.2, {u8"酸"}, u8"蔬菜"));
    
    foods.push_back(Food(13, u8"豆腐", 76, 8.1, 4.2, 3.7, 0.4, {u8"清淡"}, u8"豆制品"));
    foods.push_back(Food(14, u8"豆浆", 31, 3.0, 1.1, 1.6, 1.1, {u8"清淡"}, u8"豆制品"));
    
    foods.push_back(Food(15, u8"鸡蛋", 147, 12.6, 1.3, 10.6, 0.0, {u8"清淡"}, u8"蛋类"));
    foods.push_back(Food(16, u8"鸭蛋", 180, 12.6, 3.1, 13.0, 0.0, {u8"咸"}, u8"蛋类"));
    
    foods.push_back(Food(17, u8"苹果", 52, 0.3, 13.8, 0.2, 2.4, {u8"甜", u8"酸"}, u8"水果"));
    foods.push_back(Food(18, u8"香蕉", 89, 1.1, 22.8, 0.3, 2.6, {u8"甜"}, u8"水果"));
    foods.push_back(Food(19, u8"橙子", 47, 0.9, 11.8, 0.1, 2.4, {u8"酸", u8"甜"}, u8"水果"));
    foods.push_back(Food(20, u8"葡萄", 62, 0.6, 15.9, 0.2, 0.9, {u8"甜"}, u8"水果"));
    
    foods.push_back(Food(21, u8"牛奶", 42, 3.4, 5.0, 1.0, 0.0, {u8"清淡"}, u8"奶制品"));
    foods.push_back(Food(22, u8"酸奶", 59, 10.0, 3.6, 0.4, 0.0, {u8"酸", u8"甜"}, u8"奶制品"));
    foods.push_back(Food(23, u8"奶酪", 113, 7.0, 1.0, 9.0, 0.0, {u8"清淡"}, u8"奶制品"));
    
    foods.push_back(Food(24, u8"核桃", 654, 15.2, 13.7, 65.2, 6.7, {u8"香"}, u8"坚果"));
    foods.push_back(Food(25, u8"花生", 567, 25.8, 16.1, 49.2, 8.5, {u8"香"}, u8"坚果"));
    
    foods.push_back(Food(26, u8"糙米饭", 111, 2.6, 23.5, 0.9, 1.8, {u8"清淡"}, u8"主食"));
    foods.push_back(Food(27, u8"玉米", 86, 3.3, 19.0, 1.2, 2.7, {u8"甜"}, u8"主食"));
    foods.push_back(Food(28, u8"紫薯", 82, 1.8, 18.7, 0.2, 3.1, {u8"甜"}, u8"主食"));
    foods.push_back(Food(29, u8"荞麦面", 324, 12.6, 66.5, 2.7, 6.5, {u8"清淡"}, u8"主食"));
    
    foods.push_back(Food(30, u8"虾仁", 93, 20.6, 0.6, 0.6, 0.0, {u8"鲜", u8"海鲜"}, u8"肉类"));
    foods.push_back(Food(31, u8"鱼片", 113, 22.8, 0.0, 2.5, 0.0, {u8"鲜", u8"海鲜"}, u8"肉类"));
    foods.push_back(Food(32, u8"瘦猪肉", 143, 20.3, 1.3, 6.2, 0.0, {u8"鲜"}, u8"肉类"));
    
    foods.push_back(Food(33, u8"芹菜", 16, 0.8, 3.0, 0.2, 1.4, {u8"清淡"}, u8"蔬菜"));
    foods.push_back(Food(34, u8"黄瓜", 15, 0.7, 3.0, 0.2, 0.5, {u8"清淡"}, u8"蔬菜"));
    foods.push_back(Food(35, u8"青椒", 22, 1.0, 4.9, 0.2, 1.7, {u8"清淡"}, u8"蔬菜"));
    foods.push_back(Food(36, u8"蘑菇", 22, 3.1, 3.3, 0.3, 1.0, {u8"鲜"}, u8"蔬菜"));
    foods.push_back(Food(37, u8"白菜", 17, 1.5, 3.2, 0.2, 0.8, {u8"清淡"}, u8"蔬菜"));
    foods.push_back(Food(38, u8"茄子", 21, 1.1, 4.9, 0.2, 1.3, {u8"清淡"}, u8"蔬菜"));
    
    foods.push_back(Food(39, u8"豆腐干", 140, 16.2, 4.8, 3.6, 1.9, {u8"咸"}, u8"豆制品"));
    foods.push_back(Food(40, u8"黑豆", 341, 35.0, 33.6, 15.9, 15.5, {u8"清淡"}, u8"豆制品"));
    
    foods.push_back(Food(41, u8"猕猴桃", 61, 1.1, 14.5, 0.5, 2.6, {u8"酸", u8"甜"}, u8"水果"));
    foods.push_back(Food(42, u8"草莓", 32, 0.7, 7.7, 0.3, 2.0, {u8"甜"}, u8"水果"));
    foods.push_back(Food(43, u8"蓝莓", 57, 0.7, 14.5, 0.3, 2.4, {u8"甜", u8"酸"}, u8"水果"));
    foods.push_back(Food(44, u8"西瓜", 30, 0.6, 7.6, 0.2, 0.4, {u8"甜"}, u8"水果"));
    
    foods.push_back(Food(45, u8"杏仁", 579, 21.2, 21.7, 49.9, 11.8, {u8"香"}, u8"坚果"));
    foods.push_back(Food(46, u8"腰果", 553, 18.2, 30.2, 43.9, 3.3, {u8"香"}, u8"坚果"));
    
    foods.push_back(Food(47, u8"希腊酸奶", 97, 10.2, 3.9, 4.8, 0.0, {u8"酸", u8"甜"}, u8"奶制品"));
    foods.push_back(Food(48, u8"低脂牛奶", 35, 3.6, 5.0, 0.1, 0.0, {u8"清淡"}, u8"奶制品"));
    
    foods.push_back(Food(49, u8"鹌鹑蛋", 158, 13.1, 0.6, 11.6, 0.0, {u8"清淡"}, u8"蛋类"));
    foods.push_back(Food(50, u8"松花蛋", 171, 13.7, 4.9, 10.7, 0.0, {u8"咸"}, u8"蛋类"));
    
    saveFoods();
}
