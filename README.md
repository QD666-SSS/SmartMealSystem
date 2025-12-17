# 智能配餐推荐系统 - Web GUI版

一个现代化的、基于Web界面的智能配餐推荐系统，可以在Windows本地运行。

## 特性

- 🌐 **现代化Web界面** - 精美的渐变背景、流畅的动画效果
- 🚀 **自动启动浏览器** - 运行程序后自动在浏览器中打开 http://localhost:8000
- 👤 **用户管理** - 注册、登录、个人信息管理
- ✨ **智能推荐** - 基于个人营养目标生成每日餐单
- 📊 **营养追踪** - 实时查看营养摄入与目标的对比
- 📜 **历史记录** - 查看和管理历史餐单
- 🥗 **食物库** - 浏览和搜索食物数据库
- 📱 **响应式设计** - 支持桌面和移动设备

## 系统要求

- Windows 10/11
- Visual Studio 2022
- C++17 或更高版本

## 编译和运行

1. 使用 Visual Studio 2022 打开 `MealRecommendationSystem.sln`
2. 选择配置（Debug 或 Release）和平台（x64 推荐）
3. 按 F5 或点击"本地Windows调试器"开始运行
4. 程序会自动在默认浏览器中打开 http://localhost:8000
5. 首次运行会自动初始化示例数据

## 使用说明

### 注册和登录

1. 首次使用需要注册账号
2. 填写基本信息：年龄、性别、体重、身高、活动水平
3. 系统会自动计算每日营养目标

### 生成推荐餐单

1. 在主界面点击"智能推荐"
2. 选择日期
3. 点击"生成推荐"
4. 查看推荐的早餐、午餐、晚餐
5. 点击"保存此推荐"将餐单保存到历史记录

### 查看历史记录

1. 点击"历史餐单"菜单
2. 按日期查看所有餐单记录
3. 查看每日营养总计和目标完成度

### 浏览食物库

1. 点击"食物库"菜单
2. 使用搜索框搜索食物
3. 按类别筛选食物
4. 查看每种食物的详细营养信息

## 技术栈

- **后端**: C++17, httplib (单头文件HTTP服务器库)
- **前端**: HTML5, CSS3, JavaScript (原生)
- **构建系统**: Visual Studio 2022 MSBuild
- **网络库**: Windows Sockets 2 (ws2_32.lib)

## 项目结构

```
MealRecommendationSystem/
├── src/                    # 源代码文件
│   ├── main.cpp           # 主程序入口
│   ├── WebServer.cpp      # Web服务器实现
│   ├── Database.cpp       # 数据库管理
│   ├── RecommendationEngine.cpp  # 推荐引擎
│   └── ...
├── include/               # 头文件
│   ├── WebServer.h
│   ├── Database.h
│   ├── RecommendationEngine.h
│   └── third_party/
│       └── httplib.h      # HTTP服务器库
├── www/                   # Web前端文件
│   ├── index.html         # 主页面
│   ├── style.css          # 样式表
│   └── app.js             # JavaScript应用逻辑
├── data/                  # 数据文件
│   ├── users.txt          # 用户数据
│   ├── foods.txt          # 食物数据
│   └── meals.txt          # 餐单数据
└── MealRecommendationSystem.sln  # VS解决方案
```

## API接口

系统提供以下RESTful API接口：

- `POST /api/login` - 用户登录
- `POST /api/register` - 用户注册
- `GET /api/user/profile` - 获取用户信息
- `PUT /api/user/profile` - 更新用户信息
- `GET /api/foods` - 获取食物列表
- `GET /api/meals/history` - 获取历史餐单
- `POST /api/meals/recommend` - 生成推荐餐单
- `POST /api/meals/save` - 保存餐单
- `DELETE /api/meals/:id` - 删除餐单

## 注意事项

- 服务器默认运行在 8000 端口
- 数据保存在 data 目录下的文本文件中
- 首次运行会自动生成示例数据
- 按 Ctrl+C 可以停止服务器

## 许可证

本项目仅供学习和研究使用。
