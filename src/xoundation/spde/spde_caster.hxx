//
// Created by secondwtq 15-5-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef HEADER_SPEEDER_MONKEY_CASTER
#define HEADER_SPEEDER_MONKEY_CASTER

#include "../thirdpt/js_engine.hxx"

#include "spde_heroes.hxx"
#include "spde_classinfo.hxx"
#include "spde_vivalavida.hxx"
#include "details/spde_intrusive_object.hxx"
#include <string>

#include <cassert>

#include <type_traits>

namespace xoundation {
namespace spd {

template <typename T, bool is_enum = std::is_enum<T>::value>
struct micro_caster;

template <typename T>
struct micro_caster<T, false> {

    inline static JS::Value tojs(JSContext *c, const T& src) {
        JS::RootedObject proto(c, class_info<T>::instance()->jsc_proto);
        JSObject *jsobj = JS_NewObject(c, class_info<T>::instance()->jsc_def, proto, JS::NullPtr());
        lifetime<T> *lt = new lifetime_js<T>(src);
        JS_SetPrivate(jsobj, reinterpret_cast<void *>(lt));
        return OBJECT_TO_JSVAL(jsobj);
    }

    inline static const T& back(JSContext *c, JS::HandleValue src) {
        lifetime<T> *t = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(src.toObjectOrNull()));
        return *(t->get());
    }

};

template <typename T>
struct micro_caster<T, true> {

    inline static JS::Value tojs(JSContext *c, T src) {
        JS::RootedValue ret(c);
        ret.setInt32(static_cast<int>(src));
        return ret;
    }

    inline static T back(JSContext *c, JS::HandleValue src) {
        return static_cast<T>(src.toInt32()); }

};

template<typename T>
struct caster {
    using actualT = const T&;
    using backT = const T&;
    using jsT = JS::Value;

    // used when a C++ function returns T is called from JS
    //
    //      (ok you guys love to use 'JS' instead of 'JavaScript', 'javascript',
    //       'Javascript', 'script' or whatever, then I use 'JS' too...)
    //  ctor of T called from JS uses a different mechanism, since
    //  it has a JS lifetime by default, see details/spde_constructor.hxx.
    inline static jsT tojs(JSContext *c, actualT src) {
        return micro_caster<T>().tojs(c, src);
    }

    // used when a C++ function called from JS
    //  reads arguments passed from JS
    inline static auto back(JSContext *c, JS::HandleValue src)
            -> decltype(micro_caster<T>().back(c, src)) {
        return micro_caster<T>().back(c, src); }

};

template <typename T, bool is_intrusive = std::is_base_of<intrusive_object<T>, T>::value>
struct micro_caster_ptr;

template <typename T>
struct micro_caster_ptr<T, false> {

    inline static JS::Value tojs(JSContext *c, T *src) {
        if (src == nullptr) {
            return JS::UndefinedValue(); }

        JS::RootedObject proto(c, class_info<T>::instance()->jsc_proto);
        JSObject *jsobj = JS_NewObject(c, class_info<T>::instance()->jsc_def, proto, JS::NullPtr());
        lifetime<T> *lt = new lifetime_cxx<T>(src);
        JS_SetPrivate(jsobj, reinterpret_cast<void *>(lt));
        return OBJECT_TO_JSVAL(jsobj);
    }

};

template <typename T>
struct micro_caster_ptr<T, true> {

    inline static JS::Value tojs(JSContext *c, T *src) {
        if (src == nullptr) {
            return JS::UndefinedValue(); }

        if (!details::get_instrusive_wrapper(src).inited()) {
            JS::RootedValue ret(c, micro_caster_ptr<T, false>::tojs(c, reinterpret_cast<T *>(src)));
            details::get_instrusive_wrapper(src).init(c);
            details::get_instrusive_wrapper(src).get()->set(ret.toObjectOrNull());
            return ret;
        } else { return JS::ObjectOrNullValue(*(details::get_instrusive_wrapper(src).get())); }
    }

};

template<typename T>
struct caster<T *> {

    using actualT = T *;
    using backT = T *;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return micro_caster_ptr<T>().tojs(c, src); }

    inline static backT back(JSContext *c, JS::HandleValue src) {
        if (src.isUndefined()) {
            return nullptr; }
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
        return micro_caster_ptr<T>().tojs(c, &src); }

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

}
}

#include "details/spde_caster_numeric.hxx"
#include "details/spde_caster_string.hxx"
#include "details/spde_caster_ext.hxx"

#endif
