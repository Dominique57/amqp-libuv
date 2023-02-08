find_package(GTest)

if (GTest_FOUND)

    # gather all *.cc files
    file(GLOB TEST_SRC *.cc)

    # if you need to copy from source to build folder do it here
    # configure_file(SRC DST COPYONLY)

    add_executable(UNIT_TEST ${TEST_SRC})
    target_link_libraries(UNIT_TEST
        PRIVATE
            default_options
            coverage_options
            SOURCE_LIB
            GTest::GTest
    )

    add_test(NAME test_unitaire
             COMMAND UNIT_TEST)

    if (CODE_COVERAGE)
        message(STATUS "Enabling code coverage using lcov and genhtml !")
        setup_target_for_coverage_lcov(
            NAME coverage
            EXCLUDE
                "/usr/*"
            EXECUTABLE UNIT_TEST
        )
    endif()
else()
    message(WARNING "Google test not found, tests cannot be executed !")
endif()
