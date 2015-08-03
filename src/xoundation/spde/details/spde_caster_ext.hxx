//
// Created by secondwtq 15-8-3.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_SPDE_CONTEXT_REFERENCE_HXX
#define MOZJS_SPDE_CONTEXT_REFERENCE_HXX

#include <jsapi.h>

namespace xoundation {
namespace spd {

struct context_reference {
public:
    JSContext *context() { return m_ref; }
    operator JSContext *() {
        return context(); }

private:
    context_reference(JSContext *ref) : m_ref(ref) { }
    JSContext *m_ref;

    friend struct caster<context_reference>;
};

template<>
struct caster<context_reference> {
    using actualT = context_reference;
    using backT = context_reference;

    inline static backT back(JSContext *c, JS::HandleValue src) {
        return { c }; }
};

template<>
struct caster<JS::PersistentRootedValue> {

    using actualT = JS::PersistentRootedValue;
    using backT = JS::PersistentRootedValue;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return src.get(); }

    inline static backT back(JSContext *c, JS::HandleValue src) {
        return { c, src }; }

};

}
}

// well, I'm wrong.
//  maybe WE NEED SHARED RUNTIME!
//  currently you must provide something like
//  factory that returns a std::shared_ptr<T>
//  in C++ side for it
#if !defined(XOUNDATION_DISABLE_STDSHARED)

#include <memory>

namespace xoundation {
namespace spd {

template<typename T>
struct caster<std::shared_ptr<T>> {

    using actualT = std::shared_ptr<T>;
    using backT = std::shared_ptr<T>;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        if (src == nullptr) {
            return JS::UndefinedValue(); }

        JS::RootedObject proto(c, class_info<T>::instance()->jsc_proto);
        JSObject *jsobj = JS_NewObject(c, class_info<T>::instance()->jsc_def, proto, JS::NullPtr());
        lifetime<T> *lt = new lifetime_shared_std<T>(src);
        JS_SetPrivate(jsobj, reinterpret_cast<void *>(lt));
        return OBJECT_TO_JSVAL(jsobj);
    }

    inline static backT back(JSContext *c, JS::HandleValue src) {
        if (src.isUndefined()) {
            return nullptr; }
        lifetime_shared_std<T> *t = reinterpret_cast<lifetime_shared_std<T> *>(JS_GetPrivate(src.toObjectOrNull()));
        return t->get_shared();
    }

};

}
}

#endif

#endif // MOZJS_SPDE_CONTEXT_REFERENCE_HXX
