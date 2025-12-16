#include "../include/Food.h"
#include <iostream>
#include <sstream>
#include <iomanip>

Food::Food() : id(0), name(""), calories(0), protein(0), 
               carbohydrates(0), fat(0), fiber(0), category("") {}

Food::Food(int id, const std::string& name, double cal, double prot, 
           double carb, double fat, double fiber, 
           const std::set<std::string>& tags, const std::string& category)
    : id(id), name(name), calories(cal), protein(prot), 
      carbohydrates(carb), fat(fat), fiber(fiber), 
      tags(tags), category(category) {}

void Food::addTag(const std::string& tag) {
    tags.insert(tag);
}

bool Food::hasTag(const std::string& tag) const {
    return tags.find(tag) != tags.end();
}

void Food::displayInfo() const {
     std::cout << "========================================" << std::endl;
     std::cout << u8"  食物: " << name << u8" (ID: " << id << u8")" << std::endl;
     std::cout << u8"  类别: " << category << std::endl;
     std::cout << "========================================" << std::endl;
     std::cout << u8"  营养成分 (每100g):" << std::endl;
     std::cout << u8"    热量:     " << std::fixed << std::setprecision(1)
               << calories << u8" kcal" << std::endl;
     std::cout << u8"    蛋白质:   " << protein << u8" g" << std::endl;
     std::cout << u8"    碳水化合物: " << carbohydrates << u8" g" << std::endl;
     std::cout << u8"    脂肪:     " << fat << u8" g" << std::endl;
     std::cout << u8"    纤维素:   " << fiber << u8" g" << std::endl;

     if (!tags.empty()) {
         std::cout << u8"  口味标签: ";
         bool first = true;
         for (const auto& tag : tags) {
             if (!first) std::cout << u8", ";
             std::cout << tag;
             first = false;
         }
         std::cout << std::endl;
     }
     std::cout << "========================================" << std::endl;
 }

std::string Food::toString() const {
    std::stringstream ss;
    ss << id << "|" << name << "|" << calories << "|" << protein << "|"
       << carbohydrates << "|" << fat << "|" << fiber << "|";
    
    bool first = true;
    for (const auto& tag : tags) {
        if (!first) ss << ",";
        ss << tag;
        first = false;
    }
    ss << "|" << category;
    
    return ss.str();
}