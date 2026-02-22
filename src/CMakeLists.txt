cmake_minimum_required(VERSION 3.30)

cpmaddpackage(
    NAME
    gmod_module_base
    GITHUB_REPOSITORY
    Facepunch/gmod-module-base
    GIT_TAG
    master
    SYSTEM
    ON
    DOWNLOAD_ONLY
    YES)

# GMod expects: gmsv_<name>_<platform>.dll / gmcl_<name>_<platform>.dll
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(GMOD_PLAT "win64")
    else()
        set(GMOD_PLAT "win32")
    endif()
    set(GMOD_SUFFIX ".dll")
elseif(APPLE)
    set(GMOD_PLAT "osx64")
    set(GMOD_SUFFIX ".dll") # GMod convention
elseif(UNIX)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(GMOD_PLAT "linux64")
    else()
        set(GMOD_PLAT "linux")
    endif()
    set(GMOD_SUFFIX ".dll") # GMod convention — even on Linux
endif()

add_library(gmod_tracy SHARED)

set(build_example ON)

if(build_example)
    target_sources(
        gmod_tracy
        PRIVATE "${gmod_module_base_SOURCE_DIR}/example/src/gm_example.cpp")
else()
    target_sources(gmod_tracy PRIVATE "gmod_tracy.cpp")
endif()

target_include_directories(gmod_tracy SYSTEM
                           PRIVATE "${gmod_module_base_SOURCE_DIR}/include")

target_link_libraries(
    gmod_tracy
    PRIVATE Tracy::TracyClient spdlog
            # warnings
            Microsoft.GSL::GSL)
target_compile_definitions(gmod_tracy PRIVATE TRACY_ENABLE GMMODULE)

set(gmod_prefix "gmsv")
set_target_properties(
    gmod_tracy
    PROPERTIES CXX_STANDARD 26
               CXX_STANDARD_REQUIRED YES
               CXX_EXTENSIONS NO
               OUTPUT_NAME "${gmod_prefix}_tracy_${GMOD_PLAT}"
               PREFIX ""
               SUFFIX "${GMOD_SUFFIX}"
               POSITION_INDEPENDENT_CODE ON)

# Hide all symbols except gmod13_open / gmod13_close
if(NOT WIN32)
    target_compile_options(gmod_tracy PRIVATE -fvisibility=hidden)
endif()

