cmake_minimum_required(VERSION 3.8)

project(MyProject
    VERSION 1.0.0
    DESCRIPTION "Description"
    LANGUAGES CXX
)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE Release)
endif()

# Custom cmake file folder location
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/res/cmake/")

# Enable "coverage" target
option(CODE_COVERAGE "Should code coverage be enabled (debug only)" ON)
if (NOT UNIX OR NOT CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CODE_COVERAGE OFF)
endif()

# Enable "static-check" target
option(CODE_CHECKS "Should static analysis (clang-tidy) be enabled" ON)
if (NOT UNIX)
    set(CODE_CHECKS OFF)
endif()

# Create re-usable config libraries
## Default compilation options
add_library(default_options INTERFACE)
target_include_directories(default_options
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_features(default_options
    INTERFACE
        cxx_std_17
)
target_compile_options(default_options
    INTERFACE
        "-Wall" "-Wextra" "-pedantic"
        $<$<CONFIG:Release>:-Werror -Ofast>
        $<$<CONFIG:Debug>:-O0 -ggdb>
)

## Code coverage options
add_library(coverage_options INTERFACE)
if (CODE_COVERAGE)
    include(CodeCoverage)
    target_compile_options(coverage_options
        INTERFACE
            "--coverage"
    )
    target_link_options(coverage_options
        INTERFACE
            "--coverage"
    )
endif()

# Add static analysis ability
if (CODE_CHECKS)
    SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    find_program(CLANG_TIDY NAMES clang-tidy)
    find_program(RUN_CLANG_TIDY NAMES run-clang-tidy)
    if (CLANG_TIDY AND RUN_CLANG_TIDY)
        message(STATUS "Enabling static checks with run-clang-tidy !")
        add_custom_target(static-check
            COMMAND ${RUN_CLANG_TIDY} -quiet -p ${CMAKE_BINARY_DIR}
        )
    else()
        message(WARNING "Missing (run-)clang-tidy, disabling static checks")
    endif()
endif()


# Create SOURCE_LIB static library
add_subdirectory(src)

# Add test folder
enable_testing()
add_subdirectory(test)


# Add check target to check everything
