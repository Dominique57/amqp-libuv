find_package(amqpcpp REQUIRED)
find_package(uvw REQUIRED)

add_library(SOURCE_LIB STATIC
    entrypoint.cc entrypoint.h
    rbmq/uvwConnectionHandler.cc rbmq/uvwConnectionHandler.h
    rbmq/uvwConnection.cc rbmq/uvwConnection.h

    rbmq/utils/concurentQueue.h

    example/module.cc example/module.h
    example/communicator.cc example/communicator.h
)
target_link_libraries(SOURCE_LIB
    PUBLIC
        amqpcpp
        uvw::uvw
        coverage_options
    PRIVATE
        default_options
)

# =============================
# Generate binary using library
find_package(Boost COMPONENTS program_options REQUIRED)
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(Boost_USE_STATIC_LIBS ON)
endif()

add_executable(${PROJECT_NAME} main.cc)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC
    Boost::program_options
    default_options
    SOURCE_LIB
)
