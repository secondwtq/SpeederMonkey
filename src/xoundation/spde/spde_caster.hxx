//
// Created by secondwtq 15-5-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef HEADER_SPEEDER_MONKEY_CASTER
#define HEADER_SPEEDER_MONKEY_CASTER

#include <jsapi.h>

#include "spde_heroes.hxx"
#include "spde_classinfo.hxx"
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
        if (src == nullptr) {
            return JS::UndefinedValue();
        }

        JS::RootedObject proto(c, class_info<T>::instance()->jsc_proto);
        JSObject *jsobj = JS_NewObject(c, class_info<T>::instance()->jsc_def, proto, JS::NullPtr());
        lifetime<T> *lt = new lifetime_cxx<T>(src);
        JS_SetPrivate(jsobj, reinterpret_cast<void *>(lt));
        return OBJECT_TO_JSVAL(jsobj);
    }

    inline static backT back(JSContext *c, JS::HandleValue src) {
        if (src.isUndefined()) {
            return nullptr;
        }
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
        if (src.isUndefined()) {
            // TODO: cast undefined back to reference - an exception?
        }
        lifetime<T> *t = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(src.toObjectOrNull()));
        return *(t->get());
    }
};

template<typename T>
struct caster<const T&> {

    using actualT = const T&;
    using backT = const T&;
    using jsT = JS::Value;

    // how should tojs() do?

    inline static backT back(JSContext *c, JS::HandleValue src) {
        lifetime<T> *t = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(src.toObjectOrNull()));
        return *(t->get());
    }

};

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
    } // does toInt32() proper here?

};

template<>
struct caster<size_t> {

    using actualT = size_t;
    using backT = size_t;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return UINT_TO_JSVAL(static_cast<unsigned int>(src));
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
        // should this be undefined?
        if (src == nullptr) {
            return JS::UndefinedValue();
        }
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
        const char *t = JS_EncodeString(c, JSVAL_TO_STRING(src));
        std::string ret(t);
        JS_free(c, const_cast<char *>(t));
        return ret;
    }

};

}
}

#include "details/spde_caster_ext.hxx"

#endif
