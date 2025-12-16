#!/bin/bash

# 智能配餐推荐系统 - 智能编码环境设置脚本
# 自动检测并设置正确的UTF-8编码环境

echo "================================================"
echo "智能配餐推荐系统 - 智能编码环境设置"
echo "================================================"

# 检测可用的UTF-8 locale
echo "检测系统可用的UTF-8 locale..."
available_locales=$(locale -a 2>/dev/null | grep -i "utf8\|UTF-8" || echo "")

if [ -z "$available_locales" ]; then
    echo "❌ 警告: 未找到任何UTF-8 locale"
    echo "建议安装中文语言包："
    echo "  Ubuntu/Debian: sudo apt-get install language-pack-zh-hans language-pack-zh-hans-base"
    echo "  CentOS/RHEL: sudo yum install kde-l10n-Chinese"
    echo "  或者使用通用UTF-8: C.UTF-8"
    echo ""
    echo "使用C.UTF-8作为fallback..."
    export LANG=C.UTF-8
    export LC_ALL=C.UTF-8
else
    echo "✅ 找到以下UTF-8 locale:"
    echo "$available_locales"
    echo ""
    
    # 优先选择中文locale
    if echo "$available_locales" | grep -q "zh_CN.UTF-8"; then
        echo "🎯 使用中文UTF-8 locale..."
        export LANG=zh_CN.UTF-8
        export LC_ALL=zh_CN.UTF-8
    elif echo "$available_locales" | grep -q "C.UTF-8"; then
        echo "🎯 使用通用UTF-8 locale..."
        export LANG=C.UTF-8
        export LC_ALL=C.UTF-8
    else
        echo "🎯 使用第一个可用的UTF-8 locale..."
        first_locale=$(echo "$available_locales" | head -1 | tr -d '\r')
        export LANG=$first_locale
        export LC_ALL=$first_locale
    fi
fi

echo "当前环境设置: $LANG"

# 验证设置
echo ""
echo "验证环境设置..."
if locale 2>/dev/null | grep -q "LC_ALL=$LC_ALL"; then
    echo "✅ 环境变量设置成功"
else
    echo "⚠️  环境变量部分设置成功（某些locale可能未完全安装）"
fi

# 设置终端
export TERM=xterm-256color

# 创建全局配置文件
cat > /home/engine/project/.encoding_profile << EOF
# 智能配餐推荐系统 - 编码环境配置
export LANG=$LANG
export LC_ALL=$LC_ALL
export TERM=xterm-256color

# 验证编码
if locale 2>/dev/null | grep -q "LC_CTYPE"; then
    echo "编码环境: \$LANG ✅"
else
    echo "警告: 编码环境设置可能不完整 ⚠️"
fi
EOF

echo ""
echo "================================================"
echo "✅ 编码环境配置完成!"
echo "================================================"
echo ""
echo "启动程序的方法："
echo ""
echo "方法1 - 使用配置文件（推荐）:"
echo "  source /home/engine/project/.encoding_profile"
echo "  cd /home/engine/project/build && ./bin/MealRecommendationSystem"
echo ""
echo "方法2 - 直接运行:"
echo "  /home/engine/project/run_with_encoding.sh"
echo ""
echo "================================================"

# 创建一键运行脚本
cat > /home/engine/project/run_with_encoding.sh << 'SCRIPT_EOF'
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
SCRIPT_EOF

chmod +x /home/engine/project/run_with_encoding.sh
chmod +x /home/engine/project/.encoding_profile

echo ""
echo "🎉 所有配置完成！"
echo "现在可以运行: /home/engine/project/run_with_encoding.sh"