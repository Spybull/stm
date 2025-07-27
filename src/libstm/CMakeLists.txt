cmake_minimum_required(VERSION 3.30.8)
project(libstm C)

set(CMAKE_C_STANDARD 23)
option(BUILD_SHARED_LIBS "Build shared library" ON)

set(SRCS
    init.c db.c file.c sec.c ssh.c
    utils.c error.c formatter.c compress.c)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLCIPHER REQUIRED sqlcipher)
pkg_check_modules(JANSSON REQUIRED jansson)
pkg_check_modules(LIBSSH REQUIRED libssh)
pkg_check_modules(LIBZSTD REQUIRED libzstd)
find_package(OpenSSL REQUIRED)

add_library(stmlib ${SRCS})

target_include_directories(stmlib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SQLCIPHER_INCLUDE_DIRS}
        ${JANSSON_INCLUDE_DIRS}
        ${LIBSSH_INCLUDE_DIRS}
        ${LIBZSTD_INCLUDE_DIRS})

target_link_directories(stmlib
    PUBLIC
        ${SQLCIPHER_LIBRARY_DIRS}
        ${JANSSON_LIBRARY_DIRS}
        ${LIBSSH_LIBRARY_DIRS}
        ${OPENSSL_LIBRARY_DIRS}
        ${LIBZSTD_LIBRARY_DIRS})


# target_compile_options(stmlib PRIVATE -fvisibility=hidden)
target_link_libraries(stmlib
    PUBLIC
        ${SQLCIPHER_LIBRARIES}
        ${JANSSON_LIBRARIES}
        ${LIBSSH_LIBRARIES}
        ${LIBZSTD_LIBRARIES}
        OpenSSL::Crypto)

install(TARGETS stmlib LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
set_target_properties(stm PROPERTIES INSTALL_RPATH "@loader_path/../lib")