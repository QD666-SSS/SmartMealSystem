#!/bin/bash
# 设置编码环境变量
export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8

# 设置终端支持UTF-8
export TERM=xterm-256color

echo "编码环境已设置: $LANG"
echo "可以运行以下命令启动程序:"
echo "cd /home/engine/project/build && ./bin/MealRecommendationSystem"
