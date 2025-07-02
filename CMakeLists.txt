cmake_minimum_required(VERSION 3.30.8)
project(stm C)
include(GNUInstallDirs)

set(CMAKE_C_STANDARD 23)
set(STM_VERSION_MAJOR 1)
set(STM_VERSION_MINOR 0)
set(STM_VERSION_PATCH 0)
set(STM_VERSION_STRING "${STM_VERSION_MAJOR}.${STM_VERSION_MINOR}.${STM_VERSION_PATCH}")

add_executable(stm)
target_compile_options(stm PRIVATE -Wall -Wextra -Werror -pedantic)
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
    src/subcommands/creds/store.c
    src/server.c
    src/subcommands/server/add.c
    src/subcommands/server/del.c
    src/subcommands/server/list.c
    #src/subcommands/ssh.c
    src/stm.c)

target_link_libraries(stm PRIVATE libstm)
target_sources(stm PRIVATE ${SRCS})

install(TARGETS stm
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)