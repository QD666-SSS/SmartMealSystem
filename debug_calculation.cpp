#include <iostream>
#include "../include/User.h"

using namespace std;

int main() {
    User user1(1, "test1", "password");
    user1.setAge(25);
    user1.setGender("male");
    user1.setWeight(70.0);
    user1.setHeight(175.0);
    user1.setActivityLevel("moderate");
    
    user1.calculateNutritionGoals();
    
    cout << "=== 实际计算结果 ===" << endl;
    cout << "BMR = 10 * " << user1.getWeight() << " + 6.25 * " << user1.getHeight() << " - 5 * " << user1.getAge() << " + 5" << endl;
    double manual_bmr = 10 * 70 + 6.25 * 175 - 5 * 25 + 5;
    cout << "手动计算BMR: " << manual_bmr << endl;
    cout << "活动倍数: 1.55" << endl;
    double manual_tdee = manual_bmr * 1.55;
    cout << "手动计算TDEE: " << manual_tdee << endl;
    
    cout << "\n=== 实际返回结果 ===" << endl;
    cout << "每日卡路里目标: " << user1.getDailyCalorieGoal() << endl;
    cout << "每日蛋白质目标: " << user1.getDailyProteinGoal() << endl;
    cout << "每日碳水目标: " << user1.getDailyCarbGoal() << endl;
    cout << "每日脂肪目标: " << user1.getDailyFatGoal() << endl;
    
    cout << "\n=== 按实际值反推 ===" << endl;
    double actual_cal = user1.getDailyCalorieGoal();
    double expected_protein = 70.0 * 1.2;
    double expected_carb = actual_cal * 0.5 / 4;
    double expected_fat = actual_cal * 0.25 / 9;
    
    cout << "基于实际卡路里的期望碳水: " << expected_carb << endl;
    cout << "基于实际卡路里的期望脂肪: " << expected_fat << endl;
    
    return 0;
}