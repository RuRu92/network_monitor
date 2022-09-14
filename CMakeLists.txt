cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(network_monitor)

# Add the local CMake modules folder to the CMake search path.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Dependencies
# If dependencies are managed with conan, we prepend the current build folder
# to the CMake module path, where CMake looks for dependencies.

if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conaninfo.txt)
    message("EXISTS")
    list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}/")
endif()

Set(Boost_INSTALL_DIR ${CONAN_BOOST_ROOT})
find_package(Boost 1.79.0 REQUIRED COMPONENTS system unit_test_framework)
find_package(Filesystem REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)

# Called before any other target is defined.
enable_testing()

# On Windows, we define a preprocessor symbol with the OS version to prevent
# warnings from the Boost.Asio header files.
if(WIN32)
    message("IS WIN32")
    include(GetWinVer REQUIRED)
    get_win_ver(WINDOWS_VERSION)
endif()

#set(SOURCES "playground/main.cpp")

# Static library
set(LIB_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/websocket-client.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/http-client.cpp"
        "inc/network-monitor/file_downloader.hpp"
        )
add_library(network_monitor STATIC ${LIB_SOURCES})

#add_executable(network_monitor ${SOURCES})
target_compile_features(network_monitor
    PUBLIC
        cxx_std_14
)

message(${WINDOWS_VERSION})
target_compile_definitions(network_monitor
        PRIVATE
        $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=${WINDOWS_VERSION}>
)

target_compile_definitions(network_monitor
        PRIVATE
        TESTS_CACERT_PEM="${CMAKE_CURRENT_SOURCE_DIR}/test/cacert.pem"
        TESTS_NETWORK_LAYOUT_JSON="${CMAKE_CURRENT_SOURCE_DIR}/resources/network-layout.json"
        )

target_compile_options(network_monitor
        PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)

target_include_directories(network_monitor
        PUBLIC
            inc
)

target_link_libraries(network_monitor
        PUBLIC
        Boost::Boost
        nlohmann_json::nlohmann_json
        OpenSSL::OpenSSL
)

#target_compile_definitions(network_monitor
#        PRIVATE
#        BOOST_ASIO_ENABLE_HANDLER_TRACKING=1
#)

#target_include_directories(network_monitor
#    PRIVATE
#        src/
#        inc/network-monitor/
#)


# Tests
set(TESTS_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/test/websocket-client.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/test/http-client.cpp"
)

add_executable(network-monitor-tests ${TESTS_SOURCES})
target_compile_features(network-monitor-tests
    PRIVATE
        cxx_std_17
)
target_compile_definitions(network-monitor-tests
        PRIVATE
            $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=${WINDOWS_VERSION}>
)
message("cert path...")
message(${CMAKE_CURRENT_SOURCE_DIR}/test/cacert.pem)
target_compile_definitions(network-monitor-tests
        PRIVATE
        TESTS_CACERT_PEM="${CMAKE_CURRENT_SOURCE_DIR}/test/cacert.pem"
        TESTS_NETWORK_LAYOUT_JSON="${CMAKE_CURRENT_SOURCE_DIR}/resources/network-layout.json"
        )
target_link_libraries(network-monitor-tests
   PRIVATE
        network_monitor
        Boost::Boost
        nlohmann_json::nlohmann_json
        OpenSSL::OpenSSL
        std::filesystem
)

add_test(
        NAME network-monitor-tests
        COMMAND $<TARGET_FILE:network-monitor-tests>)

# This tells CMake to check for a specific output to verify the test outcome.
# When all unit tests pass, Boost.Test prints "No errors detected".
set_tests_properties(network-monitor-tests PROPERTIES
        PASS_REGULAR_EXPRESSION ".*No errors detected"
)
# set (EXCLUDE_DIR "src/playground/")
# file (GLOB_RECURSE SOURCE_FILES "*.cpp" "*.c", "*.hpp")
# foreach (TMP_PATH ${SOURCE_FILES})
#     string (FIND ${TMP_PATH} ${EXCLUDE_DIR} EXCLUDE_DIR_FOUND)
#     if (NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
#         list (REMOVE_ITEM SOURCE_FILES ${TMP_PATH})
#     endif ()
# endforeach(TMP_PATH)

