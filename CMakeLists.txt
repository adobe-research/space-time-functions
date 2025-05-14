cmake_minimum_required(VERSION 3.28)

project(stf)

option(STF_BUILD_TESTS "Build tests" OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

add_library(stf INTERFACE)
target_include_directories(stf INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(stf INTERFACE cxx_std_20)
add_library(stf::stf ALIAS stf)

if (STF_BUILD_TESTS)
    include(CTest)
    enable_testing()
    include(catch2)

    file(GLOB TEST_FILES "${CMAKE_CURRENT_LIST_DIR}/tests/*.cpp")
    add_executable(stf_tests ${TEST_FILES})
    target_link_libraries(stf_tests
        PRIVATE
        stf::stf
        Catch2::Catch2WithMain
    )
    catch_discover_tests(stf_tests)
endif()
