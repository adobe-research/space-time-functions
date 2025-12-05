cmake_minimum_required(VERSION 3.28)

project(stf)

option(STF_BUILD_TESTS "Build tests" OFF)
option(STF_PYTHON_BINDING "Build python binding" OFF)
option(STF_YAML_PARSER "Build YAML parser support" ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# Create main library
if (STF_YAML_PARSER)
    # Add yaml-cpp dependency
    include(yaml-cpp)
    
    # Create library with source files
    file(GLOB STF_SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
    add_library(stf ${STF_SOURCES})
    target_include_directories(stf PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    target_compile_features(stf PUBLIC cxx_std_20)
    target_link_libraries(stf PUBLIC yaml-cpp::yaml-cpp)
    target_compile_definitions(stf PUBLIC STF_YAML_PARSER_ENABLED)
else()
    # Header-only library without YAML support
    add_library(stf INTERFACE)
    target_include_directories(stf INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    target_compile_features(stf INTERFACE cxx_std_20)
endif()

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

if (STF_PYTHON_BINDING)
    include(nanobind)
    set(PY_SRC_FILE ${CMAKE_CURRENT_LIST_DIR}/python/pystf.cpp)
    nanobind_add_module(pystf NB_STATIC ${PY_SRC_FILE})
    target_link_libraries(pystf PRIVATE stf::stf)
    target_compile_features(pystf PRIVATE cxx_std_20)
    install(TARGETS pystf LIBRARY
        COMPONENT STF_Python_Runtime
        DESTINATION ${SKBUILD_PLATLIB_DIR}/space_time_functions)
    add_library(stf::pystf ALIAS pystf)
endif()
