//
// Created by secondwtq <lovejay-lovemusic@outlook.com> 2015/11/06.
// Copyright (c) 2015 SCU ISDC All rights reserved.
//
// This file is part of ISDCNext.
//
// We have always treaded the borderland.
//

#ifndef MOZJS_SPDE_ESPWRAP_HXX
#define MOZJS_SPDE_ESPWRAP_HXX

#include "../thirdpt/js_engine.hxx"

namespace xoundation {
namespace spd {
namespace espwrap {

inline JSObject *JSDefineObject(JSContext *cx, JS::HandleObject obj,
        const char *name, const JSClass *clasp,
        JSObject *proto, unsigned attrs) {
#if defined(SPD_MOZJS38)
    return JS_DefineObject(cx, obj, name, clasp, attrs);
#else
    return JS_DefineObject(cx, obj, name, clasp, proto, attrs);
#endif
}

inline JSObject *NewObject(JSContext *cx, const JSClass *clasp,
        JS::Handle<JSObject*> proto, JS::Handle<JSObject*> parent = JS::NullPtr()) {
#if defined(SPD_MOZJS38)
    return JS_NewObject(cx, clasp, parent);
#else
    return JS_NewObject(cx, clasp, proto, parent);
#endif
}

inline void SetParallelIonCompilationEnabled(JSRuntime* rt, bool enabled) {
#if defined(SPD_MOZJS38)
    JS_SetOffthreadIonCompilationEnabled(rt, enabled);
#else
    JS_SetParallelIonCompilationEnabled(rt, enabled);
#endif
}

inline bool EvaluateScriptDirect(JSContext *cx, JS::HandleObject obj,
        const char *bytes, unsigned int length, const char *filename,
        unsigned int lineno, JS::MutableHandleValue rval) {
#if defined(SPD_MOZJS38)
    JS::CompileOptions options(cx);
    options.setFileAndLine(filename, lineno);
    return JS::Evaluate(cx, obj, options, bytes, length, rval);
#else
   return JS_EvaluateScript(cx, obj, bytes, length, filename, lineno, rval);
#endif
}

#if defined(SPD_MOZJS38)
const JSDeletePropertyOp StubDelProp = nullptr;
const JSEnumerateOp StubEnum = nullptr;
const JSResolveOp StubResolve = nullptr;
const JSConvertOp StubConv = nullptr;

const uint32_t CLASS_NEW_RESOLVE = 0;
const uint32_t PROP_NATIVE_ACCESSOR = 0;

typedef JSNative TypeNativeGetter;
typedef JSNative TypeNativeSetter;

#else
const JSDeletePropertyOp StubDelProp = JS_DeletePropertyStub;
const JSEnumerateOp StubEnum = JS_EnumerateStub;
const JSResolveOp StubResolve = JS_ResolveStub;
const JSConvertOp StubConv = JS_ConvertStub;

const uint32_t CLASS_NEW_RESOLVE = JSCLASS_NEW_RESOLVE;
const uint32_t PROP_NATIVE_ACCESSOR = JSPROP_NATIVE_ACCESSORS;

typedef JSPropertyOp TypeNativeGetter;
typedef JSStrictPropertyOp TypeNativeSetter;

#endif

}
}
}

#endif // MOZJS_SPDE_ESPWRAP_HXX
