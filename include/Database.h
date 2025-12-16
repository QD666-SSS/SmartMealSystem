#ifndef DATABASE_H
#define DATABASE_H

#include "User.h"
#include "Food.h"
#include "Meal.h"
#include <vector>
#include <map>
#include <string>
#include <optional>
#include <set>

class Database {
private:
    std::vector<User> users;
    std::vector<Food> foods;
    std::vector<Meal> meals;
    
    std::string usersFile;
    std::string foodsFile;
    std::string mealsFile;

    std::vector<std::string> split(const std::string& str, char delimiter) const;
    std::set<std::string> parseTagString(const std::string& tagStr) const;

public:
    Database();
    Database(const std::string& usersFile, const std::string& foodsFile, const std::string& mealsFile);

    bool loadUsers();
    bool loadFoods();
    bool loadMeals();
    
    bool saveUsers();
    bool saveFoods();
    bool saveMeals();
    
    bool saveUser(const User& user);
    bool updateUser(const User& user);
    bool saveMeal(const Meal& meal);
    
    std::vector<User> getAllUsers() const;
    std::vector<Food> getAllFoods() const;
    std::vector<Meal> getAllMeals() const;
    
    std::vector<Meal> getMealsByUser(int userId) const;
    std::vector<Meal> getMealsByDate(const std::string& date) const;
    
    std::optional<Food> getFoodById(int id) const;
    std::optional<User> getUserById(int id) const;
    
    int getNextUserId() const;
    int getNextFoodId() const;
    int getNextMealId() const;

    void initializeSampleData();
};

#endif