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

# 查找系统库 (Mobi 和 PDF 通用)
find_library(log-lib log)
find_library(z-lib z)

# 查找 PDF 渲染所需的系统底层库
find_library(jnigraphics-lib jnigraphics)
find_library(android-lib android)

# ==========================================
# 2. Mobi 解析库 (yukino_mobi)
# ==========================================
set(MOBI_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/libmobi)
set(MOBI_SRC_DIR ${MOBI_ROOT}/src)

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

# ==========================================
# 3. PDFium 解析库 (yukino_pdf)
# ==========================================
# 指向刚才新建的 pdfium 头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/pdfium/include)

# 引入预编译的 4.7MB 单体核心库 (放在上一级的 jniLibs 目录下)
add_library(pdfium_prebuilt SHARED IMPORTED)
set_target_properties(pdfium_prebuilt PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libpdfium.so)

# 编译咱们自己的 JNI 桥接代码
add_library(yukino_pdf SHARED pdfium/pdf_bridge.cpp)

# 链接依赖关系：桥接代码 -> PDF内核 -> 系统图像处理库
target_link_libraries(yukino_pdf
        pdfium_prebuilt
        ${log-lib}
        ${android-lib}
        ${jnigraphics-lib}
)
