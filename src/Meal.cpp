#include "../include/Meal.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

Meal::Meal() : id(0), userId(0), date(""), mealType(""), 
               totalCalories(0), totalProtein(0), totalCarbs(0), 
               totalFat(0), isRecommended(false) {}

Meal::Meal(int id, int userId, const std::string& date, const std::string& mealType)
    : id(id), userId(userId), date(date), mealType(mealType),
      totalCalories(0), totalProtein(0), totalCarbs(0), 
      totalFat(0), isRecommended(false) {}

void Meal::addFood(const Food& food) {
    foods.push_back(food);
    calculateTotals();
}

void Meal::removeFood(int foodId) {
    foods.erase(
        std::remove_if(foods.begin(), foods.end(),
            [foodId](const Food& f) { return f.getId() == foodId; }),
        foods.end()
    );
    calculateTotals();
}

void Meal::calculateTotals() {
    totalCalories = 0;
    totalProtein = 0;
    totalCarbs = 0;
    totalFat = 0;
    
    for (const auto& food : foods) {
        totalCalories += food.getCalories();
        totalProtein += food.getProtein();
        totalCarbs += food.getCarbohydrates();
        totalFat += food.getFat();
    }
}

void Meal::displayMeal() const {
    std::string mealTypeCN;
    if (mealType == "breakfast") mealTypeCN = "早餐";
    else if (mealType == "lunch") mealTypeCN = "午餐";
    else if (mealType == "dinner") mealTypeCN = "晚餐";
    else if (mealType == "snack") mealTypeCN = "加餐";
    else mealTypeCN = mealType;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << mealTypeCN << " - " << date;
    if (isRecommended) std::cout << " [系统推荐]";
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    
    if (foods.empty()) {
        std::cout << "  (暂无食物)                       " << std::endl;
    } else {
        for (size_t i = 0; i < foods.size(); ++i) {
            std::cout << "  " << (i+1) << ". " << std::left << std::setw(25) 
                      << foods[i].getName() << std::endl;
            std::cout << "     热量: " << std::fixed << std::setprecision(1) 
                      << foods[i].getCalories() << " kcal, "
                      << "蛋白质: " << foods[i].getProtein() << "g" << std::endl;
        }
    }
    
    std::cout << "========================================" << std::endl;
    std::cout << "总计:                                 " << std::endl;
    std::cout << "  热量:     " << std::fixed << std::setprecision(1) 
              << totalCalories << " kcal" << std::endl;
    std::cout << "  蛋白质:   " << totalProtein << " g" << std::endl;
    std::cout << "  碳水化合物: " << totalCarbs << " g" << std::endl;
    std::cout << "  脂肪:     " << totalFat << " g" << std::endl;
    std::cout << "========================================" << std::endl;
}

std::string Meal::toString() const {
    std::stringstream ss;
    ss << id << "|" << userId << "|" << date << "|" << mealType << "|"
       << totalCalories << "|" << totalProtein << "|" << totalCarbs << "|" 
       << totalFat << "|" << (isRecommended ? "1" : "0") << "|";
    
    bool first = true;
    for (const auto& food : foods) {
        if (!first) ss << ",";
        ss << food.getId();
        first = false;
    }
    
    return ss.str();
}