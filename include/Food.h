#ifndef FOOD_H
#define FOOD_H

#include <string>
#include <vector>
#include <set>

class Food {
private:
    int id;
    std::string name;
    double calories;          // 卡路里(kcal)
    double protein;           // 蛋白质(g)
    double carbohydrates;     // 碳水化合物(g)
    double fat;               // 脂肪(g)
    double fiber;             // 纤维素(g)
    std::set<std::string> tags; // 口味标签：辣、甜、咸、酸等
    std::string category;     // 类别：主食、蔬菜、肉类、水果等

public:
    Food();
    Food(int id, const std::string& name, double cal, double prot, 
         double carb, double fat, double fiber, 
         const std::set<std::string>& tags, const std::string& category);

    // Getters
    int getId() const { return id; }
    std::string getName() const { return name; }
    double getCalories() const { return calories; }
    double getProtein() const { return protein; }
    double getCarbohydrates() const { return carbohydrates; }
    double getFat() const { return fat; }
    double getFiber() const { return fiber; }
    std::set<std::string> getTags() const { return tags; }
    std::string getCategory() const { return category; }

    // Setters
    void setId(int id) { this->id = id; }
    void setName(const std::string& name) { this->name = name; }
    void setCalories(double cal) { this->calories = cal; }
    void setProtein(double prot) { this->protein = prot; }
    void setCarbohydrates(double carb) { this->carbohydrates = carb; }
    void setFat(double fat) { this->fat = fat; }
    void setFiber(double fiber) { this->fiber = fiber; }
    void setTags(const std::set<std::string>& tags) { this->tags = tags; }
    void setCategory(const std::string& cat) { this->category = cat; }

    void addTag(const std::string& tag);
    bool hasTag(const std::string& tag) const;
    
    void displayInfo() const;
    std::string toString() const;
};

#endif
