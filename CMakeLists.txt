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
if(WIN32)
    set(MBLINK_WINDOWS_DISCOVER_DEFAULT ON)
else()
    set(MBLINK_WINDOWS_DISCOVER_DEFAULT OFF)
endif()
option(MBLINK_BUILD_WINDOWS_DISCOVER "Build the Win32 OpenPort/J2534 discovery GUI" ${MBLINK_WINDOWS_DISCOVER_DEFAULT})

if(MBLINK_ENABLE_SANITIZERS)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        message(FATAL_ERROR "MBLINK_ENABLE_SANITIZERS requires GCC or Clang.")
    endif()
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

include(CTest)
include(GNUInstallDirs)

set(MBLINK_COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/infiltratr-common")
set(MBLINK_COMMON_VERSION "1.10.0")
set(MBLINK_COMMON_COMMIT "182e64cb8b8992879e443b941565058166fe0161")

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

set(INFILTRATR_COMMON_BUILD_TESTS OFF CACHE BOOL
    "Build Infiltratr Common contract and smoke tests" FORCE)
set(INFILTRATR_COMMON_WARNINGS_AS_ERRORS ON CACHE BOOL
    "Treat Infiltratr Common compiler warnings as errors" FORCE)
add_subdirectory(
    "${MBLINK_COMMON_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/infiltratr-common"
    EXCLUDE_FROM_ALL)

set(MBLINK_LINK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/link")
set(MBLINK_LINK_VERSION "0.6.0")
set(MBLINK_LINK_COMMIT "bbb21785983f1d94483949d4a7c50adf80f4c4e7")

if(NOT EXISTS "${MBLINK_LINK_DIR}/VERSION")
    message(FATAL_ERROR
        "LINK ${MBLINK_LINK_VERSION} is required. Clone with --recurse-submodules or run git submodule update --init --recursive.")
endif()

file(READ "${MBLINK_LINK_DIR}/VERSION" MBLINK_LINK_FOUND_VERSION)
string(STRIP "${MBLINK_LINK_FOUND_VERSION}" MBLINK_LINK_FOUND_VERSION)
if(NOT MBLINK_LINK_FOUND_VERSION STREQUAL MBLINK_LINK_VERSION)
    message(FATAL_ERROR
        "LINK ${MBLINK_LINK_VERSION} is required; found ${MBLINK_LINK_FOUND_VERSION}.")
endif()

if(MBLINK_GIT_EXECUTABLE)
    execute_process(
        COMMAND "${MBLINK_GIT_EXECUTABLE}" -C "${MBLINK_LINK_DIR}" rev-parse HEAD
        OUTPUT_VARIABLE MBLINK_LINK_FOUND_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE MBLINK_LINK_COMMIT_RESULT)
    if(MBLINK_LINK_COMMIT_RESULT EQUAL 0 AND
       NOT MBLINK_LINK_FOUND_COMMIT STREQUAL MBLINK_LINK_COMMIT)
        message(FATAL_ERROR
            "LINK must be pinned to ${MBLINK_LINK_COMMIT}; found ${MBLINK_LINK_FOUND_COMMIT}.")
    endif()
endif()

add_subdirectory(
    "${MBLINK_LINK_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/link"
    EXCLUDE_FROM_ALL)

add_library(mblink STATIC
    src/core/mblink.c
    src/core/parameter.c
    src/core/scheduler.c
    src/core/telemetry_store.c
    src/core/telemetry_csv.c
    src/elm327/elm327.c
    src/elm327/can.c
    src/elm327/session.c
    src/elm327/probe.c
    src/obd2/obd2.c
    src/uds/uds.c
    src/mercedes/mercedes.c
    src/mercedes/probe.c)
target_include_directories(mblink
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(mblink PRIVATE
    MBLINK_VERSION="${PROJECT_VERSION}")
target_link_libraries(mblink PUBLIC InfiltratrCommon::Portable LINK::Core)

set(MBLINK_WARNING_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wformat=2
    -Wstrict-prototypes
    -Wmissing-prototypes)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(mblink PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
endif()

if(MBLINK_BUILD_LINUX_APP)
    if(NOT UNIX OR APPLE)
        message(FATAL_ERROR "MBLINK_BUILD_LINUX_APP is supported on Linux only.")
    endif()

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4>=4.6)
    find_program(GLIB_COMPILE_RESOURCES_EXECUTABLE
        NAMES glib-compile-resources REQUIRED)

    set(MBLINK_LINUX_RESOURCE_SOURCE
        "${CMAKE_CURRENT_BINARY_DIR}/mblink-linux-resources.c")
    add_custom_command(
        OUTPUT "${MBLINK_LINUX_RESOURCE_SOURCE}"
        COMMAND "${GLIB_COMPILE_RESOURCES_EXECUTABLE}"
            --generate-source
            "--target=${MBLINK_LINUX_RESOURCE_SOURCE}"
            mblink.gresource.xml
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/app/linux"
        DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/app/linux/mblink.gresource.xml"
            "${CMAKE_CURRENT_SOURCE_DIR}/app/linux/assets/mblink-emblem.png"
        VERBATIM)

    add_executable(mblink-linux
        app/linux/main.c
        "${MBLINK_LINUX_RESOURCE_SOURCE}")
    set_source_files_properties("${MBLINK_LINUX_RESOURCE_SOURCE}"
        PROPERTIES COMPILE_OPTIONS "-w")
    target_link_libraries(mblink-linux PRIVATE mblink PkgConfig::GTK4)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(mblink-linux PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
    endif()

    install(TARGETS mblink-linux
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(FILES app/linux/assets/mblink-emblem.png
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/180x180/apps"
        RENAME mblink.png)
    install(FILES app/linux/com.github.The-First-Infiltrator.MBLINK.desktop
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
    install(FILES LICENSE README.md
        DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/mblink")

    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME "mblink")
    set(CPACK_PACKAGE_VENDOR "The First Infiltrator")
    set(CPACK_PACKAGE_CONTACT "Shannon Smith")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
        "Portable Mercedes-oriented vehicle diagnostics")
    set(CPACK_PACKAGE_HOMEPAGE_URL
        "https://github.com/The-First-Infiltrator/MBLINK")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_DEBIAN_PACKAGE_NAME "mblink")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    include(CPack)
endif()

if(MBLINK_BUILD_WINDOWS_DISCOVER)
    if(NOT WIN32)
        message(FATAL_ERROR "MBLINK_BUILD_WINDOWS_DISCOVER is supported on Windows only.")
    endif()
    link_add_windows_discover(mblink-discover
        PRODUCT_NAME "MBLINK"
        PRODUCT_SLUG "mblink"
        WINDOW_CLASS "MBLINKDiscoverWindow")
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
    mblink_add_test(mblink-test-elm327-can tests/test_elm327_can.c)
    mblink_add_test(mblink-test-elm327-session tests/test_elm327_session.c)
    mblink_add_test(mblink-test-elm327-probe tests/test_elm327_probe.c)
    mblink_add_test(mblink-test-obd2 tests/test_obd2.c)
    mblink_add_test(mblink-test-runtime tests/test_runtime.c)
    mblink_add_test(mblink-test-parameter tests/test_parameter.c)
    mblink_add_test(mblink-test-parameter-store tests/test_parameter_store.c)
    mblink_add_test(mblink-test-scheduler-parameters tests/test_scheduler_parameters.c)
    mblink_add_test(mblink-test-isotp tests/test_isotp.c)
    mblink_add_test(mblink-test-isotp-failed-state
        tests/test_isotp_failed_state.c)
    mblink_add_test(mblink-test-isotp-functional-response
        tests/test_isotp_functional_response.c)
    mblink_add_test(mblink-test-uds tests/test_uds.c)
    mblink_add_test(mblink-test-uds-dtc tests/test_uds_dtc.c)
    mblink_add_test(mblink-test-mercedes tests/test_mercedes.c)
    mblink_add_test(mblink-test-mercedes-crd3 tests/test_mercedes_crd3.c)
    mblink_add_test(mblink-test-mercedes-om651 tests/test_mercedes_om651.c)
    mblink_add_test(mblink-test-mercedes-probe tests/test_mercedes_probe.c)
    mblink_add_test(mblink-test-mercedes-trace-replay
        tests/test_mercedes_trace_replay.c)
    mblink_add_test(mblink-test-mercedes-engine-scan
        tests/test_mercedes_engine_scan.c)
endif()
