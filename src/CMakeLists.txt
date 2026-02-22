cmake_minimum_required(VERSION 3.30)

# gmod-module-base is header-only; download only, no build step.
# WARNING: GIT_TAG "master" is unpinned. Pin to a commit hash or tag
#          for reproducible builds (e.g. GIT_TAG "abc1234..." or a release tag).
#          See: https://cmake.org/cmake/help/latest/module/FetchContent.html

include(FetchContent)

fetchcontent_declare(
    gmod_module_base
    GIT_REPOSITORY https://github.com/Facepunch/gmod-module-base.git
    GIT_TAG master
    GIT_SHALLOW TRUE
    DOWNLOAD_ONLY TRUE)
fetchcontent_makeavailable(gmod_module_base)

# -- Platform detection (GMod naming convention) ---------------------------
# GMod expects: gmsv_<name>_<platform>.dll / gmcl_<name>_<platform>.dll
# GMod uses the .dll suffix on ALL platforms by convention.

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(gmod_platform "win64")
    else()
        set(gmod_platform "win32")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(gmod_platform "osx64")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(gmod_platform "linux64")
    else()
        set(gmod_platform "linux")
    endif()
else()
    message(
        FATAL_ERROR "Unsupported platform for GMod module: ${CMAKE_SYSTEM_NAME}"
        )
endif()

option(GMOD_TRACY_BUILD_EXAMPLE
       "Build the gmod-module-base example instead of gmod_tracy source" ON)

set(gmod_tracy_realm "gmsv" CACHE STRING
                                  "GMod module realm prefix (gmsv or gmcl)")
set_property(CACHE gmod_tracy_realm PROPERTY STRINGS "gmsv" "gmcl")

if(GMOD_TRACY_BUILD_EXAMPLE)
    add_library(gmod_example_module SHARED
                "${gmod_module_base_SOURCE_DIR}/example/src/gm_example.cpp")

    set_target_properties(
        gmod_example_module
        PROPERTIES CXX_STANDARD 26
                   CXX_STANDARD_REQUIRED YES
                   CXX_EXTENSIONS NO
                   OUTPUT_NAME "${gmod_tracy_realm}_example_${gmod_platform}"
                   PREFIX ""
                   SUFFIX ".dll"
                   POSITION_INDEPENDENT_CODE ON)

    target_include_directories(gmod_example_module SYSTEM
                               PRIVATE "${gmod_module_base_SOURCE_DIR}/include")

    target_compile_definitions(gmod_example_module PRIVATE GMMODULE)

    target_compile_options(
        gmod_example_module
        PRIVATE $<$<NOT:$<PLATFORM_ID:Windows>>:-fvisibility=hidden>)
endif()

add_library(gmod_tracy SHARED "gmod_tracy.cpp")

target_include_directories(gmod_tracy SYSTEM
                           PRIVATE "${gmod_module_base_SOURCE_DIR}/include")

target_link_libraries(
    gmod_tracy
    PRIVATE warnings
            Tracy::TracyClient
            spdlog
            Microsoft.GSL::GSL)

target_compile_definitions(gmod_tracy PRIVATE TRACY_ENABLE GMMODULE)

# Hide all symbols except gmod13_open / gmod13_close on non-Windows platforms.
# On Windows, symbols are hidden by default (MSVC dllexport model).
# Uses PLATFORM_ID generator expression instead of legacy if(NOT WIN32) guard.
target_compile_options(
    gmod_tracy PRIVATE $<$<NOT:$<PLATFORM_ID:Windows>>:-fvisibility=hidden>)

set_target_properties(
    gmod_tracy
    PROPERTIES CXX_STANDARD 26
               CXX_STANDARD_REQUIRED YES
               CXX_EXTENSIONS NO
               OUTPUT_NAME "${gmod_tracy_realm}_tracy_${gmod_platform}"
               PREFIX ""
               SUFFIX ".dll"
               POSITION_INDEPENDENT_CODE ON)