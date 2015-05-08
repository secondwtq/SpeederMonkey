cmake_minimum_required(VERSION 3.0)
project(mozjs)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lmozjs-31")

set(SOURCE_FILES
    spde/spde_classinfo.hpp
    spde_test.cpp)

add_executable(mozjs ${SOURCE_FILES} spde/spde_classhelper.hpp spde/spde_helpers.hpp spde/spde_caster.hpp spde_test.cpp spde_test_common.h spde/spde_heroes.hpp spde/spde_functionbind.hpp node_native_fs.h spde/spde.hpp node_module.h)