cmake_minimum_required(VERSION 3.0)
project(mozjs)

option(BUILDEBUG "enable -g" OFF)
option(USE_STATIC "link with static mozjs binary (libjs_static.a)" OFF)

include_directories(SYSTEM ./src)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pagezero_size 10000 -image_base 100000000")

if(BUILDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

set(SOURCE_FILES
    src/xoundation/spde/spde_classinfo.hpp
    src/spde_test.cpp)

add_executable(mozjs ${SOURCE_FILES} src/xoundation/spde/spde_classhelper.hpp
src/xoundation/spde/spde_caster.hpp src/spde_test.cpp
src/xoundation/spde/spde_test_common.h src/xoundation/spde/spde_heroes.hpp
src/xoundation/spde/spde_functionbind.hpp src/xoundation/native/node_native_fs.h
src/xoundation/spde.hpp src/xoundation/native/node_module.h src/xoundation/platform.h
src/xoundation/spde_helper.hxx src/xoundation/spde/spde_vivalavida.hxx)

if (USE_STATIC)
    target_link_libraries(mozjs js_static z)

    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        target_link_libraries(mozjs pthread dl)
    endif()

else()
    target_link_libraries(mozjs mozjs-31)
endif()