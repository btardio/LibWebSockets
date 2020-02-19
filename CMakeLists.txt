cmake_minimum_required(VERSION 2.8)

# if (NOT LIBEV_FOUND)
#         find_path(LIBEV_INCLUDE_DIRS NAMES ev.h)
#         find_library(LIBEV_LIBRARIES NAMES ev)
#         if(LIBEV_INCLUDE_DIRS AND LIBEV_LIBRARIES)
#                 set(LIBEV_FOUND 1)
#         endif()
# endif()

#gcc -L/lib64 -Wl,-rpath=/lib64 -I/usr/include/hiredis -o server lwsev.c lws-callback.c lwsev-echogen.c lwsev-http.c lwsev-mirror.c lwsev-status.c -lwebsockets -lev -lhiredis -lSSL


# Try to find libev
# Once done, this will define
#
# LIBEV_FOUND        - system has libev
# LIBEV_INCLUDE_DIRS - libev include directories
# LIBEV_LIBRARIES    - libraries needed to use libev

if(LIBEV_INCLUDE_DIRS AND LIBEV_LIBRARIES)
  set(LIBEV_FIND_QUIETLY TRUE)
else()
  find_path(
    LIBEV_INCLUDE_DIR
    NAMES ev.h
    HINTS ${LIBEV_ROOT_DIR}
    PATH_SUFFIXES include)

  find_library(
    LIBEV_LIBRARY
    NAME ev
    HINTS ${LIBEV_ROOT_DIR}
    PATH_SUFFIXES ${CMAKE_INSTALL_LIBDIR})

  set(LIBEV_INCLUDE_DIRS ${LIBEV_INCLUDE_DIR})
  set(LIBEV_LIBRARIES ${LIBEV_LIBRARY})

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
    libev DEFAULT_MSG LIBEV_LIBRARY LIBEV_INCLUDE_DIR)

  mark_as_advanced(LIBEV_LIBRARY LIBEV_INCLUDE_DIR)
endif()









find_package(PkgConfig)
pkg_check_modules(LIB_WEBSOCKETS REQUIRED libwebsockets)
pkg_check_modules(LIB_SSL REQUIRED libssl)
pkg_check_modules(ZLIB REQUIRED zlib)
pkg_check_modules(hiredis REQUIRED hiredis)
pkg_check_modules(GLIB-2.0 REQUIRED glib-2.0)
pkg_check_modules(json-glib-1.0 REQUIRED json-glib-1.0)
#pkg_check_modules(LIB_EV REQUIRED libev)
#pkg_check_modules(hiredisev REQUIRED hiredisev)
#pkg_check_modules(LIB_EV REQUIRED libev)

#pkg_check_modules(LIBEV REQUIRED libev)

#get_filename_component(
#  LIB_WEBSOCKETS_INSTALL_DIR
#  ${LIB_WEBSOCKETS_LIBRARY_DIRS}
#  DIRECTORY
#)

#add_definitions(INSTALL_DATADIR="/home/btardio/Desktop/websitegrab/libwebsocketserver/libwebsockets/share")

include_directories(${LIB_WEBSOCKETS_INCLUDE_DIRS})
include_directories(${LIB_SSL_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${hiredis_INCLUDE_DIRS})
include_directories(${LIBEV_INCLUDE_DIRS})
include_directories(${GLIB-2.0_INCLUDE_DIRS})
include_directories(${json-glib-1.0_INCLUDE_DIRS})
#include_directories("/home/btardio/Desktop/websitegrab/libwebsocketserver/libwebsockets/lib/")
#include_directories(${LIBEV_INCLUDE_DIRS})


link_directories(${LIB_WEBSOCKETS_LIBRARY_DIRS})
link_directories(${LIB_SSL_LIBRARY_DIRS})
link_directories(${ZLIB_LIBRARY_DIRS})
link_directories(${hiredis_LIBRARY_DIRS})
link_directories(${LIBEV_LIBRARIES})
link_directories(${GLIB-2.0_LIBRARIES})
link_directories(${json-glib-1.0_LIBRARIES})
#link_directories("/usr/lib64/")
#link_directories(${LIBEV_LIBRARY_DIRS})

add_definitions(${LIB_WEBSOCKETS_CFLAGS_OTHER})
add_definitions(${LIB_SSL_CFLAGS_OTHER})
add_definitions(${ZLIB_CFLAGS_OTHER})
add_definitions(${hiredis_CFLAGS_OTHER})
add_definitions(${GLIB-2.0_CFLAGS_OTHER})
add_definitions(${json-glib-1.0_CFLAGS_OTHER})
#add_definitions(${LIBEV_CFLAGS_OTHER})



add_executable(
    server
      #lws.c
      #lws-callback.c
      lwsev.c
      lws-callback.c
      #lwsev-echogen.c
      #lwsev-http.c
      #lwsev-mirror.c
      #lwsev-status.c
      #lwsev
)

target_link_libraries(server ${LIB_WEBSOCKETS_LIBRARIES})
target_link_libraries(server ${LIB_SSL_LIBRARIES})
target_link_libraries(server ${ZLIB_LIBRARIES})
target_link_libraries(server ${hiredis_LIBRARIES})
target_link_libraries(server ${LIBEV_LIBRARIES})
target_link_libraries(server ${GLIB-2.0_LIBRARIES})
target_link_libraries(server ${json-glib-1.0_LIBRARIES})

#target_link_libraries(a-test-server ${LIBEV_LIBRARIES})

# set_property(TARGET a-test-server
#                         PROPERTY COMPILE_DEFINITIONS
#                                 INSTALL_DATADIR="/home/btardio/Desktop/websitegrab/libwebsocketserver/libwebsockets/share"
#                         )

#set_target_properties(test-server
#    PROPERTIES
#      INCLUDE_DIRECTORIES
#        ${LIB_WEBSOCKETS_INCLUDE_DIRS}
#      LINK_FLAGS
#        "-L${LIB_WEBSOCKETS_LIBRARY_DIRS}"
#      COMPILE_DEFINITIONS
#        INSTALL_DATADIR="${LIB_WEBSOCKETS_INSTALL_DIR}/share"
#)