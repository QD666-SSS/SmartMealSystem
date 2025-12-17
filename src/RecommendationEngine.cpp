#include "../include/RecommendationEngine.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <map>

RecommendationEngine::RecommendationEngine() {}

void RecommendationEngine::setFoodDatabase(const std::vector<Food>& foods) {
    foodDatabase = foods;
}

void RecommendationEngine::addToHistory(int userId, const Meal& meal) {
    userHistory[userId].push_back(meal);
}

void RecommendationEngine::loadHistory(const std::map<int, std::vector<Meal>>& history) {
    userHistory = history;
}

bool RecommendationEngine::isAllergenFree(const Food& food, const User& user) const {
    for (const auto& allergen : user.getAllergens()) {
        if (food.hasTag(allergen)) {
            return false;
        }
    }
    return true;
}

double RecommendationEngine::calculateFoodScore(const Food& food, const User& user,
                                                 const std::string& mealType,
                                                 double remainingCalories,
                                                 double remainingProtein,
                                                 double remainingCarbs,
                                                 double remainingFat) const {
    (void)mealType;
    
    double score = 100.0;
    
    if (!isAllergenFree(food, user)) {
        return -1000.0;
    }
    
    for (const auto& avoidedTag : user.getAvoidedTags()) {
        if (food.hasTag(avoidedTag)) {
            score -= 50.0;
        }
    }
    
    for (const auto& preferredTag : user.getPreferredTags()) {
        if (food.hasTag(preferredTag)) {
            score += 30.0;
        }
    }
    
    // 营养平衡评分
    double nutritionScore = 0.0;
    if (remainingCalories > 0) {
        double calorieRatio = food.getCalories() / remainingCalories;
        nutritionScore += 20.0 * (1.0 - std::abs(calorieRatio - 1.0));
    }
    
    if (remainingProtein > 0) {
        double proteinRatio = food.getProtein() / remainingProtein;
        nutritionScore += 25.0 * (1.0 - std::abs(proteinRatio - 1.0));
    }
    
    if (remainingCarbs > 0) {
        double carbRatio = food.getCarbohydrates() / remainingCarbs;
        nutritionScore += 15.0 * (1.0 - std::abs(carbRatio - 1.0));
    }
    
    if (remainingFat > 0) {
        double fatRatio = food.getFat() / remainingFat;
        nutritionScore += 10.0 * (1.0 - std::abs(fatRatio - 1.0));
    }
    
    score += nutritionScore;
    
    // 多样性评分 - 避免重复推荐相同食物
    auto history = userHistory.find(user.getId());
    if (history != userHistory.end()) {
        int recentCount = 0;
        int historySize = history->second.size();
        int recentLimit = std::min(10, historySize);
        
        for (int i = historySize - recentLimit; i < historySize; ++i) {
            if (i >= 0) {
                const auto& pastMeal = history->second[i];
                for (const auto& pastFood : pastMeal.getFoods()) {
                    if (pastFood.getId() == food.getId()) {
                        recentCount++;
                    }
                }
            }
        }
        score -= recentCount * 15.0;
        
        if (recentCount > 0 && recentLimit > 0) {
            double recencyWeight = static_cast<double>(recentCount) / recentLimit;
            score -= recencyWeight * 20.0;
        }
    }
    
    return score;
}

std::vector<Food> RecommendationEngine::filterFoodsByCategory(const std::string& category) const {
    std::vector<Food> filtered;
    for (const auto& food : foodDatabase) {
        if (food.getCategory() == category) {
            filtered.push_back(food);
        }
    }
    return filtered;
}

Meal RecommendationEngine::recommendMeal(const User& user, const std::string& mealType,
                                         double targetCalories, double targetProtein,
                                         double targetCarbs, double targetFat) {
    Meal meal(0, user.getId(), "", mealType);
    meal.setIsRecommended(true);
    
    double remainingCalories = targetCalories;
    double remainingProtein = targetProtein;
    double remainingCarbs = targetCarbs;
    double remainingFat = targetFat;
    
    std::vector<std::string> categories;
    std::map<std::string, double> categoryWeights;
    
    if (mealType == "breakfast") {
        categories = {u8"主食", u8"蛋类", u8"奶制品", u8"水果"};
        categoryWeights[u8"主食"] = 0.4;
        categoryWeights[u8"蛋类"] = 0.3;
        categoryWeights[u8"奶制品"] = 0.2;
        categoryWeights[u8"水果"] = 0.1;
    } else if (mealType == "lunch") {
        categories = {u8"主食", u8"肉类", u8"蔬菜"};
        categoryWeights[u8"主食"] = 0.35;
        categoryWeights[u8"肉类"] = 0.4;
        categoryWeights[u8"蔬菜"] = 0.25;
    } else if (mealType == "dinner") {
        categories = {u8"主食", u8"蔬菜", u8"豆制品", u8"肉类"};
        categoryWeights[u8"主食"] = 0.3;
        categoryWeights[u8"蔬菜"] = 0.3;
        categoryWeights[u8"豆制品"] = 0.25;
        categoryWeights[u8"肉类"] = 0.15;
    } else {
        categories = {u8"水果", u8"坚果"};
        categoryWeights[u8"水果"] = 0.7;
        categoryWeights[u8"坚果"] = 0.3;
    }
    
    for (const auto& category : categories) {
        std::vector<Food> categoryFoods = filterFoodsByCategory(category);
        if (categoryFoods.empty()) continue;
        
        double categoryCalTarget = targetCalories * categoryWeights[category];
        double categoryProtTarget = targetProtein * categoryWeights[category];
        double categoryCarbTarget = targetCarbs * categoryWeights[category];
        double categoryFatTarget = targetFat * categoryWeights[category];
        
        std::vector<std::pair<double, Food>> scoredFoods;
        for (const auto& food : categoryFoods) {
            double score = calculateFoodScore(food, user, mealType,
                                             categoryCalTarget,
                                             categoryProtTarget,
                                             categoryCarbTarget,
                                             categoryFatTarget);
            if (score > -500) {
                scoredFoods.push_back({score, food});
            }
        }
        
        if (!scoredFoods.empty()) {
            std::sort(scoredFoods.begin(), scoredFoods.end(),
                     [](const auto& a, const auto& b) { return a.first > b.first; });
            
            size_t topChoices = std::min(static_cast<size_t>(3), scoredFoods.size());
            size_t selectedIndex = 0;
            
            if (topChoices > 1) {
                for (size_t i = 1; i < topChoices; ++i) {
                    if (scoredFoods[i].first >= scoredFoods[0].first * 0.9) {
                        selectedIndex = i;
                        break;
                    }
                }
            }
            
            meal.addFood(scoredFoods[selectedIndex].second);
            remainingCalories -= scoredFoods[selectedIndex].second.getCalories();
            remainingProtein -= scoredFoods[selectedIndex].second.getProtein();
            remainingCarbs -= scoredFoods[selectedIndex].second.getCarbohydrates();
            remainingFat -= scoredFoods[selectedIndex].second.getFat();
        }
    }
    
    return meal;
}

