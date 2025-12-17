cmake_minimum_required(VERSION 3.10)
project(MealRecommendationSystem)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用多处理器编译
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# 查找包含目录
include_directories(./include)
include_directories(./src)

# 设置源码文件
set(SOURCES
    src/main.cpp
    src/WebServer.cpp
    src/User.cpp
    src/Food.cpp
    src/Meal.cpp
    src/Database.cpp
    src/RecommendationEngine.cpp
    src/Utils.cpp
)

# 创建可执行文件
add_executable(MealRecommendationServer ${SOURCES})

# 链接库
if(WIN32)
    target_link_libraries(MealRecommendationServer ws2_32)
else()
    # Linux系统可能需要其他库
    find_package(Threads REQUIRED)
    target_link_libraries(MealRecommendationServer Threads::Threads)
endif()

# 创建数据目录
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data)

# 设置输出目录
set_target_properties(MealRecommendationServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# 复制HTML文件到构建目录
configure_file(${CMAKE_SOURCE_DIR}/src/index.html ${CMAKE_CURRENT_BINARY_DIR}/index.html COPYONLY)