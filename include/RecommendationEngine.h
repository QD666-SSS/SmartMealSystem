#ifndef RECOMMENDATION_ENGINE_H
#define RECOMMENDATION_ENGINE_H

#include "User.h"
#include "Food.h"
#include "Meal.h"
#include <vector>
#include <map>

class RecommendationEngine {
private:
    std::vector<Food> foodDatabase;
    std::map<int, std::vector<Meal>> userHistory;  // userId -> meals

    double calculateFoodScore(const Food& food, const User& user, 
                              const std::string& mealType,
                              double remainingCalories,
                              double remainingProtein,
                              double remainingCarbs,
                              double remainingFat) const;
    
    std::vector<Food> filterFoodsByCategory(const std::string& category) const;
    bool isAllergenFree(const Food& food, const User& user) const;

public:
    RecommendationEngine();
    
    void setFoodDatabase(const std::vector<Food>& foods);
    void addToHistory(int userId, const Meal& meal);
    void loadHistory(const std::map<int, std::vector<Meal>>& history);
    
    std::vector<Meal> recommendDailyMeals(const User& user, const std::string& date);
    Meal recommendMeal(const User& user, const std::string& mealType, 
                       double targetCalories, double targetProtein,
                       double targetCarbs, double targetFat);
    
    std::vector<Food> getAlternativeFoods(const Food& food, const User& user, int count = 3);
    
    void displayRecommendationStats(const std::vector<Meal>& meals);
};

#endif
