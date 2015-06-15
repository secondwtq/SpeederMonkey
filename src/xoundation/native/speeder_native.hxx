//
// Created by secondwtq 15-6-15.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_SPEEDER_NATIVE_HXX
#define MOZJS_SPEEDER_NATIVE_HXX

#include <iostream>
#include <string>

#include <jsapi.h>
#include <xoundation/spde/spde_test_common.h>
#include <xoundation/spde.hpp>

namespace xoundation {
namespace native {

inline bool speeder_print(JSContext *context, unsigned int argc, JS::Value *vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    for (size_t i = 0; i < args.length(); i++) {
        const char *t = JS_EncodeString(context, JS::ToString(context, args[i]));
        printf("%s", t);
        JS_free(context, const_cast<char *>(t));
    }
    printf("\n");
    args.rval().setUndefined();

    return true;
}

inline std::string speeder_getline() {
    std::string ret;
    getline(std::cin, ret);
    return ret;
}

inline bool speeder_collectgarbage(JSContext *context, unsigned int argc, JS::Value *vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS_GC(JS_GetRuntime(context));
    args.rval().setUndefined();
    return true;
}

void register_interface_speeder(JSContext *context, JS::HandleObject parent) {
    JS::RootedObject speeder(context, JS_DefineObject(context, parent, "speeder", nullptr, nullptr,
                            JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY));

    JS_DefineFunction(context, speeder, "print", speeder_print, 1, attrs_func_default);
    JS_DefineFunction(context, speeder, "collectgarbage", speeder_collectgarbage, 0, attrs_func_default);
    JS_DefineFunction(context, speeder, "getline", spd::function_callback_wrapper<decltype(speeder_getline),
                              speeder_getline>::callback, 0, attrs_func_default);
}

}
}

#endif // MOZJS_SPEEDER_NATIVE_HXX
