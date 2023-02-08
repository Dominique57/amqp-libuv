# Copyright (c) 2012 - 2017, Lars Bilke
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


# File heavily modified by :
# Dominique MICHEL
# https://github.com/Dominique57/ProjectBase


include(CMakeParseArguments)

# Check prereqs
find_program(GCOV_PATH gcov)
find_program(LCOV_PATH  NAMES lcov lcov.bat lcov.exe lcov.perl)
find_program(GENHTML_PATH NAMES genhtml genhtml.perl genhtml.bat)
find_program(CPPFILT_PATH NAMES c++filt)

if(NOT GCOV_PATH)
    message(FATAL_ERROR "gcov not found! Aborting...")
endif()

# Check supported compiler (Clang, GNU and Flang)
get_property(LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)
foreach(LANG ${LANGUAGES})
  if("${CMAKE_${LANG}_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
    if("${CMAKE_${LANG}_COMPILER_VERSION}" VERSION_LESS 3)
      message(FATAL_ERROR "Clang version must be 3.0.0 or greater! Aborting...")
    endif()
  elseif(NOT "${CMAKE_${LANG}_COMPILER_ID}" MATCHES "GNU"
         AND NOT "${CMAKE_${LANG}_COMPILER_ID}" MATCHES "(LLVM)?[Ff]lang")
    message(FATAL_ERROR "Compiler is not GNU or Flang! Aborting...")
  endif()
endforeach()

# Warning if code could be compiled in non-debug mode
get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug" OR GENERATOR_IS_MULTI_CONFIG))
    message(WARNING "Code coverage results with an optimised (non-Debug) build may be misleading")
endif()


# Defines a target for running and collection code coverage information
# Builds dependencies, runs the given executable and outputs reports.
# NOTE! The executable should always have a ZERO as exit code otherwise
# the coverage generation will not complete.
#
# setup_target_for_coverage_lcov(
#     NAME testrunner_coverage                    # New target name
#     BASE_DIRECTORY "../"                        # Base directory for report
#                                                 #  (defaults to PROJECT_SOURCE_DIR)
#     OUTPUT_DIRECTORY "."                        # Directory where html file will be
#                                                 # stored (defaults to .)
#     EXCLUDE "src/dir1/*" "src/dir2/*"           # Patterns to exclude (can be relative
#                                                 #  to BASE_DIRECTORY, with CMake 3.4+)
#     EXECUTABLE testrunner -j ${PROCESSOR_COUNT} # Executable in PROJECT_BINARY_DIR
#     DEPENDENCIES testrunner                     # Dependencies to build first
#     NO_DEMANGLE                                 # Don't demangle C++ symbols
#                                                 #  even if c++filt is found
# )
function(setup_target_for_coverage_lcov)
    # Additional checks only if function used
    if(NOT LCOV_PATH)
        message(FATAL_ERROR "lcov not found! Aborting...")
    endif()

    if(NOT GENHTML_PATH)
        message(FATAL_ERROR "genhtml not found! Aborting...")
    endif()

    # Function parameters
    set(options
        NO_DEMANGLE)
    set(oneValueArgs
        BASE_DIRECTORY NAME OUTPUT_DIRECTORY)
    set(multiValueArgs
        EXCLUDE EXECUTABLE EXECUTABLE_ARGS DEPENDENCIES LCOV_ARGS GENHTML_ARGS)
    cmake_parse_arguments(Coverage
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    # Set base directory (as absolute path), or default to PROJECT_SOURCE_DIR
    if(DEFINED Coverage_BASE_DIRECTORY)
        get_filename_component(BASEDIR ${Coverage_BASE_DIRECTORY} ABSOLUTE)
    else()
        set(BASEDIR ${PROJECT_SOURCE_DIR})
    endif()

    if(NOT DEFINED Coverage_OUTPUT_DIRECTORY)
        set(Coverage_OUTPUT_DIRECTORY ".")
    endif()

    # Collect excludes (CMake 3.4+: Also compute absolute paths)
    set(LCOV_EXCLUDES "")
    foreach(EXCLUDE ${Coverage_EXCLUDE} ${COVERAGE_EXCLUDES} ${COVERAGE_LCOV_EXCLUDES})
        if(CMAKE_VERSION VERSION_GREATER 3.4)
            get_filename_component(EXCLUDE ${EXCLUDE} ABSOLUTE BASE_DIR ${BASEDIR})
        endif()
        list(APPEND LCOV_EXCLUDES "${EXCLUDE}")
    endforeach()
    list(REMOVE_DUPLICATES LCOV_EXCLUDES)

    # Conditional arguments
    if(CPPFILT_PATH AND NOT ${Coverage_NO_DEMANGLE})
      set(GENHTML_EXTRA_ARGS "--demangle-cpp")
    endif()

    # Setting up commands which will be run to generate coverage data.
    # Cleanup lcov
    set(LCOV_CLEAN_CMD
        ${LCOV_PATH} ${Coverage_LCOV_ARGS} --gcov-tool ${GCOV_PATH}
            -directory . -b ${BASEDIR} --zerocounters
    )
    # Create baseline to make sure untouched files show up in the report
    set(LCOV_BASELINE_CMD
        ${LCOV_PATH} ${Coverage_LCOV_ARGS} --gcov-tool ${GCOV_PATH} -c -i -d .
            -b ${BASEDIR} -o ${Coverage_NAME}.base
    )
    # Run tests
    set(LCOV_EXEC_TESTS_CMD
        ${Coverage_EXECUTABLE} ${Coverage_EXECUTABLE_ARGS}
    )
    # Capturing lcov counters and generating report
    set(LCOV_CAPTURE_CMD
        ${LCOV_PATH} ${Coverage_LCOV_ARGS} --gcov-tool ${GCOV_PATH}
            --directory . -b ${BASEDIR} --capture
            --output-file ${Coverage_NAME}.capture
    )
    # add baseline counters
    set(LCOV_BASELINE_COUNT_CMD
        ${LCOV_PATH} ${Coverage_LCOV_ARGS} --gcov-tool ${GCOV_PATH}
            -a ${Coverage_NAME}.base -a ${Coverage_NAME}.capture
            --output-file ${Coverage_NAME}.total
    )
    # filter collected data to final coverage report
    set(LCOV_FILTER_CMD
        ${LCOV_PATH} ${Coverage_LCOV_ARGS} --gcov-tool ${GCOV_PATH}
            --remove ${Coverage_NAME}.total ${LCOV_EXCLUDES}
            --output-file ${Coverage_NAME}.info
    )
    # Generate HTML output
    set(LCOV_GEN_HTML_CMD
        ${GENHTML_PATH} ${GENHTML_EXTRA_ARGS} ${Coverage_GENHTML_ARGS}
            -o ${Coverage_OUTPUT_DIRECTORY}/${Coverage_NAME}
            ${Coverage_NAME}.info
    )

    # Setup target
    add_custom_target(${Coverage_NAME}
        COMMAND ${LCOV_CLEAN_CMD}
        COMMAND ${LCOV_BASELINE_CMD}
        COMMAND ${LCOV_EXEC_TESTS_CMD}
        COMMAND ${LCOV_CAPTURE_CMD}
        COMMAND ${LCOV_BASELINE_COUNT_CMD}
        COMMAND ${LCOV_FILTER_CMD}
        COMMAND ${LCOV_GEN_HTML_CMD}

        # Set output files as GENERATED (will be removed on 'make clean')
        BYPRODUCTS
            ${Coverage_NAME}.base
            ${Coverage_NAME}.capture
            ${Coverage_NAME}.total
            ${Coverage_NAME}.info
            ${Coverage_NAME}/index.html
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        DEPENDS ${Coverage_DEPENDENCIES}
        VERBATIM # Protect arguments to commands
        COMMENT "Resetting code coverage counters to zero.\nProcessing code coverage counters and generating report."
    )

    # Show where to find the lcov info report
    add_custom_command(TARGET ${Coverage_NAME} POST_BUILD
        COMMAND ;
        COMMENT "Lcov code coverage info report saved in ${Coverage_NAME}.info."
    )

    # Show info where to find the report
    add_custom_command(TARGET ${Coverage_NAME} POST_BUILD
        COMMAND ;
        COMMENT "Open ${Coverage_OUTPUT_DIRECTORY}/${Coverage_NAME}/index.html in your browser to view the coverage report."
    )

endfunction()

