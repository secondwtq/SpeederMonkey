//
// Created by secondwtq 15-8-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_INTRUSIVE_OBJECT_HXX
#define MOZJS_INTRUSIVE_OBJECT_HXX

#include "../../thirdpt/js_engine.hxx"

#include "../spde_vivalavida.hxx"
#include "../spde_classinfo.hxx"

#include <cassert>

namespace xoundation {
namespace spd {

template <typename T>
struct intrusive_object;

namespace details {

template<typename T>
struct persistent_rooted_wrapper;

template <typename T>
persistent_rooted_wrapper<JS::PersistentRootedObject>&
        get_instrusive_wrapper(intrusive_object<T> *src);


template <typename T>
struct persistent_rooted_wrapper {

    bool inited() const { return (m_impl != nullptr); }
    T *get() { return m_impl; }

    T *init(JSContext *c) {
        assert(m_impl == nullptr);
        return (m_impl = new T(c));
    }

    // donot use this within JS ctor!
    //  it assumes the lifetime is managed by CXX!
    //  cuz if an intrusived object is not initialized,
    //  it must be constructed in CXX
    //
    // an object created in JS would be automatically initialized
    //  by ctor_addon_intrusive<T, true>::callback.
    T *init_with_obj(JSContext *c, T *src) {
        assert(m_impl == nullptr);

        JS::RootedObject proto(c, class_info<T>::instance()->jsc_proto);
        JS::RootedObject jsobj(c, JS_NewObject(c, class_info<T>::instance()->jsc_def, proto, JS::NullPtr()));
        lifetime<T> *lt = new lifetime_cxx<T>(src);
        JS_SetPrivate(jsobj, reinterpret_cast<void *>(lt));
        m_impl->set(jsobj);
    }

    ~persistent_rooted_wrapper() {
        if (m_impl) {
            delete m_impl; }
    }

private:
    T *m_impl = nullptr;
};

}

template <typename T>
struct intrusive_object {
private:

    details::persistent_rooted_wrapper<JS::PersistentRootedObject> unique_obj;

    template <typename T_>
    friend details::persistent_rooted_wrapper<JS::PersistentRootedObject> &
            details::get_instrusive_wrapper(intrusive_object<T_> *);

};

namespace details {

template <typename T>
inline details::persistent_rooted_wrapper<JS::PersistentRootedObject> &
        get_instrusive_wrapper(intrusive_object<T> *src) {
    return src->unique_obj; }

}

// what do you think of the type of it's return value?
template <typename T>
JS::HandleObject get_intrusive_object(intrusive_object<T> *src) {
    details::persistent_rooted_wrapper<JS::PersistentRootedObject>& wrap = details::get_instrusive_wrapper(src);
    assert(wrap.inited());
    return *(wrap.get()); }

template <typename T>
JS::HandleObject get_intrusive_object_with_init(JSContext *c, intrusive_object<T> *src) {
    details::persistent_rooted_wrapper<JS::PersistentRootedObject>& wrap = details::get_instrusive_wrapper(src);
    if (!wrap.inited()) {
        wrap.init_with_obj(c, src); }
    return *(wrap.get()); }

}
}

#endif // MOZJS_INTRUSIVE_OBJECT_HXX
