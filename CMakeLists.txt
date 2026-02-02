cmake_minimum_required(VERSION 3.10)
project(MPHF C)

# 设置C标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 添加库
add_library(mphf STATIC mphf.c)

# 添加可执行文件
add_executable(mphf_example example.c)

# 链接库到可执行文件
target_link_libraries(mphf_example mphf)

# 包含目录
target_include_directories(mphf_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 如果是Windows平台，添加相关定义
if(WIN32)
    target_compile_definitions(mphf_example PRIVATE _WIN32)
endif()

# # 安装规则（可选）
# install(TARGETS mphf DESTINATION lib)
# install(FILES mphf.h DESTINATION include)