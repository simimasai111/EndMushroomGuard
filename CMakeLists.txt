cmake_minimum_required(VERSION 3.20)
project(EndMushroomGuard LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# LeviLamina SDK 随服务器安装（提供 ll/api 与 mc/ 头文件、库）。
# 若 find_package 找不到，用 -DLeviLamina_DIR=<SDK 的 cmake 配置目录> 指定。
# 也可改用 xmake：add_requires("levilamina") 后 target_add_requires。
find_package(LeviLamina CONFIG REQUIRED)

add_library(${PROJECT_NAME} SHARED src/main.cpp)

# 链接目标名以当前 SDK 导出为准（常见为 LeviLamina::LeviLamina）。
target_link_libraries(${PROJECT_NAME} PRIVATE LeviLamina::LeviLamina)
