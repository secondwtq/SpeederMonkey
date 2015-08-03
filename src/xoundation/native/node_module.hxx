//
// Created by secondwtq 15-5-8.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_NODE_MODULE_H
#define MOZJS_NODE_MODULE_H

#include <jsapi.h>
#include <xoundation/spde.hxx>
#include <xoundation/spde/spde_test_common.hxx>

namespace xoundation {

namespace native {

static JSClass sandbox_class = {
    "sandbox", JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub, JS_DeletePropertyStub,
    JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,
    nullptr, nullptr, nullptr, nullptr,
    JS_GlobalObjectTraceHook
};

bool create_global_env(JSContext *c, unsigned int argc, JS::Value *vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::CompartmentOptions options;
    options.setVersion(JSVERSION_LATEST);

    JS::RootedObject global(c, JS_NewGlobalObject(c, &sandbox_class, nullptr,
                                                 JS::DontFireOnNewGlobalHook, options));
    if (!global) return false;
    {
        JSAutoCompartment cp(c, global);
        if (!JS_InitStandardClasses(c, global)) return false;
    }
    JS_FireOnNewGlobalObject(c, global);

    JS::MutableHandleObject h(&global);
    if (!JS_WrapObject(c, h)) {
        printf("wrapping failed!\n");
        return false;
    }

    args.rval().setObject(*global);
    return true;
}

bool eval(JSContext *c, unsigned int argc, JS::Value *vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    std::string source = spd::caster<const std::string&>::back(c, args[0]);
    std::string name = spd::caster<const std::string&>::back(c, args[1]);

    JS::RootedObject current_global(c, JS::CurrentGlobalOrNull(c));
    JS::RootedValue ret(c);
    bool ok = JS_EvaluateScript(c, current_global, source.c_str(), static_cast<unsigned int>(source
                                    .length()), name.c_str(), 0, &ret);

    args.rval().set(ret);
    return ok;
}

bool eval_in_sandbox(JSContext *c, unsigned int argc, JS::Value *vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    std::string source = spd::caster<const std::string&>::back(c, args[0]);
    std::string name = spd::caster<const std::string&>::back(c, args[2]);
    JS::RootedObject new_global(c, args[1].toObjectOrNull());
    JS::RootedValue ret(c);
    bool ok;

    {
        JSAutoCompartment ac(c, new_global); (void) ac; // needed for contexts work properly!
        ok = JS_EvaluateScript(c, new_global, source.c_str(), static_cast<unsigned int>(source
                                        .length()), name.c_str(), 0, &ret);
    }

    args.rval().set(ret);
    return ok;
}

void register_interface_modules(JSContext *context, JS::Handle<JSObject *> parent) {
    JS::RootedObject native_module(context, JS_DefineObject(context, parent, "_native_module",
                                                            nullptr, nullptr, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE));

    JS_DefineFunction(context, native_module, "eval", eval, 2, attrs_func_default);
    JS_DefineFunction(context, native_module, "eval_in_sandbox", eval_in_sandbox, 3, attrs_func_default);
    JS_DefineFunction(context, native_module, "create_global_env", create_global_env, 0, attrs_func_default);
}

}

}

#endif //MOZJS_NODE_MODULE_H
