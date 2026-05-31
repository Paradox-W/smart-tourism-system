cmake_minimum_required(VERSION 3.15)
project(SmartTourismSystem VERSION 1.0.0 LANGUAGES C CXX)

# C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 覆盖默认 Debug 编译标志
if(MSVC)
    set(CMAKE_C_FLAGS_DEBUG "/Od" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "/Od" CACHE STRING "" FORCE)
else()
    set(CMAKE_C_FLAGS_DEBUG "-O0" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "-O0" CACHE STRING "" FORCE)
endif()

# ============================================================
# 头文件目录
# ============================================================
include_directories(
    ${CMAKE_SOURCE_DIR}/third_party      # cpp-httplib.h, json.hpp, sqlite3.h
    ${CMAKE_SOURCE_DIR}/include           # 项目头文件
)

# ============================================================
# 收集源文件
# ============================================================
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 排除 tests 目录
list(FILTER SOURCES EXCLUDE REGEX "tests/")

# ============================================================
# 构建可执行文件（包含 sqlite3.c）
# ============================================================
add_executable(${PROJECT_NAME} ${SOURCES} ${CMAKE_SOURCE_DIR}/third_party/sqlite3.c)

# 编译 sqlite3.c 时关闭警告（第三方源码），调试信息用 -gdwarf-4
set_source_files_properties(${CMAKE_SOURCE_DIR}/third_party/sqlite3.c PROPERTIES
    COMPILE_FLAGS "-w"
)

# 强制覆盖调试信息格式（MinGW 8.1 的 as 不支持 gdwarf-5/4）
# 只保留 -g 不带版本号
foreach(flag_var CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELEASE)
    string(REPLACE "-gdwarf-5" ""  ${flag_var} "${${flag_var}}")
    string(REPLACE "-gdwarf-4" ""  ${flag_var} "${${flag_var}}")
endforeach()

# ============================================================
# 链接库
# ============================================================
# Windows 平台需要链接 ws2_32 (Winsock) 和 advapi32
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 advapi32)
endif()

# 需要链接 pthread（MinGW/gcc）
if(NOT MSVC)
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
    target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++ -static)
endif()

# Copy MinGW runtime DLLs next to the executable so the demo does not pick up
# incompatible DLLs from PATH.
if(WIN32 AND NOT MSVC)
    foreach(runtime_dll libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll)
        find_file(MINGW_RUNTIME_${runtime_dll} ${runtime_dll})
        if(MINGW_RUNTIME_${runtime_dll})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${MINGW_RUNTIME_${runtime_dll}}
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>/${runtime_dll}
                COMMENT "Copying MinGW runtime ${runtime_dll}"
            )
        endif()
    endforeach()
endif()

# ============================================================
# 编译选项
# ============================================================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /utf-8
        /W4
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Release>:/O2>
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Release>:-O2>
    )
endif()

# ============================================================
# 算法测试编译选项
# 用法: cmake -DRUN_ALGORITHM_TESTS=ON ..
# 运行: ./bin/SmartTourismSystem --test
# ============================================================
option(RUN_ALGORITHM_TESTS "Compile algorithm unit tests" OFF)
if(RUN_ALGORITHM_TESTS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RUN_ALGORITHM_TESTS)
    target_sources(${PROJECT_NAME} PRIVATE tests/test_algorithms.cpp)
    message(STATUS "Algorithm tests: ENABLED")
endif()

# ============================================================
# 将 data 目录复制到构建输出目录
# ============================================================
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
    COMMENT "Copying data directory to output"
)

message(STATUS "")
message(STATUS "=== Smart Tourism System Build Configuration ===")
message(STATUS "C++ Standard: C++17")
message(STATUS "SQLite3: bundled (third_party/sqlite3.c)")
message(STATUS "Source files: ${SOURCES}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "")
