
add_library(SOURCE_LIB STATIC
    # TODO: add your source file here
    sample.cc sample.hh
)
target_link_libraries(SOURCE_LIB
    PRIVATE
        default_options
        coverage_options
)

# =============================
# Generate binary using library
find_package(Boost COMPONENTS program_options REQUIRED)
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(Boost_USE_STATIC_LIBS ON)
endif()

add_executable(${PROJECT_NAME} main.cc)
target_include_directories(${PROJECT_NAME} PUBLIC
    ${Boost_INCLUDE_DIR}
)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC
    ${Boost_LIBRARIES}
    default_options
    SOURCE_LIB
)
