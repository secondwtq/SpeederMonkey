//
// Created by secondwtq 15-5-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef HEADER_SPEEDER_MONKEY_CASTER
#define HEADER_SPEEDER_MONKEY_CASTER

#include <jsapi.h>

#include "spde_heroes.hpp"
#include "spde_classinfo.hpp"
#include "spde_vivalavida.hxx"
#include <string>

#include <cassert>

namespace xoundation {
namespace spd {

template<typename T>
struct caster {
    using actualT = const T&;
    using backT = const T&;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        JS::RootedObject proto(c, class_info<T>::instance()->jsc_proto);
        JSObject *jsobj = JS_NewObject(c, class_info<T>::instance()->jsc_def, proto, JS::NullPtr());
        lifetime<T> *lt = new lifetime_js<T>(src);
        JS_SetPrivate(jsobj, reinterpret_cast<void *>(lt));
        return OBJECT_TO_JSVAL(jsobj);
    }

    inline static backT back(JSContext *c, JS::HandleValue src) {
        lifetime<T> *t = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(src.toObjectOrNull()));
        return *(t->get());
    }

};

template<typename T>
struct caster<T *> {

    using actualT = T *;
    using backT = T *;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        JS::RootedObject proto(c, class_info<T>::instance()->jsc_proto);
        JSObject *jsobj = JS_NewObject(c, class_info<T>::instance()->jsc_def, proto, JS::NullPtr());
        lifetime<T> *lt = new lifetime_cxx<T>(src);
        JS_SetPrivate(jsobj, reinterpret_cast<void *>(lt));
        return OBJECT_TO_JSVAL(jsobj);
    }

    inline static backT back(JSContext *c, JS::HandleValue src) {
        lifetime<T> *t = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(src.toObjectOrNull()));
        return t->get();
    }

};

template<typename T>
struct caster<T&> {

    using actualT = const T&;
    using backT = T&;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        JS::RootedObject proto(c, class_info<T>::instance()->jsc_proto);
        JSObject *jsobj = JS_NewObject(c, class_info<T>::instance()->jsc_def, proto, JS::NullPtr());
        lifetime<T> *lt = new lifetime_cxx<T>(&src);
        printf("setting private T& %lx ...\n", &src);
        JS_SetPrivate(jsobj, reinterpret_cast<void *>(lt));
        return OBJECT_TO_JSVAL(jsobj);
    }

    inline static backT back(JSContext *c, JS::HandleValue src) {
        lifetime<T> *t = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(src.toObjectOrNull()));
        return *(t->get());
    }
};

template<typename T>
struct caster<const T&> {

    using actualT = const T&;
    using backT = T&;
    using jsT = JS::Value;

};

template<>
struct caster<JS::MutableHandleValue> {

    using actualT = JS::MutableHandleValue;
    using backT = JS::MutableHandleValue;
    using jsT = JS::MutableHandleValue;

    inline static jsT tojs(JSContext *c, actualT src) {
        return src; }

    inline static backT back(JSContext *c, JS::MutableHandleValue src) {
        return src; }

};

template<>
struct caster<JS::HandleValue> {

    using actualT = JS::HandleValue;
    using jsT = JS::HandleValue;
    using backT = JS::HandleValue;

    inline static jsT tojs(JSContext *c, actualT src) {
        return src; }

    inline static backT back(JSContext *c, JS::HandleValue src) {
        return src; }

};

//template<>
//struct caster<JS::HandleValue> {
//
//    using actualT = JS::HandleValue;
//    using backT = JS::HandleValue;
//    using jsT = JS::Value;
//
//    inline static jsT tojs(JSContext *c, actualT src) {
//        return
//    }
//
//    inline static backT back(JSContext *c, JS::Value *src) {
//        return { *src };
//    }
//
//};

template<>
struct caster<int> {
    using actualT = int;
    using backT = int;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return INT_TO_JSVAL(src);
    }

    inline static backT back(JSContext *, JS::HandleValue src) {
        return src.toInt32();
    }

};

template<>
struct caster<long> {
    using actualT = long;
    using backT = long;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return INT_TO_JSVAL(src);
    }

    inline static backT back(JSContext *, JS::HandleValue src) {
        return src.toInt32();
    }

};

template<>
struct caster<bool> {
    using actualT = bool;
    using backT = bool;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return BOOLEAN_TO_JSVAL(src);
    }

    inline static backT back(JSContext *c, JS::HandleValue src) {
        return src.toBoolean();
    }
};

template<>
struct caster<const char *> {

    using actualT = const char *;
    using backT = const char *;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return STRING_TO_JSVAL(JS_NewStringCopyZ(c, src));
    }

};

template<>
struct caster<std::string> {

    using actualT = const std::string&;
    using backT = std::string;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return STRING_TO_JSVAL(JS_NewStringCopyZ(c, src.c_str()));
    }

    inline static backT back(JSContext *c, JS::HandleValue src) {
        return JS_EncodeString(c, JSVAL_TO_STRING(src));
    }

};

template<>
struct caster<std::string&> {

    using actualT = const std::string&;
    using backT = std::string;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return STRING_TO_JSVAL(JS_NewStringCopyZ(c, src.c_str()));
    }

    inline static backT back(JSContext *c, JS::HandleValue src) {
        return JS_EncodeString(c, JSVAL_TO_STRING(src));
    }

};

template<>
struct caster<const std::string&> {

    using actualT = const std::string&;
    using backT = std::string;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return STRING_TO_JSVAL(JS_NewStringCopyZ(c, src.c_str()));
    }

    inline static backT back(JSContext *c, JS::HandleValue src) {
        return JS_EncodeString(c, JSVAL_TO_STRING(src));
    }

};

}
}
#endif
