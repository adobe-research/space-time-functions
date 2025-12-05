if (TARGET yaml-cpp::yaml-cpp)
    return()
endif()

# First try to find system yaml-cpp
find_package(yaml-cpp QUIET)
if (yaml-cpp_FOUND)
    message(STATUS "Found system yaml-cpp")
    return()
endif()

# Try pkg-config
find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
    pkg_check_modules(YAML_CPP QUIET yaml-cpp)
    if (YAML_CPP_FOUND)
        add_library(yaml-cpp::yaml-cpp INTERFACE IMPORTED)
        target_link_libraries(yaml-cpp::yaml-cpp INTERFACE ${YAML_CPP_LIBRARIES})
        target_include_directories(yaml-cpp::yaml-cpp INTERFACE ${YAML_CPP_INCLUDE_DIRS})
        target_compile_options(yaml-cpp::yaml-cpp INTERFACE ${YAML_CPP_CFLAGS_OTHER})
        message(STATUS "Found yaml-cpp via pkg-config")
        return()
    endif()
endif()

message(STATUS "Third-party (external): creating target 'yaml-cpp::yaml-cpp'")

include(CPM)
CPMAddPackage(
    NAME yaml-cpp
    GITHUB_REPOSITORY jbeder/yaml-cpp
    GIT_TAG yaml-cpp-0.7.0
    OPTIONS
        "YAML_CPP_BUILD_TESTS OFF"
        "YAML_CPP_BUILD_TOOLS OFF"
        "YAML_CPP_BUILD_CONTRIB OFF"
)
