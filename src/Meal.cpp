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
     if (mealType == "breakfast") mealTypeCN = u8"早餐";
     else if (mealType == "lunch") mealTypeCN = u8"午餐";
     else if (mealType == "dinner") mealTypeCN = u8"晚餐";
     else if (mealType == "snack") mealTypeCN = u8"加餐";
     else mealTypeCN = mealType;

     std::cout << "\n========================================" << std::endl;
     std::cout << mealTypeCN << u8" - " << date;
     if (isRecommended) std::cout << u8" [系统推荐]";
     std::cout << std::endl;
     std::cout << "========================================" << std::endl;

     if (foods.empty()) {
         std::cout << u8"  (暂无食物)                       " << std::endl;
     } else {
         for (size_t i = 0; i < foods.size(); ++i) {
             std::cout << u8"  " << (i+1) << u8". " << std::left << std::setw(25)
                       << foods[i].getName() << std::endl;
             std::cout << u8"     热量: " << std::fixed << std::setprecision(1)
                       << foods[i].getCalories() << u8" kcal, "
                       << u8"蛋白质: " << foods[i].getProtein() << u8"g" << std::endl;
         }
     }

     std::cout << "========================================" << std::endl;
     std::cout << u8"总计:                                 " << std::endl;
     std::cout << u8"  热量:     " << std::fixed << std::setprecision(1)
               << totalCalories << u8" kcal" << std::endl;
     std::cout << u8"  蛋白质:   " << totalProtein << u8" g" << std::endl;
     std::cout << u8"  碳水化合物: " << totalCarbs << u8" g" << std::endl;
     std::cout << u8"  脂肪:     " << totalFat << u8" g" << std::endl;
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