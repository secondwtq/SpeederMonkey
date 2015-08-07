//
// Created by secondwtq 15-8-7.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_SPDE_PERSISTENT_WRAP_HXX
#define MOZJS_SPDE_PERSISTENT_WRAP_HXX

#include "../../thirdpt/js_engine.hxx"

#include "../spde_classinfo.hxx"
#include "../spde_vivalavida.hxx"

#include <cassert>

namespace xoundation {
namespace spd {
namespace details {

// not to be used externally
//  since we only performed simple asserts
template <typename T>
struct persistent_rooted_wrap {

    typedef JS::PersistentRooted<T> internalT;

    persistent_rooted_wrap() : m_impl(nullptr) { }
    // TODO: needs more test on copy ctor and assignment op.
    persistent_rooted_wrap(const persistent_rooted_wrap<T>& other)
            : m_impl(init_with_other(other)) { }
    ~persistent_rooted_wrap() {
        if (inited()) {
            delete m_impl; }
    }

    persistent_rooted_wrap<T>& operator = (const persistent_rooted_wrap<T>& other) {
        m_impl = init_with_other(other); }

    bool inited() const { return (m_impl != nullptr); }
    internalT *get() {
        // getting the pointer without initialization
        //  makes no sense
        assert(m_impl != nullptr);
        return m_impl;
    }

    persistent_rooted_wrap<T>& init(JSContext *c) {
        assert(m_impl == nullptr);
        m_impl = new internalT(c);
        return *this;
    }

    persistent_rooted_wrap<T>& init_with_jsobj(JSContext *c, JS::Handle<T> obj) {
        assert(m_impl == nullptr);
        m_impl = new internalT(c, obj);
        return *this;
    }

    persistent_rooted_wrap<T>& init_with_persistent_rooted(JSContext *c, const JS::PersistentRooted<T>& obj) {
        assert(m_impl == nullptr);
        m_impl = new internalT(c, &obj.get());
        return *this;
    }

    // donot use this within JS ctor!
    //  it assumes the lifetime is managed by CXX!
    //  cuz if an intrusived object is not initialized,
    //  it must be constructed in CXX
    //
    // an object created in JS would be automatically initialized
    //  by ctor_addon_intrusive<T, true>::callback.
    template <typename NativeT>
    persistent_rooted_wrap<T>& init_with_nativeobj(JSContext *c, NativeT *src) {
        assert(m_impl == nullptr);

        JS::RootedObject proto(c, class_info<NativeT>::instance()->jsc_proto);
        JS::RootedObject jsobj(c, JS_NewObject(c, class_info<T>::instance()->jsc_def, proto, JS::NullPtr()));
        lifetime <NativeT> *lt = new lifetime_cxx<NativeT>(src);
        JS_SetPrivate(jsobj, reinterpret_cast<void *>(lt));
        return this->init_with_jsobj(c, jsobj);
    }

private:
    internalT *m_impl = nullptr;

    static internalT *init_with_other(const persistent_rooted_wrap<T>& other) {
        return other.inited() ? new internalT(other) : nullptr; }
};

}
}
}

#endif // MOZJS_SPDE_PERSISTENT_WRAP_HXX
