cmake_minimum_required(VERSION 3.30.8)
project(libstm C)

set(CMAKE_C_STANDARD 23)
option(BUILD_SHARED_LIBS "Build shared library" ON)

set(SRCS init.c db.c file.c sec.c ssh.c
    utils.c error.c formatter.c compress.c)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLCIPHER REQUIRED sqlcipher)
pkg_check_modules(JANSSON REQUIRED jansson)
pkg_check_modules(LIBSSH REQUIRED libssh)
pkg_check_modules(LIBZSTD REQUIRED libzstd)
add_library(stmlib ${SRCS})

target_include_directories(stmlib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${SQLCIPHER_INCLUDE_DIRS} ${JANSSON_INCLUDE_DIRS} ${LIBSSH_INCLUDE_DIRS}
)

# target_compile_options(stmlib PRIVATE -fvisibility=hidden)
target_link_libraries(stmlib PUBLIC
                     ${SQLCIPHER_LIBRARIES}
                     ${JANSSON_LIBRARIES}
                     ${LIBSSH_LIBRARIES}
                     ${LIBZSTD_LIBRARIES}
                     crypto)