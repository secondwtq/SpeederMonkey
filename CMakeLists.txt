cmake_minimum_required(VERSION 3.0)
project(mozjs)

include_directories(SYSTEM .)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    xoundation/spde/spde_classinfo.hpp
    spde_test.cpp)

add_executable(mozjs ${SOURCE_FILES} xoundation/spde/spde_classhelper.hpp xoundation/spde/spde_helpers.hpp xoundation/spde/spde_caster.hpp spde_test.cpp xoundation/spde/spde_test_common.h xoundation/spde/spde_heroes.hpp xoundation/spde/spde_functionbind.hpp xoundation/native/node_native_fs.h xoundation/spde.hpp xoundation/native/node_module.h xoundation/platform.h)
target_link_libraries(mozjs mozjs-31)
