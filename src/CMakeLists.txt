cmake_minimum_required(VERSION 3.30)

# TODO: add conviniet launch command:
# GLIBC_TUNABLES=glibc.rtld.optional_static_tls=1024 steam -applaunch 4000 -console +map gm_flatgrass -noworkshop -noreplay

# steamapps/common/GarrysMod
# ❯ gdb ./bin/linux64/srcds
# (gdb) set environment LD_LIBRARY_PATH ./bin/linux64
# (gdb) run -game garrysmod +map gm_construct +sv_hibernate_think 1

cpmaddpackage(
    NAME
    gmod_module_base
    GITHUB_REPOSITORY
    Facepunch/gmod-module-base
    GIT_TAG
    development
    GIT_SHALLOW
    TRUE)

add_library(gmod_tracy SHARED "gmod_tracy.cpp")

target_link_libraries(
    gmod_tracy
    PRIVATE #warnings
            gmod-module-base
            Tracy::TracyClient
            spdlog
            Microsoft.GSL::GSL)

target_compile_definitions(gmod_tracy PRIVATE TRACY_ENABLE)

set_target_properties(
    gmod_tracy
    PROPERTIES CXX_STANDARD 26
               CXX_STANDARD_REQUIRED YES
               CXX_EXTENSIONS NO
               POSITION_INDEPENDENT_CODE ON)

set(gmod_tracy_realm "gmcl" CACHE STRING
                                  "GMod module realm prefix (gmsv or gmcl)")
set_property(CACHE gmod_tracy_realm PROPERTY STRINGS "gmsv" "gmcl")

set_gmod_suffix_prefix(gmod_tracy)

set(GMOD_LUA_DIR
    "/run/media/eugene/lib/SteamLibrary/steamapps/common/GarrysMod/garrysmod/lua"
    CACHE PATH "Path to garrysmod/lua inside your GMod installation")

install(TARGETS gmod_tracy LIBRARY DESTINATION "${GMOD_LUA_DIR}/bin"
        RUNTIME DESTINATION "${GMOD_LUA_DIR}/bin")

install(FILES "${CMAKE_CURRENT_LIST_DIR}/tracy_loader.lua"
        DESTINATION "${GMOD_LUA_DIR}/autorun/${gmod_autorun_subfolder}")

add_custom_target(
    symlink_gmod
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GMOD_LUA_DIR}/bin"
    COMMAND
        ${CMAKE_COMMAND} -E rm -f
        "${GMOD_LUA_DIR}/bin/$<TARGET_FILE_NAME:gmod_tracy>"
    COMMAND
        ${CMAKE_COMMAND} -E create_symlink "$<TARGET_FILE:gmod_tracy>"
        "${GMOD_LUA_DIR}/bin/$<TARGET_FILE_NAME:gmod_tracy>"
    COMMAND
        ${CMAKE_COMMAND} -E make_directory
        "${GMOD_LUA_DIR}/autorun/${gmod_autorun_subfolder}"
    COMMAND
        ${CMAKE_COMMAND} -E rm -f
        "${GMOD_LUA_DIR}/autorun/${gmod_autorun_subfolder}/tracy_loader.lua"
    COMMAND
        ${CMAKE_COMMAND} -E create_symlink
        "${CMAKE_CURRENT_LIST_DIR}/tracy_loader.lua"
        "${GMOD_LUA_DIR}/autorun/${gmod_autorun_subfolder}/tracy_loader.lua"
    # TARGET_FILE genex in COMMAND implicitly adds dep on gmod_tracy
    COMMENT "symlink_gmod => ${GMOD_LUA_DIR}"
    VERBATIM)

add_custom_target(
    launch_gmod
    COMMAND
        ${CMAKE_COMMAND} -E env
        "GLIBC_TUNABLES=glibc.rtld.optional_static_tls=1024" steam -applaunch
        4000 -console +map gm_flatgrass -noworkshop -noreplay
    COMMENT "launch_gmod => steam -applaunch 4000"
    VERBATIM)
