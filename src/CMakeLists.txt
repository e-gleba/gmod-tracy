cmake_minimum_required(VERSION 3.30)

# TODO: fix it prettier somewhere, can be useful
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
    PRIVATE warnings
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

set_gmod_suffix_prefix(gmod_tracy)

# TODO: avoid hardcoded path
set(GMOD_LUA_DIR
    "/run/media/eugene/lib/SteamLibrary/steamapps/common/GarrysMod/garrysmod/lua"
    CACHE PATH "Path to garrysmod/lua inside your GMod installation")

install(TARGETS gmod_tracy LIBRARY DESTINATION "${GMOD_LUA_DIR}/bin"
        RUNTIME DESTINATION "${GMOD_LUA_DIR}/bin")

install(FILES "${CMAKE_CURRENT_LIST_DIR}/tracy_loader.lua"
        DESTINATION "${GMOD_LUA_DIR}/autorun/${gmod_autorun_subfolder}")

set(GMOD_MODULE_NAME "gmod_tracy"
    CACHE STRING "binary module name passed to require() in tracy_loader.lua")

configure_file("${CMAKE_CURRENT_LIST_DIR}/tracy_loader.lua.in"
               "${CMAKE_CURRENT_BINARY_DIR}/tracy_loader.lua" @ONLY)

set(loader_lua "${CMAKE_CURRENT_BINARY_DIR}/tracy_loader.lua")

# TODO: gmsv or gmcl? those are unused now
set(gmod_tracy_realm "gmcl" CACHE STRING
                                  "gmod module realm prefix (gmsv or gmcl)")
set_property(CACHE gmod_tracy_realm PROPERTY STRINGS "gmsv" "gmcl")

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
        ${CMAKE_COMMAND} -E rm -f
        "${GMOD_LUA_DIR}/autorun/server/tracy_loader.lua"
    COMMAND
        ${CMAKE_COMMAND} -E create_symlink "${loader_lua}"
        "${GMOD_LUA_DIR}/autorun/server/tracy_loader.lua"
    COMMENT "symlink_gmod => ${GMOD_LUA_DIR} (module=@${GMOD_MODULE_NAME}@)"
    DEPENDS ${loader_lua}
    VERBATIM)

add_dependencies(symlink_gmod gmod_tracy)

# TODO: use adequate launch command using user steam flags and etc but with
# imidiate map load, idk
# Also this is relevant to patched gmod x64
# See https://github.com/solsticegamestudios/GModPatchTool/issues/232#issuecomment-3938961378
add_custom_target(
    launch_gmod
    COMMAND
        ${CMAKE_COMMAND} -E env
        "GLIBC_TUNABLES=glibc.rtld.optional_static_tls=1024" steam -applaunch
        4000 -console +map gm_flatgrass -noworkshop -noreplay
    COMMENT "launch_gmod => steam -applaunch 4000"
    VERBATIM)

add_dependencies(launch_gmod symlink_gmod)
