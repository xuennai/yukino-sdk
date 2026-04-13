cmake_minimum_required(VERSION 3.22.1)
project("yukino_native")

# ==========================================
# 1. 全局配置
# ==========================================
# 极致的体积优化和死代码消除编译参数
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -fvisibility=hidden -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -fvisibility=hidden -fvisibility-inlines-hidden -ffunction-sections -fdata-sections")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections -s")

add_definitions(-DUSE_ZLIB)

# 查找系统库
find_library(log-lib log)
find_library(z-lib z)

# ==========================================
# 2. Mobi 解析库 (yukino_mobi)
# ==========================================
set(MOBI_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/libmobi)
set(MOBI_SRC_DIR ${MOBI_ROOT}/src)

if(NOT EXISTS "${MOBI_SRC_DIR}/read.c")
    message(FATAL_ERROR "
    🚨 [Yukino 致命错误] 找不到 libmobi 源码！
    路径: ${MOBI_SRC_DIR} 是空的！
    请确保 CI 环境执行了: git submodule update --init --recursive
    ")
endif()

include_directories(${MOBI_SRC_DIR})

set(MOBI_SOURCES
        mobi_bridge.c
        ${MOBI_SRC_DIR}/buffer.c
        ${MOBI_SRC_DIR}/compression.c
        ${MOBI_SRC_DIR}/debug.c
        ${MOBI_SRC_DIR}/encryption.c
        ${MOBI_SRC_DIR}/index.c
        ${MOBI_SRC_DIR}/memory.c
        ${MOBI_SRC_DIR}/meta.c
        ${MOBI_SRC_DIR}/parse_rawml.c
        ${MOBI_SRC_DIR}/randombytes.c
        ${MOBI_SRC_DIR}/read.c
        ${MOBI_SRC_DIR}/sha1.c
        ${MOBI_SRC_DIR}/structure.c
        ${MOBI_SRC_DIR}/util.c
        ${MOBI_SRC_DIR}/xmlwriter.c
)

add_library(yukino_mobi SHARED ${MOBI_SOURCES})
target_link_libraries(yukino_mobi ${log-lib} ${z-lib})
