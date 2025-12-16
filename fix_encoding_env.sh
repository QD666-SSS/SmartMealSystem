#!/bin/bash

# 智能配餐推荐系统 - 环境编码修复脚本
# 用于解决中文乱码问题

echo "================================================"
echo "智能配餐推荐系统 - 编码环境修复脚本"
echo "================================================"

# 检查可用的locale
echo "检查系统可用的locale..."
available_locales=$(locale -a 2>/dev/null | grep -E "UTF-8|utf8")

if [ -z "$available_locales" ]; then
    echo "警告: 未找到UTF-8 locale，可能需要安装中文语言包"
else
    echo "找到以下UTF-8 locale:"
    echo "$available_locales"
fi

# 尝试设置中文UTF-8环境变量
if echo "$available_locales" | grep -q "zh_CN.UTF-8"; then
    echo "设置中文UTF-8环境..."
    export LANG=zh_CN.UTF-8
    export LC_ALL=zh_CN.UTF-8
    locale_set="zh_CN.UTF-8"
elif echo "$available_locales" | grep -q "C.UTF-8"; then
    echo "设置通用UTF-8环境..."
    export LANG=C.UTF-8
    export LC_ALL=C.UTF-8
    locale_set="C.UTF-8"
else
    echo "警告: 建议安装中文语言包"
    export LANG=en_US.UTF-8
    export LC_ALL=en_US.UTF-8
    locale_set="en_US.UTF-8"
fi

echo "当前环境设置: $locale_set"

# 创建永久环境变量设置文件
echo "创建环境变量配置文件..."
cat > /home/engine/project/set_encoding.sh << EOF
#!/bin/bash
# 设置编码环境变量
export LANG=$LANG
export LC_ALL=$LC_ALL

# 设置终端支持UTF-8
export TERM=xterm-256color

echo "编码环境已设置: \$LANG"
echo "可以运行以下命令启动程序:"
echo "cd /home/engine/project/build && ./bin/MealRecommendationSystem"
EOF

chmod +x /home/engine/project/set_encoding.sh

echo "================================================"
echo "修复完成!"
echo "================================================"
echo "运行方式:"
echo "1. 每次运行前执行: source /home/engine/project/set_encoding.sh"
echo "2. 或者直接运行: /home/engine/project/set_encoding.sh"
echo "================================================"