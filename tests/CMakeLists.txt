include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
    GIT_SHALLOW    TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# 顶层 tests/ 的 inc/ 目录——helper 会把它加进每个测试 target 的 PRIVATE include path，
# 这样 `<common/ecx_config_xxx.hpp>` 这种角括号 include 才能在编译时被 __has_include 命中。
# 必须在 helper 函数定义之前 set，且作用域延伸到所有 add_subdirectory 进来的模块。
set(ECX_TESTS_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc")

# ---------------------------------------------------------------------------
# 目录约定
#   tests/
#   ├── inc/common/
#   │       ├── ecx_config_<name>.hpp           # 跨模块共享 config 头（扁平、统一命名）
#   │       └── ...                             # 跨模块公用测试工具 (headers-only)
#   ├── unit/**/<module>/
#   │           ├── CMakeLists.txt          # 该模块显式声明它的所有测试 exec
#   │           ├── test_*.cpp              # 测试源（变体专属与默认同层）
#   │           └── ecx_config_<name>.hpp   # （可选）模块本地 config，优先于共享
#   └── CMakeLists.txt
#
# 测试 exec 与 config 的对应关系完全由模块 CMakeLists.txt 显式声明，
# 顶层只做：定义工具/helper + 自动 add_subdirectory 带 CMakeLists.txt 的模块。
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
# 声明一个测试可执行文件。
#   ecx_add_test_executable(
#       NAME    <exec_name>
#       SOURCES <src1> [<src2> ...]
#       [CONFIG <ecx_config_xxx.hpp>]   # 可选；通过 ECX_INC_USER_CONFIG 注入
#                                       # （不传则走 ecx_config_default.hpp）
#       [PREFIX <ctest_prefix>]         # 可选；默认 "<NAME>::"
#   )
# 行为：
#   - 模块目录与共享 configs/ 都在 PRIVATE include 路径上，
#     所以 CONFIG 字符串既可以指向模块本地 ecx_config_*.hpp，
#     也可以指向 tests/configs/ 下的共享 ecx_config_*.hpp。
#   - 不传 CONFIG 时 ECX_INC_USER_CONFIG 保持默认值 "ecx_config.hpp"，
#     在测试 include 路径里找不到此文件 → __has_include 返回 false → 默认 config。
# ---------------------------------------------------------------------------
function(ecx_add_test_executable)
    cmake_parse_arguments(ARG "" "NAME;CONFIG;DEFINES;PREFIX" "SOURCES" ${ARGN})

    if(NOT ARG_NAME)
        message(FATAL_ERROR "ecx_add_test_executable: 缺少 NAME 参数")
    endif()
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "ecx_add_test_executable(${ARG_NAME}): 缺少 SOURCES")
    endif()

    add_executable(${ARG_NAME} ${ARG_SOURCES})
    target_link_libraries(${ARG_NAME} PRIVATE
        deps
        ecx::ecx
        GTest::gtest_main
        GTest::gtest
        GTest::gmock
    )
    target_include_directories(${ARG_NAME} PRIVATE
        "${ECX_TESTS_INC_DIR}"             # 顶层 tests/inc/：跨模块共享 config 与工具头
    )

    if(ARG_CONFIG)
        # 把 helper 的 CONFIG 字符串注入 ECX_INC_USER_CONFIG，
        # 让 src/_inc/configuration.hpp 通过 __has_include + #include 间接拉到指定 config 头。
        target_compile_definitions(${ARG_NAME} PRIVATE
            "ECX_INC_USER_CONFIG=\"${ARG_CONFIG}\""
        )
    endif()

    if(ARG_DEFINES)
        target_compile_definitions(${ARG_NAME} PRIVATE ${ARG_DEFINES})
    endif()

    set(prefix "${ARG_PREFIX}")
    if(NOT prefix)
        set(prefix "${ARG_NAME}::")
    endif()
    gtest_discover_tests(${ARG_NAME} TEST_PREFIX "${prefix}")
endfunction()

# ---------------------------------------------------------------------------
# 自动 add_subdirectory：unit/ 下任何带 CMakeLists.txt 的子目录都纳入。
# 仅做"目录是否参与构建"的判定，不揣测目录里应该产生哪些 exec。
# ---------------------------------------------------------------------------
file(GLOB_RECURSE _module_paths LIST_DIRECTORIES true CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/unit/*"
)
foreach(_module_path IN LISTS _module_paths)
    if(IS_DIRECTORY "${_module_path}" AND EXISTS "${_module_path}/CMakeLists.txt")
        add_subdirectory("${_module_path}")
    endif()
endforeach()
