# 最低CMake版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称和语言
project(MyProtoCode CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录 - 为不同构建类型设置不同的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/output/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/output/release)

# 查找依赖库
# 1. pthread库
find_package(Threads REQUIRED)

# 设置muduo库路径（根据实际安装路径调整）
set(MUDUO_INCLUDE_DIR "/path/to/muduo/include")  # 替换为实际路径
set(MUDUO_LIB_DIR "/path/to/muduo/lib")          # 替换为实际路径

# 添加头文件搜索路径
include_directories(
    ${CMAKE_SOURCE_DIR}/Myproto
    ${CMAKE_SOURCE_DIR}/Connect
    ${CMAKE_SOURCE_DIR}/ServiceHandler
    ${CMAKE_SOURCE_DIR}/Server
    ${CMAKE_SOURCE_DIR}/Client
    ${MUDUO_INCLUDE_DIR}  # 添加muduo头文件路径
)

# 添加库文件搜索路径
link_directories(${MUDUO_LIB_DIR})  # 添加muduo库文件路径

# 查找muduo库
find_library(MUDUO_NET_LIB muduo_net)
find_library(MUDUO_BASE_LIB muduo_base)

if(NOT MUDUO_NET_LIB OR NOT MUDUO_BASE_LIB)
    message(FATAL_ERROR "muduo library not found")
endif()

# 收集所有源代码文件
file(GLOB_RECURSE SERVER_SOURCES
    ${CMAKE_SOURCE_DIR}/main.cpp
    ${CMAKE_SOURCE_DIR}/Myproto/*.cpp
    ${CMAKE_SOURCE_DIR}/Myproto/*.cc
    ${CMAKE_SOURCE_DIR}/Connect/*.cpp
    ${CMAKE_SOURCE_DIR}/ServiceHandler/*.cpp
    ${CMAKE_SOURCE_DIR}/Server/*.cpp
)

file(GLOB_RECURSE CLIENT_SOURCES
    ${CMAKE_SOURCE_DIR}/test/client_main.cpp
    ${CMAKE_SOURCE_DIR}/Myproto/*.cpp
    ${CMAKE_SOURCE_DIR}/Myproto/*.cc
    ${CMAKE_SOURCE_DIR}/Connect/*.cpp
    ${CMAKE_SOURCE_DIR}/ServiceHandler/*.cpp
    ${CMAKE_SOURCE_DIR}/Client/*.cpp
)

# 构建服务器可执行文件
add_executable(myproto_server ${SERVER_SOURCES})

# 构建客户端测试可执行文件
add_executable(myproto_client_test ${CLIENT_SOURCES})

# 设置不同构建类型的编译选项
# 调试版本选项
target_compile_options(myproto_server PRIVATE
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-Wall>
    $<$<CONFIG:Debug>:-Wextra>
    $<$<CONFIG:Debug>:-pedantic>
)

target_compile_options(myproto_client_test PRIVATE
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-Wall>
    $<$<CONFIG:Debug>:-Wextra>
    $<$<CONFIG:Debug>:-pedantic>
)

# 发布版本选项
target_compile_options(myproto_server PRIVATE
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Release>:-Wall>
    $<$<CONFIG:Release>:-Wextra>
    $<$<CONFIG:Release>:-pedantic>
    $<$<CONFIG:Release>:-DNDEBUG>
)

target_compile_options(myproto_client_test PRIVATE
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Release>:-Wall>
    $<$<CONFIG:Release>:-Wextra>
    $<$<CONFIG:Release>:-pedantic>
    $<$<CONFIG:Release>:-DNDEBUG>
)

# 链接依赖库
target_link_libraries(myproto_server
    Threads::Threads
    ${MUDUO_NET_LIB}
    ${MUDUO_BASE_LIB}
)

target_link_libraries(myproto_client_test
    Threads::Threads
    ${MUDUO_NET_LIB}
    ${MUDUO_BASE_LIB}
)

# 安装规则
install(TARGETS myproto_server myproto_client_test
    RUNTIME DESTINATION bin
)

# 构建测试选项
option(BUILD_TESTS "Build test programs" ON)

if(BUILD_TESTS)
    message(STATUS "Tests will be built")
endif()