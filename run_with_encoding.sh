#!/bin/bash
# 一键运行智能配餐推荐系统（自动设置编码环境）

# 设置编码环境
if [ -f "/home/engine/project/.encoding_profile" ]; then
    source /home/engine/project/.encoding_profile
else
    # Fallback设置
    export LANG=C.UTF-8
    export LC_ALL=C.UTF-8
fi

echo "================================================"
echo "智能配餐推荐系统"
echo "================================================"

# 检查程序是否存在
if [ ! -f "/home/engine/project/build/bin/MealRecommendationSystem" ]; then
    echo "错误: 程序不存在，请先编译项目"
    echo "运行: cd /home/engine/project && mkdir build && cd build && cmake .. && make"
    exit 1
fi

# 运行程序
cd /home/engine/project/build
./bin/MealRecommendationSystem
