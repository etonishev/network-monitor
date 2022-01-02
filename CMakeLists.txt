cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

IF(MSVC)
    SET(CMAKE_CXX_FLAGS "/EHsc")
ENDIF(MSVC)

# Dependencies
# If dependencies are managed with conan, we prepend the current build folder
# to the CMake module path, where CMake looks for dependencies.
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conaninfo.txt)
    list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")
endif()
find_package(Boost 1.74 REQUIRED COMPONENTS system)

# Static library
set(LIB_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/websocket-client.cpp"
)

add_library(network-monitor STATIC ${LIB_SOURCES})

target_compile_features(network-monitor
    PRIVATE
        cxx_std_14
)

target_link_libraries(network-monitor
    PUBLIC
        Boost::Boost
)

target_include_directories(network-monitor
    PUBLIC
        inc/network-monitor
)

# Tests
set(TESTS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/websocket-client.cpp"
)
add_executable(network-monitor-tests ${TESTS_SOURCES})
target_compile_features(network-monitor-tests
    PRIVATE
        cxx_std_14
)
target_link_libraries(network-monitor-tests
    PRIVATE
        network-monitor
        Boost::Boost
)