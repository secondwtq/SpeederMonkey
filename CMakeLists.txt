cmake_minimum_required(VERSION 3.0)
project(mozjs)

option(USE_STATIC "link with static mozjs binary (libjs_static.a)" OFF)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/usr/local/lib/pkgconfig")
# find_package(PkgConfig)
# pkg_check_modules(SPIDERM mozjs-31)
# include_directories(SYSTEM ${SPIDERM_INCLUDE_DIRS})

# include_directories(/usr/local/include/mozjs-38)

include_directories(SYSTEM ./src)
# 150616 CMake script revise
#   * build in Linux
#   * pagezerrro is OS X exclusive
if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g -O3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include js/RequiredDefines.h")
    if(APPLE)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pagezero_size 10000 -image_base 100000000")
    endif(APPLE)
endif (NOT WIN32)

function(prepend var prefix)
    set(listVar "")
    foreach(f ${ARGN})
        list(APPEND listVar "${prefix}/${f}")
    endforeach(f)
    set(${var} "${listVar}" PARENT_SCOPE)
endfunction(prepend)

set(HEADERS_SPDE spde_caster.hxx spde_heroes.hxx spde_vivalavida.hxx spde_functionbind.hxx
    spde_classinfo.hxx spde_classhelper.hxx spde_test_common.hxx spde_common.hxx
    details/spde_method_callback_wrapper.hxx details/spde_property_accessor.hxx
    details/spde_caster_ext.hxx details/spde_caster_numeric.hxx details/spde_caster_string.hxx
    details/spde_constructor.hxx details/spde_intrusive_object.hxx spde_enum.hxx details/spde_persistent_wrap.hxx)
prepend(HEADERS_SPDE "spde" ${HEADERS_SPDE})

set(HEADERS_XNATIVE node_module.hxx node_buffer.hxx node_native_fs.hxx speeder_native.hxx)
prepend(HEADERS_XNATIVE "native" ${HEADERS_XNATIVE})

set(HEADERS_XOUNDATION ${HEADERS_SPDE} ${HEADERS_XNATIVE} platform.h spde.hxx spde_helper.hxx thirdpt/js_engine.hxx)
prepend(HEADERS_XOUNDATION "xoundation" ${HEADERS_XOUNDATION})

set(SOURCES_XOUNDATION ${SOURCES_XOUNDATION} ${HEADERS_XOUNDATION})
prepend(SOURCES_XOUNDATION "src" ${SOURCES_XOUNDATION})

add_executable(mozjs ${SOURCES_XOUNDATION} src/spde_test.cpp)
add_executable(jsrunner ${SOURCES_XOUNDATION} src/jsrunner/jsrunner.cxx src/xoundation/spde/spde_espwrap.hxx)

find_package(MozJS REQUIRED)
if(MOZJS_FOUND)
    include_directories(SYSTEM ${MOZJS_INCLUDE_DIR})
    target_link_libraries(mozjs ${MOZJS_LIBRARY})
endif()

# if (USE_STATIC)
#     target_link_libraries(mozjs js_static z)
#     target_link_libraries(jsrunner js_static z)

#     if(CMAKE_SYSTEM_NAME MATCHES "Linux")
#         target_link_libraries(mozjs pthread dl)
#         target_link_libraries(jsrunner pthread dl)
#     endif()

# else()
#     target_link_libraries(mozjs mozjs-31)
#     target_link_libraries(jsrunner mozjs-31)
# #    target_link_libraries(mozjs js_static z)
# #    target_link_libraries(jsrunner mozjs-38)
# endif()
