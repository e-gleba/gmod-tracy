cmake_minimum_required(VERSION 3.30)

project(
    garrys_mod_tracy_integration
    VERSION 1.0.0
    DESCRIPTION "Garry's Mod Tracy profiler module"
    HOMEPAGE_URL "https://github.com/e-gleba/gmod-tracy"
    LANGUAGES C CXX)

list(
    APPEND
    CMAKE_PREFIX_PATH
    "${CMAKE_CURRENT_LIST_DIR}/cmake"
    "${CMAKE_CURRENT_LIST_DIR}/cmake/code_quality")

if(CMAKE_CROSSCOMPILING)
    list(
        APPEND
        CMAKE_FIND_ROOT_PATH
        "${CMAKE_CURRENT_LIST_DIR}/cmake"
        "${CMAKE_CURRENT_LIST_DIR}/cmake/code_quality")
endif()

find_package(warnings CONFIG REQUIRED)

find_package(clang_format CONFIG REQUIRED)
find_package(clang_tidy CONFIG REQUIRED)
find_package(clang_doc CONFIG REQUIRED)
find_package(cpplint CONFIG REQUIRED)
find_package(doxygen CONFIG REQUIRED)

find_package(cpm CONFIG REQUIRED)

add_subdirectory(src)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