std::vector<Meal> RecommendationEngine::recommendDailyMeals(const User& user, const std::string& date) {
    std::vector<Meal> dailyMeals;
    
    double breakfastCalories = user.getDailyCalorieGoal() * 0.3;
    double lunchCalories = user.getDailyCalorieGoal() * 0.4;
    double dinnerCalories = user.getDailyCalorieGoal() * 0.3;
    
    double breakfastProtein = user.getDailyProteinGoal() * 0.25;
    double lunchProtein = user.getDailyProteinGoal() * 0.4;
    double dinnerProtein = user.getDailyProteinGoal() * 0.35;
    
    double breakfastCarbs = user.getDailyCarbGoal() * 0.3;
    double lunchCarbs = user.getDailyCarbGoal() * 0.4;
    double dinnerCarbs = user.getDailyCarbGoal() * 0.3;
    
    double breakfastFat = user.getDailyFatGoal() * 0.3;
    double lunchFat = user.getDailyFatGoal() * 0.4;
    double dinnerFat = user.getDailyFatGoal() * 0.3;
    
    Meal breakfast = recommendMeal(user, "breakfast", breakfastCalories, 
                                   breakfastProtein, breakfastCarbs, breakfastFat);
    breakfast.setDate(date);
    dailyMeals.push_back(breakfast);
    
    Meal lunch = recommendMeal(user, "lunch", lunchCalories,
                              lunchProtein, lunchCarbs, lunchFat);
    lunch.setDate(date);
    dailyMeals.push_back(lunch);
    
    Meal dinner = recommendMeal(user, "dinner", dinnerCalories,
                               dinnerProtein, dinnerCarbs, dinnerFat);
    dinner.setDate(date);
    dailyMeals.push_back(dinner);
    
    return dailyMeals;
}

std::vector<Food> RecommendationEngine::getAlternativeFoods(const Food& food, const User& user, int count) {
    std::vector<std::pair<double, Food>> scoredFoods;
    
    for (const auto& candidate : foodDatabase) {
        if (candidate.getId() == food.getId()) continue;
        
        double score = calculateFoodScore(candidate, user, "alternative",
                                         food.getCalories(), food.getProtein(),
                                         food.getCarbohydrates(), food.getFat());
        if (score > -500) {
            scoredFoods.push_back({score, candidate});
        }
    }
    
    std::sort(scoredFoods.begin(), scoredFoods.end(),
             [](const auto& a, const auto& b) { return a.first > b.first; });
    
    std::vector<Food> alternatives;
    for (int i = 0; i < std::min(count, (int)scoredFoods.size()); ++i) {
        alternatives.push_back(scoredFoods[i].second);
    }
    
    return alternatives;
}

void RecommendationEngine::displayRecommendationStats(const std::vector<Meal>& meals) {
     std::cout << u8"\n=== 推荐统计 ===" << std::endl;

     double totalCalories = 0, totalProtein = 0, totalCarbs = 0, totalFat = 0;

     for (const auto& meal : meals) {
         totalCalories += meal.getTotalCalories();
         totalProtein += meal.getTotalProtein();
         totalCarbs += meal.getTotalCarbs();
         totalFat += meal.getTotalFat();
     }

     std::cout << u8"总营养摄入:" << std::endl;
     std::cout << u8"  热量: " << std::fixed << std::setprecision(1) << totalCalories << u8" kcal" << std::endl;
     std::cout << u8"  蛋白质: " << totalProtein << u8" g" << std::endl;
     std::cout << u8"  碳水化合物: " << totalCarbs << u8" g" << std::endl;
     std::cout << u8"  脂肪: " << totalFat << u8" g" << std::endl;

     for (const auto& meal : meals) {
         std::cout << u8"\n" << meal.getMealType() << u8" - " << meal.getTotalCalories()
                   << u8" kcal (" << std::fixed << std::setprecision(1)
                   << (meal.getTotalCalories() / totalCalories * 100) << u8"%)" << std::endl;
     }
 }