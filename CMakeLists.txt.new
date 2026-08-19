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

option(MBLINK_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(MBLINK_BUILD_LINUX_APP "Build the native GTK4 Linux front end" OFF)

if(MBLINK_ENABLE_SANITIZERS)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        message(FATAL_ERROR "MBLINK_ENABLE_SANITIZERS requires GCC or Clang.")
    endif()
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

include(CTest)

set(MBLINK_COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/infiltratr-common")
set(MBLINK_COMMON_VERSION "1.7.0")
set(MBLINK_COMMON_COMMIT "a5b52f7ad62fcbb3462c496b7831e7f974161bcf")

if(NOT EXISTS "${MBLINK_COMMON_DIR}/VERSION")
    message(FATAL_ERROR
        "Infiltratr Common ${MBLINK_COMMON_VERSION} is required. Clone with --recurse-submodules or run git submodule update --init --recursive.")
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
    src/core/parameter.c
    src/core/scheduler.c
    src/core/telemetry_store.c
    src/core/telemetry_csv.c
    src/core/isotp.c
    src/elm327/elm327.c
    src/elm327/session.c
    src/elm327/probe.c
    src/obd2/obd2.c
    src/uds/uds.c
    src/mercedes/mercedes.c)
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

if(MBLINK_BUILD_LINUX_APP)
    if(NOT UNIX OR APPLE)
        message(FATAL_ERROR "MBLINK_BUILD_LINUX_APP is supported on Linux only.")
    endif()

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4>=4.6)

    add_executable(mblink-linux app/linux/main.c)
    target_link_libraries(mblink-linux PRIVATE mblink PkgConfig::GTK4)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(mblink-linux PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
    endif()
endif()

function(mblink_add_test target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE mblink)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(${target} PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
    endif()
    add_test(NAME ${target} COMMAND ${target})
endfunction()

if(BUILD_TESTING)
    mblink_add_test(mblink-test-core tests/test_core.c)
    target_compile_definitions(mblink-test-core PRIVATE
        MBLINK_TEST_EXPECTED_VERSION="${PROJECT_VERSION}")

    mblink_add_test(mblink-test-elm327 tests/test_elm327.c)
    mblink_add_test(mblink-test-elm327-session tests/test_elm327_session.c)
    mblink_add_test(mblink-test-elm327-probe tests/test_elm327_probe.c)
    mblink_add_test(mblink-test-obd2 tests/test_obd2.c)
    mblink_add_test(mblink-test-runtime tests/test_runtime.c)
    mblink_add_test(mblink-test-parameter tests/test_parameter.c)
    mblink_add_test(mblink-test-isotp tests/test_isotp.c)
    mblink_add_test(mblink-test-isotp-failed-state
        tests/test_isotp_failed_state.c)
    mblink_add_test(mblink-test-isotp-functional-response
        tests/test_isotp_functional_response.c)
    mblink_add_test(mblink-test-uds tests/test_uds.c)
    mblink_add_test(mblink-test-mercedes tests/test_mercedes.c)
endif()
