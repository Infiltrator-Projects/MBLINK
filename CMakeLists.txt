# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.20)
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()
if(MSVC OR WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" MBLINK_PROJECT_VERSION)
string(STRIP "${MBLINK_PROJECT_VERSION}" MBLINK_PROJECT_VERSION)
if(NOT MBLINK_PROJECT_VERSION MATCHES "^[0-9]+\.[0-9]+\.[0-9]+$")
    message(FATAL_ERROR "VERSION must contain a semantic major.minor.patch version.")
endif()
project(MBLINK VERSION "${MBLINK_PROJECT_VERSION}" LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Canonical MBLINK typography. The exact font archive is a required source asset.
# It contains all three original MB Corpo TTF files supplied for the project.
set(MBLINK_FONT_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/mb-corpo-fonts.tar.xz")
if(NOT EXISTS "${MBLINK_FONT_ARCHIVE}")
    message(FATAL_ERROR "Required MBLINK MB Corpo font archive is missing: ${MBLINK_FONT_ARCHIVE}")
endif()
set(MBLINK_HAVE_BUNDLED_FONTS TRUE)
set(MBLINK_FONT_DIR "")
set(MBLINK_FONT_FILES "")
file(SHA256 "${MBLINK_FONT_ARCHIVE}" MBLINK_FONT_ARCHIVE_SHA256)
    if(NOT MBLINK_FONT_ARCHIVE_SHA256 STREQUAL
       "bdb6063f838a7fab22b4d6b412170640c69511df53aa3dfa9a4ea8431c9d8274")
        message(FATAL_ERROR "MBLINK font archive hash mismatch.")
    endif()
    set(MBLINK_FONT_DIR "${CMAKE_CURRENT_BINARY_DIR}/mblink-fonts")
    file(REMOVE_RECURSE "${MBLINK_FONT_DIR}")
    file(MAKE_DIRECTORY "${MBLINK_FONT_DIR}")
    file(ARCHIVE_EXTRACT INPUT "${MBLINK_FONT_ARCHIVE}" DESTINATION "${MBLINK_FONT_DIR}")
    set(MBLINK_FONT_A_COND_REGULAR "${MBLINK_FONT_DIR}/mb_corpo_a_cond_regular.ttf")
    set(MBLINK_FONT_S_BOLD "${MBLINK_FONT_DIR}/mb_corpo_s_bold.ttf")
    set(MBLINK_FONT_S_REGULAR "${MBLINK_FONT_DIR}/mb_corpo_s_regular.ttf")
    set(MBLINK_FONT_FILES
        "${MBLINK_FONT_A_COND_REGULAR}"
        "${MBLINK_FONT_S_BOLD}"
        "${MBLINK_FONT_S_REGULAR}")
    set(MBLINK_FONT_EXPECTED_SHA256
        "c8bcd7e1a7d71169b38491d9b7c1ffe7ba7b46e888f0c1219931343a47bc0e05"
        "d37ea986e2344d83390f94f170e6272b56efd00bfec808afe8314c4ca45d43b4"
        "94ede6629443c03d4362dcef425fb3ff520be5d654370021a34e81286804465c")
    list(LENGTH MBLINK_FONT_FILES MBLINK_FONT_COUNT)
    math(EXPR MBLINK_FONT_LAST "${MBLINK_FONT_COUNT} - 1")
    foreach(MBLINK_FONT_INDEX RANGE 0 ${MBLINK_FONT_LAST})
        list(GET MBLINK_FONT_FILES ${MBLINK_FONT_INDEX} MBLINK_FONT_FILE)
        list(GET MBLINK_FONT_EXPECTED_SHA256 ${MBLINK_FONT_INDEX} MBLINK_FONT_SHA256_EXPECTED)
        file(SHA256 "${MBLINK_FONT_FILE}" MBLINK_FONT_SHA256_ACTUAL)
        if(NOT MBLINK_FONT_SHA256_ACTUAL STREQUAL MBLINK_FONT_SHA256_EXPECTED)
            message(FATAL_ERROR "MBLINK font hash mismatch: ${MBLINK_FONT_FILE}")
        endif()
    endforeach()

option(MBLINK_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(MBLINK_BUILD_LINUX_APP "Build the native GTK4 Linux front end" OFF)
set(MBLINK_BUILD_PROFILE "source" CACHE STRING "Build identity: source, generic, or native")
set_property(CACHE MBLINK_BUILD_PROFILE PROPERTY STRINGS source generic native)
if(NOT MBLINK_BUILD_PROFILE MATCHES "^(source|generic|native)$")
    message(FATAL_ERROR "MBLINK_BUILD_PROFILE must be source, generic, or native.")
endif()
set(MBLINK_PACKAGE_VERSION "${PROJECT_VERSION}" CACHE STRING "Debian package version; native installers append +nativeN")
if(NOT MBLINK_PACKAGE_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+(\\+native[0-9]+)?$")
    message(FATAL_ERROR "MBLINK_PACKAGE_VERSION must be semantic version or semantic version+nativeN.")
endif()
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
set(MBLINK_LINK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/link")
if(NOT EXISTS "${MBLINK_LINK_DIR}/VERSION")
    message(FATAL_ERROR "LINK is required. Clone with --recurse-submodules or initialise submodules recursively.")
endif()
file(READ "${MBLINK_LINK_DIR}/VERSION" MBLINK_LINK_VERSION)
string(STRIP "${MBLINK_LINK_VERSION}" MBLINK_LINK_VERSION)
if(NOT MBLINK_LINK_VERSION MATCHES "^[0-9]+\.[0-9]+\.[0-9]+$")
    message(FATAL_ERROR "Pinned LINK VERSION is not semantic: ${MBLINK_LINK_VERSION}")
endif()
find_program(MBLINK_GIT_EXECUTABLE git)
set(MBLINK_SOURCE_REVISION "")
if(MBLINK_GIT_EXECUTABLE)
    execute_process(
        COMMAND "${MBLINK_GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_SOURCE_DIR}" rev-parse HEAD
        OUTPUT_VARIABLE MBLINK_SOURCE_REVISION_CANDIDATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE MBLINK_SOURCE_REVISION_RESULT)
    if(MBLINK_SOURCE_REVISION_RESULT EQUAL 0 AND
       MBLINK_SOURCE_REVISION_CANDIDATE MATCHES "^[0-9a-fA-F]{40}$")
        set(MBLINK_SOURCE_REVISION "${MBLINK_SOURCE_REVISION_CANDIDATE}")
    endif()
    execute_process(COMMAND "${MBLINK_GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_SOURCE_DIR}" ls-tree HEAD src/link OUTPUT_VARIABLE MBLINK_LINK_GITLINK OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET RESULT_VARIABLE MBLINK_LINK_GITLINK_RESULT)
    execute_process(COMMAND "${MBLINK_GIT_EXECUTABLE}" -C "${MBLINK_LINK_DIR}" rev-parse HEAD OUTPUT_VARIABLE MBLINK_LINK_FOUND_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET RESULT_VARIABLE MBLINK_LINK_COMMIT_RESULT)
    if(MBLINK_LINK_GITLINK_RESULT EQUAL 0 AND MBLINK_LINK_COMMIT_RESULT EQUAL 0)
        string(REGEX MATCH "^160000 commit ([0-9a-fA-F]+)" MBLINK_LINK_GITLINK_MATCH "${MBLINK_LINK_GITLINK}")
        if(NOT MBLINK_LINK_GITLINK_MATCH)
            message(FATAL_ERROR "Unable to resolve the LINK gitlink from the MBLINK commit.")
        endif()
        set(MBLINK_LINK_EXPECTED_COMMIT "${CMAKE_MATCH_1}")
        if(NOT MBLINK_LINK_FOUND_COMMIT STREQUAL MBLINK_LINK_EXPECTED_COMMIT)
            message(FATAL_ERROR "LINK checkout does not match the MBLINK gitlink: expected ${MBLINK_LINK_EXPECTED_COMMIT}, found ${MBLINK_LINK_FOUND_COMMIT}.")
        endif()
    endif()
endif()
add_subdirectory("${MBLINK_LINK_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/link" EXCLUDE_FROM_ALL)
if(TARGET link-core)
    target_compile_definitions(link-core PRIVATE
        MBLINK_VERSION="${PROJECT_VERSION}"
        MBLINK_BUILD_PROFILE="${MBLINK_BUILD_PROFILE}")
    if(MBLINK_SOURCE_REVISION)
        target_compile_definitions(link-core PRIVATE
            MBLINK_BUILD_REVISION="${MBLINK_SOURCE_REVISION}")
    endif()
endif()
add_library(mblink STATIC
    src/core/mblink.c src/core/parameter.c src/core/scheduler.c src/core/telemetry_store.c src/core/telemetry_csv.c
    src/embedded/console.c
    src/elm327/elm327.c src/elm327/can.c src/elm327/session.c src/elm327/probe.c
    src/obd2/obd2.c src/kwp2000/kwp2000.c src/uds/uds.c src/mercedes/mercedes.c src/mercedes/vin.c
    src/mercedes/server.c src/mercedes/probe.c src/mercedes/discover_plan.c src/mercedes/data_scan.c
    src/mercedes/signal_catalog.c)
target_include_directories(mblink PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(mblink PRIVATE MBLINK_VERSION="${PROJECT_VERSION}" MBLINK_BUILD_PROFILE="${MBLINK_BUILD_PROFILE}")
if(MBLINK_SOURCE_REVISION)
    target_compile_definitions(mblink PRIVATE
        MBLINK_BUILD_REVISION="${MBLINK_SOURCE_REVISION}")
endif()
target_link_libraries(mblink PUBLIC LINK::Core)
if(UNIX)
    target_link_libraries(mblink PUBLIC m)
endif()
set(MBLINK_WARNING_FLAGS -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wstrict-prototypes -Wmissing-prototypes)
add_executable(mblink-did-lab tools/mblink-did-lab.c)
target_link_libraries(mblink-did-lab PRIVATE mblink)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(mblink-did-lab PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
endif()
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(mblink PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
endif()
if(MBLINK_BUILD_LINUX_APP)
    if(NOT UNIX OR APPLE)
        message(FATAL_ERROR "MBLINK_BUILD_LINUX_APP is supported on Linux only.")
    endif()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4>=4.6)
    pkg_check_modules(FONTCONFIG REQUIRED IMPORTED_TARGET fontconfig)
    find_program(GLIB_COMPILE_RESOURCES_EXECUTABLE NAMES glib-compile-resources REQUIRED)
    set(MBLINK_LINUX_RESOURCE_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/mblink-linux-resources.c")
    add_custom_command(OUTPUT "${MBLINK_LINUX_RESOURCE_SOURCE}"
        COMMAND "${GLIB_COMPILE_RESOURCES_EXECUTABLE}" --generate-source "--target=${MBLINK_LINUX_RESOURCE_SOURCE}" mblink.gresource.xml
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/app/linux"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/app/linux/mblink.gresource.xml" "${CMAKE_CURRENT_SOURCE_DIR}/app/linux/assets/mblink-emblem.png" VERBATIM)
    add_executable(mblink-linux
        app/linux/main.c
        app/linux/session_trace.c
        app/linux/style.c
        app/linux/about-dialog.c
        app/linux/c207-replay.c
        "${MBLINK_LINUX_RESOURCE_SOURCE}")
    set_source_files_properties(
        app/linux/main.c
        app/linux/about-dialog.c
        app/linux/c207-replay.c
        PROPERTIES
        COMPILE_OPTIONS "-include;${MBLINK_LINK_DIR}/platform/linux/link-gtk-i18n.h")
    set_source_files_properties("${MBLINK_LINUX_RESOURCE_SOURCE}" PROPERTIES COMPILE_OPTIONS "-w")
    target_link_libraries(mblink-linux PRIVATE mblink PkgConfig::GTK4 PkgConfig::FONTCONFIG)
    target_compile_definitions(mblink-linux PRIVATE
        "MBLINK_FONT_BUILD_DIR=\"${MBLINK_FONT_DIR}\""
        "MBLINK_FONT_INSTALL_DIR=\"${CMAKE_INSTALL_FULL_DATADIR}/fonts/truetype/mblink\"")
    link_enable_linux_gtk_shell(mblink-linux UDEV_RULE_NAME "60-mblink-openport2.rules")
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(mblink-linux PRIVATE ${MBLINK_WARNING_FLAGS} -Werror)
    endif()
    install(TARGETS mblink-linux RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(FILES app/linux/assets/mblink-emblem.png DESTINATION "${CMAKE_INSTALL_DATADIR}/pixmaps" RENAME mblink.png)
    if(MBLINK_HAVE_BUNDLED_FONTS)
        install(FILES ${MBLINK_FONT_FILES}
            DESTINATION "${CMAKE_INSTALL_DATADIR}/fonts/truetype/mblink")
    endif()
    install(FILES app/linux/com.github.The-First-Infiltrator.MBLINK.desktop DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
    install(FILES LICENSE DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/mblink" RENAME copyright)
    install(FILES README.md DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/mblink")
    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME "mblink")
    set(CPACK_PACKAGE_VENDOR "The First Infiltrator")
    set(CPACK_PACKAGE_CONTACT "Shannon Smith")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Portable Mercedes-oriented vehicle diagnostics")
    set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Infiltrator-Projects/MBLINK")
    set(CPACK_PACKAGE_VERSION "${MBLINK_PACKAGE_VERSION}")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_DEBIAN_PACKAGE_NAME "mblink")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/packaging/debian/postinst.in"
        "${CMAKE_CURRENT_BINARY_DIR}/postinst" @ONLY)
    file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/postinst"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/postinst")
    include(CPack)
endif()
if(MBLINK_BUILD_WINDOWS_DISCOVER)
    if(NOT WIN32)
        message(FATAL_ERROR "MBLINK_BUILD_WINDOWS_DISCOVER is supported on Windows only.")
    endif()
    link_add_windows_discover(mblink-discover PRODUCT_NAME "MBLINK" PRODUCT_SLUG "mblink" WINDOW_CLASS "MBLINKDiscoverWindow"
        PRODUCT_VERSION "${PROJECT_VERSION}" PRODUCT_COPYRIGHT "Copyright (C) 2026 Shannon Smith"
        PRODUCT_SUBTITLE "Mercedes-Benz Diagnostics" PRODUCT_WEBSITE "https://github.com/Infiltrator-Projects/MBLINK"
        PRODUCT_ICON_PNG "${CMAKE_CURRENT_SOURCE_DIR}/app/ios/MBLINK/Assets.xcassets/AppIcon.appiconset/AppIcon-60@3x.png"
        FULL_SWEEP_PLAN_FUNCTION mblink_discover_full_sweep_plan)
    target_link_libraries(mblink-discover PRIVATE mblink)
    if(MBLINK_HAVE_BUNDLED_FONTS)
        file(TO_NATIVE_PATH "${MBLINK_FONT_S_REGULAR}" MBLINK_FONT_S_REGULAR_RC)
        file(TO_NATIVE_PATH "${MBLINK_FONT_S_BOLD}" MBLINK_FONT_S_BOLD_RC)
        file(TO_NATIVE_PATH "${MBLINK_FONT_A_COND_REGULAR}" MBLINK_FONT_A_COND_REGULAR_RC)
        string(REPLACE "\\" "\\\\" MBLINK_FONT_S_REGULAR_RC "${MBLINK_FONT_S_REGULAR_RC}")
        string(REPLACE "\\" "\\\\" MBLINK_FONT_S_BOLD_RC "${MBLINK_FONT_S_BOLD_RC}")
        string(REPLACE "\\" "\\\\" MBLINK_FONT_A_COND_REGULAR_RC "${MBLINK_FONT_A_COND_REGULAR_RC}")
        set(MBLINK_WINDOWS_FONT_RC "${CMAKE_CURRENT_BINARY_DIR}/mblink-fonts.rc")
        file(WRITE "${MBLINK_WINDOWS_FONT_RC}"
            "1101 RCDATA \"${MBLINK_FONT_S_REGULAR_RC}\"\n"
            "1102 RCDATA \"${MBLINK_FONT_S_BOLD_RC}\"\n"
            "1103 RCDATA \"${MBLINK_FONT_A_COND_REGULAR_RC}\"\n")
        target_sources(mblink-discover PRIVATE "${MBLINK_WINDOWS_FONT_RC}")
        endif()
    if(MSVC)
        target_compile_options(mblink-discover PRIVATE "/FI${MBLINK_LINK_DIR}/platform/windows/link-discover-theme.h")
    endif()
    target_compile_definitions(mblink-discover PRIVATE
        LINK_THEME_BACKGROUND=0x00080605
        LINK_THEME_PANEL=0x00181310
        LINK_THEME_INPUT=0x0014100D
        LINK_THEME_TEXT=0x00EFECE8
        LINK_THEME_MUTED=0x00989189
        LINK_THEME_ACCENT=0x00E2DDD7
        LINK_THEME_ACCENT_TEXT=0x00181411
        "LINK_PRODUCT_FONT_UI=\"MB Corpo S Title WEB\""
        "LINK_PRODUCT_FONT_DISPLAY=\"MB Corpo A Title Cond WEB\""
        "LINK_PRODUCT_FONT_LOG=\"MB Corpo S Title WEB\""
        LINK_PRODUCT_FONT_RESOURCE_REGULAR=1101
        LINK_PRODUCT_FONT_RESOURCE_BOLD=1102
        LINK_PRODUCT_FONT_RESOURCE_CONDENSED=1103)
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
    target_compile_definitions(mblink-test-core PRIVATE MBLINK_TEST_EXPECTED_VERSION="${PROJECT_VERSION}")
    mblink_add_test(mblink-test-link-facade tests/test_link_facade.c)
    mblink_add_test(mblink-test-elm327 tests/test_elm327.c)
    mblink_add_test(mblink-test-elm327-can tests/test_elm327_can.c)
    mblink_add_test(mblink-test-elm327-session tests/test_elm327_session.c)
    mblink_add_test(mblink-test-elm327-probe tests/test_elm327_probe.c)
    mblink_add_test(mblink-test-obd2 tests/test_obd2.c)
    mblink_add_test(mblink-test-dtc-knowledge tests/test_dtc_knowledge.c)
    mblink_add_test(mblink-test-runtime tests/test_runtime.c)
    mblink_add_test(mblink-test-embedded-console tests/test_embedded_console.c)
    mblink_add_test(mblink-test-parameter tests/test_parameter.c)
    mblink_add_test(mblink-test-parameter-store tests/test_parameter_store.c)
    mblink_add_test(mblink-test-scheduler-parameters tests/test_scheduler_parameters.c)
    mblink_add_test(mblink-test-isotp tests/test_isotp.c)
    mblink_add_test(mblink-test-isotp-failed-state tests/test_isotp_failed_state.c)
    mblink_add_test(mblink-test-isotp-functional-response tests/test_isotp_functional_response.c)
    mblink_add_test(mblink-test-uds tests/test_uds.c)
    mblink_add_test(mblink-test-uds-dtc tests/test_uds_dtc.c)
    mblink_add_test(mblink-test-discover-plan tests/test_discover_plan.c)
    mblink_add_test(mblink-test-mercedes tests/test_mercedes.c)
    mblink_add_test(mblink-test-mercedes-server tests/test_mercedes_server.c)
    mblink_add_test(mblink-test-stm32-reporter tests/test_stm32_reporter.c)
    target_sources(mblink-test-stm32-reporter PRIVATE
        ${MBLINK_LINK_DIR}/platform/stm32/link-stm32-can.c
        ${MBLINK_LINK_DIR}/platform/stm32/link-stm32-uds-server.c)
    target_include_directories(mblink-test-stm32-reporter PRIVATE
        ${MBLINK_LINK_DIR}/platform/stm32)
    mblink_add_test(mblink-test-mercedes-vin tests/test_mercedes_vin.c)
    mblink_add_test(mblink-test-mercedes-crd3 tests/test_mercedes_crd3.c)
    mblink_add_test(mblink-test-mercedes-om651 tests/test_mercedes_om651.c)
    mblink_add_test(mblink-test-mercedes-did-lab tests/test_mercedes_did_lab.c)
    mblink_add_test(mblink-test-mercedes-signal-catalog tests/test_mercedes_signal_catalog.c)
    mblink_add_test(mblink-test-mercedes-probe tests/test_mercedes_probe.c)
    mblink_add_test(mblink-test-mercedes-module-scan tests/test_mercedes_module_scan.c)
    mblink_add_test(mblink-test-mercedes-data-scan tests/test_mercedes_data_scan.c)
    mblink_add_test(mblink-test-mercedes-trace-replay tests/test_mercedes_trace_replay.c)
    mblink_add_test(mblink-test-c207-obd-flow-replay tests/test_c207_obd_flow_replay.c)
    mblink_add_test(mblink-test-linux-session-trace tests/test_linux_session_trace.c)
    target_sources(mblink-test-linux-session-trace PRIVATE app/linux/session_trace.c)
    target_include_directories(mblink-test-linux-session-trace PRIVATE app/linux)
    mblink_add_test(mblink-test-mercedes-captured-deep-scan tests/test_mercedes_captured_deep_scan.c)
    mblink_add_test(mblink-test-mercedes-engine-scan tests/test_mercedes_engine_scan.c)
endif()
