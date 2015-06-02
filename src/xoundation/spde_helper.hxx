//
// Created by secondwtq <lovejay-lovemusic@outlook.com> 2015/05/29.
// Copyright (c) 2015 SCU ISDC All rights reserved.
//
// This file is part of ISDCNext.
//
// We have always treaded the borderland.
//

#ifndef MOZJS_HELPER_HXX
#define MOZJS_HELPER_HXX

#include <jsapi.h>
#include <stdio.h>

#include "spde/spde_classhelper.hpp"

namespace xoundation {

static JSClass cls_global = {
    "mozjs_context", JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub, JS_DeletePropertyStub,
    JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,
    nullptr, nullptr, nullptr, nullptr,
    JS_GlobalObjectTraceHook
};

// helpers, 150529 EVE
template <typename T>
using klass = spd::class_helper<T>;

class SpdInitialize {
    public:

    static bool Inited(bool set = false) {
        static bool inited = false;
        if (!set) {
            return inited;
        } else {
            inited = true;
            return inited;
        }
    }

    static void Init() {
        if (!Inited()) {
            if (!JS_Init()) {
                printf("SpdInitialize: JS_Init failed!\n");
            } else {
                Inited(true);
            }
        }
        return;
    }

};

// runtime management 150529 EVE
class SpdRuntime {

    public:

    static inline void error_reporter(JSContext *context, const char *msg, JSErrorReport *rep) {
        (void) context; // a placeholder to suppress compiler warning
        fprintf(stderr, "%s:%u: %s\n", rep->filename ? rep->filename : "[anonymous]",
                rep->lineno, msg); }

    // tsc.js failed with strict mode
    SpdRuntime(size_t max_bytes = 32 * 1024L * 1024L, size_t stack_size = 4096,
                   bool jit = true, bool strict = false) {
        SpdInitialize::Init();
        m_runtime = JS_NewRuntime(max_bytes, JS_USE_HELPER_THREADS);
        m_context = JS_NewContext(m_runtime, stack_size);
        if (!m_context)
            fprintf(stderr, "SpdRuntime::SpdRuntime: create context failed!\n");

        JS_SetNativeStackQuota(m_runtime, 256 * sizeof(size_t) * 1024);

        JS_SetErrorReporter(m_context, SpdRuntime::error_reporter);

        if (jit) {
            enable_jit(); }
        if (strict) {
            use_strict(); }
    }

    ~SpdRuntime() {
        JS_DestroyContext(m_context);
        JS_DestroyRuntime(m_runtime);
    }

    void enable_jit() {
        JS::RuntimeOptionsRef(m_runtime).setBaseline(true).setIon(true).setAsmJS(true);

        JS_SetParallelIonCompilationEnabled(m_runtime, true);
        JS_SetGlobalJitCompilerOption(m_runtime, JSJITCOMPILER_ION_ENABLE, 1);
        JS_SetGlobalJitCompilerOption(m_runtime, JSJITCOMPILER_BASELINE_ENABLE, 1);
        // JS::ContextOptionsRef(m_context).setExtraWarnings(1).setVarObjFix(1);
    }

    // extra warnings and strict mode
    void use_strict() {
        JS::ContextOptionsRef(m_context).setStrictMode(1); }

    void set_error_reporter(void (*func)(JSContext *, const char *, JSErrorReport *)) {
        JS_SetErrorReporter(m_context, func); }

    JSContext *context() { return m_context; }
    JSRuntime *runtime() { return m_runtime; }

    operator JSContext *() {
        return context(); }

//    JS::HandleObject create_global() {
//        JS::CompartmentOptions options;
//        options.setVersion(JSVERSION_LATEST);
//
//        JS::RootedObject global(context(), JS_NewGlobalObject(context(), &cls_global,
//                                      nullptr, JS::DontFireOnNewGlobalHook, options));
//        if (!global) return JS::NullPtr();
//        JSAutoCompartment cp(context(), global);
//        if (!JS_InitStandardClasses(context(), global)) return JS::NullPtr();
//        // JS_FireOnNewGlobalObject(context(), global);
////        JS::MutableHandleObject h(&global);
////        if (!JS_WrapObject(context(), h)) {
////            fprintf(stderr, "SpdRuntime::create_global - wrapping failed!\n");
////            return JS::NullPtr();
////        }
//
//        return global;
//    }

    private:

    JSRuntime *m_runtime;
    JSContext *m_context;

};

}

#endif // MOZJS_HELPER_HXX
