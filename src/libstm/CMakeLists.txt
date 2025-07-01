cmake_minimum_required(VERSION 3.30.8)
project(libstm C)
set(CMAKE_C_STANDARD 23)

set(SRCS
    init.c
    db.c
    file.c
    sec.c
    sysuser.c
    utils.c
    error.c
    formatter.c)


find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLCIPHER REQUIRED sqlcipher)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JANSSON REQUIRED jansson)

add_library(libstm STATIC ${SRCS})
target_link_libraries(libstm PRIVATE
                     crypto
                     ${SQLCIPHER_LIBRARIES}
                     ${JANSSON_LIBRARIES})