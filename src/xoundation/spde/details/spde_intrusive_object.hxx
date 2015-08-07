//
// Created by secondwtq 15-8-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_INTRUSIVE_OBJECT_HXX
#define MOZJS_INTRUSIVE_OBJECT_HXX

#include "../../thirdpt/js_engine.hxx"

#include "spde_persistent_wrap.hxx"

namespace xoundation {
namespace spd {

// forward declarations
template <typename T>
struct intrusive_object;

namespace details {
template <typename T>
persistent_rooted_wrap<JSObject *>&
        get_instrusive_wrapper(intrusive_object<T> *src);
}

template <typename T>
struct intrusive_object {
private:

    details::persistent_rooted_wrap<JSObject *> unique_obj;

    template <typename T_>
    friend details::persistent_rooted_wrap<JSObject *> &
            details::get_instrusive_wrapper(intrusive_object<T_> *);

};

namespace details {

template <typename T>
inline details::persistent_rooted_wrap<JSObject *> &
        get_instrusive_wrapper(intrusive_object<T> *src) {
    return src->unique_obj; }

}

// what do you think of the type of it's return value?
template <typename T>
JS::HandleObject get_intrusive_object(intrusive_object<T> *src) {
    details::persistent_rooted_wrap<JSObject *>& wrap = details::get_instrusive_wrapper(src);
    assert(wrap.inited());
    return *(wrap.get()); }

template <typename T>
JS::HandleObject get_intrusive_object_with_init(JSContext *c, intrusive_object<T> *src) {
    details::persistent_rooted_wrap<JSObject *>& wrap = details::get_instrusive_wrapper(src);
    if (!wrap.inited()) {
        // is it a bug? ->
        // TODO: needs test, and must written in CXX.
        wrap.init_with_nativeobj(c, reinterpret_cast<T *>(src)); }
    return *(wrap.get()); }

}
}

#endif // MOZJS_INTRUSIVE_OBJECT_HXX
