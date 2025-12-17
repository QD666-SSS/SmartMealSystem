cmake_minimum_required(VERSION 3.10)
project(MealRecommendationSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置源文件
set(SOURCES
    src/main.cpp
    src/User.cpp
    src/Food.cpp
    src/Meal.cpp
    src/Database.cpp
    src/RecommendationEngine.cpp
    src/Utils.cpp
)

# 添加资源文件 (Windows)
if(WIN32)
    list(APPEND SOURCES resources/version.rc)
endif()

# 设置头文件
set(HEADERS
    include/User.h
    include/Food.h
    include/Meal.h
    include/Database.h
    include/RecommendationEngine.h
    include/Utils.h
)

# 添加可执行文件
add_executable(MealRecommendationSystem ${SOURCES} ${HEADERS})

# 包含头文件目录
target_include_directories(MealRecommendationSystem PRIVATE include)

# 编译器特定设置
if(MSVC)
    # MSVC编译器设置
    target_compile_options(MealRecommendationSystem PRIVATE
        /utf-8           # 使用UTF-8编码
        /W3              # 警告级别3
        /wd4819          # 禁用C4819编码警告
    )
    target_compile_definitions(MealRecommendationSystem PRIVATE 
        UNICODE 
        _UNICODE
        _CRT_SECURE_NO_WARNINGS
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang编译器设置
    target_compile_options(MealRecommendationSystem PRIVATE
        -finput-charset=UTF-8
        -fexec-charset=UTF-8
        -Wall
        -Wextra
    )
endif()

# Windows特定设置
if(WIN32)
    # 设置控制台应用程序
    set_target_properties(MealRecommendationSystem PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# 设置输出目录
set_target_properties(MealRecommendationSystem PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 复制数据目录到构建目录
add_custom_command(TARGET MealRecommendationSystem POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    $<TARGET_FILE_DIR:MealRecommendationSystem>/data
)
