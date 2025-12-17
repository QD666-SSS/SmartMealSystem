@echo off
setlocal
chcp 65001 > nul

echo ==================================
echo 智能配餐推荐系统 - Windows发布打包脚本
echo ==================================

REM 检查编译结果
if not exist "build\bin\Release\MealRecommendationSystem.exe" (
    echo 错误：未找到编译好的程序。
    echo 正在尝试自动编译...
    call build.bat
    if errorlevel 1 (
        echo 自动编译失败，请检查错误信息。
        pause
        exit /b 1
    )
)

set DIST_ROOT=dist
set DIST_NAME=MealRecommendationSystem_v2.0
set DIST_DIR=%DIST_ROOT%\%DIST_NAME%

REM 清理旧发布
if exist "%DIST_DIR%" (
    echo 清理旧的发布目录...
    rmdir /s /q "%DIST_DIR%"
)

REM 创建目录结构
echo 创建发布目录...
if not exist "%DIST_ROOT%" mkdir "%DIST_ROOT%"
mkdir "%DIST_DIR%"
mkdir "%DIST_DIR%\data"

REM 复制核心文件
echo 复制程序文件...
copy "build\bin\Release\MealRecommendationSystem.exe" "%DIST_DIR%\" > nul

REM 复制文档
echo 复制文档...
copy "README.md" "%DIST_DIR%\" > nul
copy "USER_GUIDE.md" "%DIST_DIR%\" > nul
copy "LICENSE" "%DIST_DIR%\" > nul 2>&1

REM 创建启动脚本
echo 创建启动脚本...
(
echo @echo off
echo chcp 65001 ^> nul
echo title 智能配餐推荐系统
echo color 0A
echo mode con cols=100 lines=40
echo.
echo 正在启动系统...
echo.
echo MealRecommendationSystem.exe
echo.
echo 程序已退出。
echo pause
) > "%DIST_DIR%\启动系统.bat"

echo.
echo ==================================
echo 打包成功！
echo ==================================
echo 发布包位置: %CD%\%DIST_DIR%
echo.
echo 您可以将 "%DIST_NAME%" 文件夹打包压缩，
echo 发送给用户即可直接使用。
echo.
echo 提示: 首次运行时会自动生成示例数据。
echo ==================================
pause
