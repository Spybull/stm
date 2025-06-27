cmake_minimum_required(VERSION 3.30.8)
project(stm C)

set(CMAKE_C_STANDARD 23)
set(STM_VERSION_MAJOR 1)
set(STM_VERSION_MINOR 0)
set(STM_VERSION_PATCH 0)
set(STM_VERSION_STRING "${STM_VERSION_MAJOR}.${STM_VERSION_MINOR}.${STM_VERSION_PATCH}")

add_executable(stm)
target_compile_options(stm PRIVATE -Wall -Wextra -Werror -pedantic)
add_definitions(-D_GNU_SOURCE -D_XOPEN_SOURCE)

add_subdirectory(src/libstm)
set(SRCS src/init.c src/stm.c)

target_link_libraries(stm PRIVATE libstm)
target_sources(stm PRIVATE ${SRCS})
