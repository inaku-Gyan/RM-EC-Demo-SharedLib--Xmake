cmake_minimum_required(VERSION 3.24)

project(ecx
    VERSION 0.1.0
    DESCRIPTION "Header-only shared library for embedded RoboMaster projects"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(ecx INTERFACE)
add_library(ecx::ecx ALIAS ecx)

target_include_directories(ecx INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_compile_features(ecx INTERFACE cxx_std_20)

add_subdirectory(deps_inc)

option(ECX_BUILD_TESTS     "Build host-side tests" ${PROJECT_IS_TOP_LEVEL})
option(ECX_BUILD_ARM_CHECK "Compile algo+proto with arm-none-eabi-gcc as a smoke check" OFF)

if(ECX_BUILD_TESTS AND NOT CMAKE_CROSSCOMPILING)
    enable_testing()
    add_subdirectory(tests)
endif()

if(ECX_BUILD_ARM_CHECK)
    file(GLOB_RECURSE ALL_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
    set(CHECK_CPP_CONTENT "// Auto-generated compile check file\n")
    foreach(HEADER IN LISTS ALL_HEADERS)
        file(RELATIVE_PATH REL_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/src" "${HEADER}")
        if(NOT REL_HEADER MATCHES "(^|/)_")
            string(APPEND CHECK_CPP_CONTENT "#include <${REL_HEADER}>\n")
        endif()
    endforeach()

    set(CHECK_CPP_FILE "${CMAKE_CURRENT_BINARY_DIR}/arm_compile_check_gen.cpp")
    file(WRITE "${CHECK_CPP_FILE}" "${CHECK_CPP_CONTENT}")

    add_library(ecx_arm_check STATIC "${CHECK_CPP_FILE}")
    target_compile_definitions(ecx_arm_check PRIVATE
        "ECX_INC_USER_CONFIG=\"ecx_config_template.hpp\""
    )
    target_link_libraries(ecx_arm_check PRIVATE
        deps
        ecx::ecx
    )
endif()
