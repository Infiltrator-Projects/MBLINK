# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.20)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" MBLINK_PROJECT_VERSION)
string(STRIP "${MBLINK_PROJECT_VERSION}" MBLINK_PROJECT_VERSION)
if(NOT MBLINK_PROJECT_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "VERSION must contain a semantic major.minor.patch version.")
endif()

project(MBLINK VERSION "${MBLINK_PROJECT_VERSION}" LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)

set(MBLINK_COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/infiltratr-common")
set(MBLINK_COMMON_VERSION "1.5.0")
set(MBLINK_COMMON_COMMIT "a0e75ffbe4e038c74c8f1e3d589f2dae87b2b7bb")

if(NOT EXISTS "${MBLINK_COMMON_DIR}/VERSION")
    find_program(MBLINK_GIT_EXECUTABLE git REQUIRED)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
        execute_process(
            COMMAND "${MBLINK_GIT_EXECUTABLE}" submodule update --init --depth 1 -- src/infiltratr-common
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            RESULT_VARIABLE MBLINK_COMMON_INIT_RESULT)
    else()
        set(MBLINK_COMMON_INIT_RESULT 1)
    endif()

    if(NOT MBLINK_COMMON_INIT_RESULT EQUAL 0 OR
       NOT EXISTS "${MBLINK_COMMON_DIR}/VERSION")
        message(FATAL_ERROR
            "Infiltratr Common ${MBLINK_COMMON_VERSION} is required. Clone with --recurse-submodules or run git submodule update --init --recursive.")
    endif()
endif()

file(READ "${MBLINK_COMMON_DIR}/VERSION" MBLINK_COMMON_FOUND_VERSION)
string(STRIP "${MBLINK_COMMON_FOUND_VERSION}" MBLINK_COMMON_FOUND_VERSION)
if(NOT MBLINK_COMMON_FOUND_VERSION STREQUAL MBLINK_COMMON_VERSION)
    message(FATAL_ERROR
        "Infiltratr Common ${MBLINK_COMMON_VERSION} is required; found ${MBLINK_COMMON_FOUND_VERSION}.")
endif()

find_program(MBLINK_GIT_EXECUTABLE git)
if(MBLINK_GIT_EXECUTABLE)
    execute_process(
        COMMAND "${MBLINK_GIT_EXECUTABLE}" -C "${MBLINK_COMMON_DIR}" rev-parse HEAD
        OUTPUT_VARIABLE MBLINK_COMMON_FOUND_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE MBLINK_COMMON_COMMIT_RESULT)
    if(MBLINK_COMMON_COMMIT_RESULT EQUAL 0 AND
       NOT MBLINK_COMMON_FOUND_COMMIT STREQUAL MBLINK_COMMON_COMMIT)
        message(FATAL_ERROR
            "Infiltratr Common must be pinned to ${MBLINK_COMMON_COMMIT}; found ${MBLINK_COMMON_FOUND_COMMIT}.")
    endif()
endif()

add_library(infiltratr-common STATIC
    "${MBLINK_COMMON_DIR}/src/core.c"
    "${MBLINK_COMMON_DIR}/src/format.c")
target_include_directories(infiltratr-common PUBLIC
    "${MBLINK_COMMON_DIR}/include")
target_compile_features(infiltratr-common PUBLIC c_std_11)

add_library(mblink STATIC
    src/core/mblink.c
    src/elm327/elm327.c
    src/elm327/session.c
    src/elm327/probe.c
    src/obd2/obd2.c)
target_include_directories(mblink
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(mblink PRIVATE
    MBLINK_VERSION="${PROJECT_VERSION}")
target_link_libraries(mblink PUBLIC infiltratr-common)

set(MBLINK_WARNING_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wformat=2
    -Wstrict-prototypes
    -Wmissing-prototypes)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(infiltratr-common PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
    target_compile_options(mblink PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(infiltratr-common PUBLIC m)
endif()

if(BUILD_TESTING)
    add_executable(mblink-core-smoke tests/core_smoke.c)
    target_link_libraries(mblink-core-smoke PRIVATE mblink)
    target_compile_definitions(mblink-core-smoke PRIVATE
        MBLINK_TEST_EXPECTED_VERSION="${PROJECT_VERSION}")

    add_executable(mblink-elm327-smoke tests/elm327_smoke.c)
    target_link_libraries(mblink-elm327-smoke PRIVATE mblink)

    add_executable(mblink-elm327-session-smoke tests/elm327_session_smoke.c)
    target_link_libraries(mblink-elm327-session-smoke PRIVATE mblink)

    add_executable(mblink-elm327-probe-smoke tests/elm327_probe_smoke.c)
    target_link_libraries(mblink-elm327-probe-smoke PRIVATE mblink)

    add_executable(mblink-obd2-smoke tests/obd2_smoke.c)
    target_link_libraries(mblink-obd2-smoke PRIVATE mblink)

    add_executable(mblink-runtime-smoke tests/runtime_smoke.c)
    target_link_libraries(mblink-runtime-smoke PRIVATE mblink)

    add_executable(mblink-isotp-smoke tests/isotp_smoke.c)
    target_link_libraries(mblink-isotp-smoke PRIVATE mblink)

    add_executable(mblink-isotp-failed-state-smoke
        tests/isotp_failed_state_smoke.c)
    target_link_libraries(mblink-isotp-failed-state-smoke PRIVATE mblink)

    add_executable(mblink-isotp-functional-response-smoke
        tests/isotp_functional_response_smoke.c)
    target_link_libraries(mblink-isotp-functional-response-smoke PRIVATE mblink)

    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(mblink-core-smoke PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
        target_compile_options(mblink-elm327-smoke PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
        target_compile_options(mblink-elm327-session-smoke PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
        target_compile_options(mblink-elm327-probe-smoke PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
        target_compile_options(mblink-obd2-smoke PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
        target_compile_options(mblink-runtime-smoke PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
        target_compile_options(mblink-isotp-smoke PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
        target_compile_options(mblink-isotp-failed-state-smoke PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
        target_compile_options(mblink-isotp-functional-response-smoke PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
    endif()

    add_test(NAME mblink-core-smoke COMMAND mblink-core-smoke)
    add_test(NAME mblink-elm327-smoke COMMAND mblink-elm327-smoke)
    add_test(NAME mblink-elm327-session-smoke COMMAND mblink-elm327-session-smoke)
    add_test(NAME mblink-elm327-probe-smoke COMMAND mblink-elm327-probe-smoke)
    add_test(NAME mblink-obd2-smoke COMMAND mblink-obd2-smoke)
    add_test(NAME mblink-runtime-smoke COMMAND mblink-runtime-smoke)
    add_test(NAME mblink-isotp-smoke COMMAND mblink-isotp-smoke)
    add_test(NAME mblink-isotp-failed-state-smoke
             COMMAND mblink-isotp-failed-state-smoke)
    add_test(NAME mblink-isotp-functional-response-smoke
             COMMAND mblink-isotp-functional-response-smoke)
endif()
