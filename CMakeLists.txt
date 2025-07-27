cmake_minimum_required(VERSION 3.30.8)
project(stm C)
include(GNUInstallDirs)

set(CMAKE_C_STANDARD 23)
set(STM_VERSION_MAJOR 1)
set(STM_VERSION_MINOR 0)
set(STM_VERSION_PATCH 0)
set(STM_VERSION_STRING "${STM_VERSION_MAJOR}.${STM_VERSION_MINOR}.${STM_VERSION_PATCH}")

add_executable(stm)
target_compile_options(stm PRIVATE -Wall -Wextra -Werror)
target_compile_options(stm PRIVATE $<$<CONFIG:Debug>:-g -O0>)
target_compile_options(stm PRIVATE $<$<CONFIG:Release>:-O3>)
add_definitions(-D_GNU_SOURCE -D_XOPEN_SOURCE)

target_include_directories(stm PRIVATE ${CMAKE_SOURCE_DIR}/src)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()
message(STATUS "Current build type: ${CMAKE_BUILD_TYPE}")

add_subdirectory(src/libstm)

set(SRCS
    src/cmd.c
    src/init.c
    src/creds.c
    src/server.c
    src/dump.c
    src/load.c
    src/stm.c
)

set(CREDS_SUBCMD_SRC
    src/subcommands/creds/store.c
    src/subcommands/creds/status.c
    src/subcommands/creds/kill.c
)

set(SERVER_SUBCMD_SRC
    src/subcommands/server/add.c
    src/subcommands/server/del.c
    src/subcommands/server/list.c
    src/subcommands/server/ssh.c
    src/subcommands/server/find.c
    src/subcommands/server/info.c
    src/subcommands/server/info/get.c
)

if(APPLE)
    target_link_directories(stm PRIVATE /opt/homebrew/lib)
    target_include_directories(stm PRIVATE /opt/homebrew/include)
    target_link_libraries(stm PRIVATE csv)
    
    find_path(ARGP_INCLUDE_DIR argp.h HINTS /opt/homebrew/include /usr/local/include)
    find_library(ARGP_LIB argp HINTS /opt/homebrew/lib /usr/local/lib)

    if(ARGP_INCLUDE_DIR AND ARGP_LIB)
        add_library(argp INTERFACE)
        target_include_directories(argp INTERFACE ${ARGP_INCLUDE_DIR})
        target_link_libraries(argp INTERFACE ${ARGP_LIB})
    else()
        message(FATAL_ERROR "argp-standalone not found, please install via brew")
    endif()

    target_link_libraries(stm PRIVATE csv)
endif()


target_sources(stm PRIVATE ${SRCS} ${SERVER_SUBCMD_SRC} ${CREDS_SUBCMD_SRC})
target_link_libraries(stm PRIVATE stmlib argp)

install(TARGETS stm RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
if(APPLE)
    set_target_properties(stmlib PROPERTIES INSTALL_NAME_DIR "@rpath")
endif()