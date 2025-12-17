# 智能配餐推荐系统 - 部署指南

本指南将指导您如何将智能配餐推荐系统部署为容器化应用，使其可以在任何支持 Docker 的环境中运行。

## 前置要求

- [Docker](https://docs.docker.com/get-docker/)
- [Docker Compose](https://docs.docker.com/compose/install/)

## 快速开始

### 1. 使用 Docker Compose (推荐)

我们提供了 `docker-compose.yml` 文件，可以一键构建并运行系统。

**构建镜像：**

```bash
docker-compose build
```

**运行系统：**

你可以直接使用我们提供的脚本：

```bash
./run_docker.sh
```

或者手动运行：

由于本系统是一个交互式命令行程序，需要使用 `run` 命令而不是 `up`，并且不需要 `-d` (后台运行)。

```bash
docker-compose run --rm meal-system
```

- `--rm`：容器退出后自动删除，保持环境清洁。
- 数据持久化：系统数据会自动保存在 `meal_data` 卷中，即使删除容器，数据也不会丢失。

### 2. 使用 Docker 手动构建和运行

如果您不想使用 Docker Compose，也可以手动使用 Docker 命令。

**构建镜像：**

```bash
docker build -t meal-recommendation-system .
```

**运行容器：**

```bash
docker run -it --rm \
  -v meal_data_manual:/app/build/bin/data \
  meal-recommendation-system
```

- `-it`：开启交互式终端，这对本系统是必须的。
- `-v ...`：挂载数据卷，确保用户数据和历史记录持久化保存。

## 数据备份与迁移

如果您使用 Docker Compose 部署，数据存储在名为 `meal_data` 的 Docker 卷中。

**查看数据卷位置：**

```bash
docker volume inspect meal_data
```

**备份数据：**

您可以将数据卷中的文件复制出来进行备份：

```bash
# 创建一个临时容器来访问数据卷并打包
docker run --rm -v meal_data:/data -v $(pwd):/backup ubuntu tar cvf /backup/meal_data_backup.tar /data
```

**恢复数据：**

```bash
# 将备份数据解压回数据卷
docker run --rm -v meal_data:/data -v $(pwd):/backup ubuntu bash -c "cd / && tar xvf /backup/meal_data_backup.tar"
```

## 常见问题

### 1. 中文显示乱码

我们在 Docker 镜像中已经配置了完整的 UTF-8 中文环境 (`zh_CN.UTF-8`)，应该可以正常显示中文。如果仍然遇到乱码，请检查您的终端模拟器（如 iTerm2, Windows Terminal, PuTTY 等）的编码设置是否为 UTF-8。

### 2. 无法输入

请确保在运行 `docker run` 时加上了 `-it` 参数，或在 `docker-compose.yml` 中配置了 `stdin_open: true` 和 `tty: true`（我们已经默认配置好了）。

## Windows 本地部署 (Visual Studio) 🪟

如果您希望在 Windows 上获得最佳的本地体验，或需要分发给其他 Windows 用户，请按照以下步骤操作。

### 1. 环境准备

- 操作系统：Windows 10/11
- 编译器：[Visual Studio 2022](https://visualstudio.microsoft.com/zh-hans/vs/) (需安装 "使用 C++ 的桌面开发" 工作负载)
- 构建工具：CMake (Visual Studio 通常已内置)

### 2. 一键编译与发布

我们提供了批处理脚本来简化编译和打包过程。

1. **编译项目**
   双击运行项目根目录下的 `build.bat`。
   - 这将自动检测 VS2022，配置项目并进行 Release 模式编译。
   - 编译完成后，可执行文件位于 `build/bin/Release/`。

2. **打包发布 (推荐)**
   双击运行 `deploy_windows.bat`。
   - 此脚本会检查编译状态（如果未编译会自动编译）。
   - 创建一个独立的 `dist/MealRecommendationSystem_v2.0` 文件夹。
   - 包含可执行文件、必要的文档和 `启动系统.bat`。
   - **您可以直接将此文件夹压缩并发送给用户，无需安装任何开发环境即可运行。**

### 3. 手动编译

如果您更喜欢使用命令行：

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### 4. 常见问题 (Windows)

**Q: 中文显示乱码？**
A: 我们的启动脚本 (`run.bat` 和 `启动系统.bat`) 已经内置了 `chcp 65001` 命令来切换控制台到 UTF-8 模式。请务必通过这些脚本启动，而不是直接双击 exe 文件。

**Q: 杀毒软件误报？**
A: 由于是未签名的自制软件，某些杀毒软件可能会提示风险。请将其添加到信任列表。
