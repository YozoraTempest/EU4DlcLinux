cmake_minimum_required(VERSION 3.20)
project(EU4DlcLinux VERSION 0.1.0 LANGUAGES CXX)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8
   OR NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
    message(FATAL_ERROR "EU4DlcLinux targets native x86_64 Linux with glibc.")
endif()

find_package(Threads REQUIRED)
add_library(Creamlinux SHARED main.cpp src/runtime.cpp)
target_compile_features(Creamlinux PRIVATE cxx_std_14)
target_include_directories(Creamlinux PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(Creamlinux PRIVATE ${CMAKE_DL_LIBS} Threads::Threads)
set_target_properties(Creamlinux PROPERTIES
    PREFIX lib64
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
configure_file(package/cream.sh lib/cream.sh COPYONLY)
configure_file(package/cream_api.ini lib/cream_api.ini COPYONLY)
install(TARGETS Creamlinux LIBRARY DESTINATION .)
install(PROGRAMS package/cream.sh DESTINATION .)
install(FILES package/cream_api.ini README.md LICENSE THIRD_PARTY_NOTICES.md DESTINATION .)
install(FILES docs/MIGRATION.md docs/VALIDATION.md docs/archive-manifest.json docs/UPSTREAM_README.md DESTINATION docs)

include(CTest)
if(BUILD_TESTING)
    add_executable(config_tests tests/config_tests.cpp)
    target_compile_features(config_tests PRIVATE cxx_std_14)
    target_include_directories(config_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME config COMMAND config_tests "${CMAKE_CURRENT_SOURCE_DIR}/package/cream_api.ini")

    add_library(mock_steam SHARED tests/mock_steam.cpp)
    target_include_directories(mock_steam PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    add_executable(runtime_tests tests/runtime_tests.cpp)
    target_compile_features(runtime_tests PRIVATE cxx_std_14)
    target_include_directories(runtime_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(runtime_tests PRIVATE mock_steam ${CMAKE_DL_LIBS})
    add_test(NAME runtime COMMAND runtime_tests "$<TARGET_FILE:mock_steam>")
    set_tests_properties(runtime PROPERTIES ENVIRONMENT
        "LD_PRELOAD=$<TARGET_FILE:Creamlinux>;CREAM_CONFIG_PATH=${CMAKE_CURRENT_SOURCE_DIR}/package/cream_api.ini")
    add_test(NAME missing_config COMMAND runtime_tests --bad-config)
    set_tests_properties(missing_config PROPERTIES ENVIRONMENT
        "LD_PRELOAD=$<TARGET_FILE:Creamlinux>;CREAM_CONFIG_PATH=${CMAKE_CURRENT_BINARY_DIR}/missing.ini")
    add_test(NAME launcher COMMAND sh "${CMAKE_CURRENT_SOURCE_DIR}/tests/launcher_tests.sh")

    add_library(mock_overlay SHARED tests/mock_overlay.cpp)
    target_link_libraries(mock_overlay PRIVATE ${CMAKE_DL_LIBS})
    add_executable(loader_tests tests/loader_tests.cpp)
    target_link_libraries(loader_tests PRIVATE ${CMAKE_DL_LIBS})
    add_test(NAME preload_order COMMAND sh "${CMAKE_CURRENT_SOURCE_DIR}/tests/preload_order_tests.sh"
        "${CMAKE_CURRENT_SOURCE_DIR}/package/cream.sh" "$<TARGET_FILE:Creamlinux>"
        "$<TARGET_FILE:mock_steam>" "$<TARGET_FILE:mock_overlay>" "$<TARGET_FILE:loader_tests>")
    set_tests_properties(preload_order PROPERTIES TIMEOUT 20)

    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_executable(plugin_fixture tests/plugin_fixture.cpp)
    target_include_directories(plugin_fixture PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(plugin_fixture PRIVATE ${CMAKE_DL_LIBS})
    add_test(NAME plugin_package COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/plugin_package_tests.py"
        "$<TARGET_FILE:Creamlinux>" "$<TARGET_FILE:mock_steam>"
        "$<TARGET_FILE:plugin_fixture>" "${CMAKE_CURRENT_SOURCE_DIR}/package/cream_api.ini")
endif()
