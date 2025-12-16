#ifndef MEAL_H
#define MEAL_H

#include "Food.h"
#include <vector>
#include <string>
#include <ctime>

class Meal {
private:
    int id;
    int userId;
    std::string date;
    std::string mealType;  // breakfast, lunch, dinner, snack
    std::vector<Food> foods;
    double totalCalories;
    double totalProtein;
    double totalCarbs;
    double totalFat;
    bool isRecommended;

public:
    Meal();
    Meal(int id, int userId, const std::string& date, const std::string& mealType);

    void addFood(const Food& food);
    void removeFood(int foodId);
    void calculateTotals();
    
    // Getters
    int getId() const { return id; }
    int getUserId() const { return userId; }
    std::string getDate() const { return date; }
    std::string getMealType() const { return mealType; }
    std::vector<Food> getFoods() const { return foods; }
    double getTotalCalories() const { return totalCalories; }
    double getTotalProtein() const { return totalProtein; }
    double getTotalCarbs() const { return totalCarbs; }
    double getTotalFat() const { return totalFat; }
    bool getIsRecommended() const { return isRecommended; }

    // Setters
    void setId(int id) { this->id = id; }
    void setUserId(int userId) { this->userId = userId; }
    void setDate(const std::string& date) { this->date = date; }
    void setMealType(const std::string& type) { this->mealType = type; }
    void setIsRecommended(bool recommended) { this->isRecommended = recommended; }

    void displayMeal() const;
    std::string toString() const;
};

#endif
