cmake_minimum_required(VERSION 3.10)
project(MealRecommendationSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加包含目录
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/third_party)

# 源文件
set(SOURCES
    src/main.cpp
    src/User.cpp
    src/Food.cpp
    src/Meal.cpp
    src/Database.cpp
    src/RecommendationEngine.cpp
    src/Utils.cpp
    src/WebServer.cpp
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# Linux/Unix平台需要链接pthread
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 复制data和www文件夹到输出目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/www
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/www
)

message(STATUS "========================================")
message(STATUS "注意：本项目主要为 Windows + Visual Studio 2022 设计")
message(STATUS "推荐使用 Visual Studio 2022 打开 MealRecommendationSystem.sln")
message(STATUS "CMake 构建仅用于测试和非Windows平台")
message(STATUS "========================================")
