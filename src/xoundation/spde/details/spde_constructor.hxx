//
// Created by secondwtq 15-8-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_SPDE_CONSTRUCTOR_HXX
#define MOZJS_SPDE_CONSTRUCTOR_HXX

#include "../../thirdpt/js_engine.hxx"

#include "../spde_common.hxx"
#include "../spde_heroes.hxx"
#include "../spde_vivalavida.hxx"
#include "spde_intrusive_object.hxx"
#include "../spde_classinfo.hxx"
#include "../spde_caster.hxx"
#include "../spde_functionbind.hxx"

#include <tuple>

namespace xoundation {
namespace spd {
namespace details {

template<typename T, typename ... Args>
struct ctor_internal {
    using info_t = class_info<T>;

    template<size_t ... N>
    inline static T *callback(std::tuple<Args ...> args, indices<N ...>) {
        return new T(std::get<N>(args) ...); }

    // for placement new
    template<size_t ... N>
    inline static T *callback(std::tuple<Args ...> args, indices<N ...>, void *ptr) {
        return new (ptr) T(std::get<N>(args) ...); }
};

template <typename T, bool is_intrusive = std::is_base_of<intrusive_object_base, T>::value>
struct ctor_addon_intrusive {
    inline static void callback(JSContext *c, T *src, JS::HandleObject obj) { } };

template <typename T>
struct ctor_addon_intrusive<T, true> {
    inline static void callback(JSContext *c, T *src, JS::HandleObject obj) {
        get_instrusive_wrapper(src).init(c);
        get_instrusive_wrapper(src).get()->set(obj);
    }
};

// template template parameter ...
template <typename T, template <typename> class LifetimeT, typename ... Args>
struct lifetime_creation {
    inline lifetime<T> *operator () (std::tuple<typename caster<Args>::backT ...> args); };

template <typename T, typename ... Args>
struct lifetime_creation<T, lifetime_js, Args ...> {
    inline lifetime<T> *operator () (std::tuple<typename caster<Args>::backT ...> args) {
        lifetime<T> *t = new lifetime_js<T>(LIFETIME_PLACEMENT_CONSTRUCT);
        ctor_internal<T, Args ...>::callback(args, typename indices_builder
                <sizeof ... (Args)>::type(), t->get());
        return t;
    }
};

template <typename T, typename ... Args>
struct lifetime_creation<T, lifetime_cxx, Args ...> {
    inline lifetime<T> *operator () (std::tuple<typename caster<Args>::backT ...> args) {
        T *inside = ctor_internal<T, Args ...>::callback(args, typename indices_builder
                <sizeof ... (Args)>::type());
        return new lifetime_cxx<T>(inside);
    }
};

template <typename T, bool use_js_lifetime, typename ... Args>
struct lifetime_booltype {
    typedef lifetime_creation<T, lifetime_js, Args ...> type; };
template <typename T, typename ... Args>
struct lifetime_booltype<T, false, Args ...> {
    typedef lifetime_creation<T, lifetime_cxx, Args ...> type; };

template <typename T, typename ... Args>
struct ctor_callback {

    inline static void dtor_callback(JSFreeOp *op, JSObject *obj) {
        delete reinterpret_cast<lifetime<T> *>(JS_GetPrivate(obj)); }

    inline static bool callback_invalid(JSContext *context, unsigned int argc, JS::Value *vp) {
        return true; }

    template <bool use_js_lifetime>
    using lifetime_booltype = typename details::lifetime_booltype<T, use_js_lifetime, Args ...>::type;
    template <bool jslifetime> // removed default parameter since it to be used internally
    inline static bool callback(JSContext *c, unsigned int argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        auto args_tuple = details::construct_args<typename caster<Args>::backT ...>(c, args);

        lifetime<T> *t = lifetime_booltype<jslifetime>()(args_tuple);

        JS::RootedObject proto(c, class_info<T>::instance()->jsc_proto);
        JS::RootedObject jsobj(c, JS_NewObject(c, class_info<T>::instance()->jsc_def, proto, JS::NullPtr()));
        JS_SetPrivate(jsobj, reinterpret_cast<void *>(t));
        ctor_addon_intrusive<T>::callback(c, t->get(), jsobj);

        args.rval().set(OBJECT_TO_JSVAL(jsobj));
        return true;
    }
};

template <typename T, LifetimeType lt, typename ... Args>
struct class_def {
    inline void operator () (JS::HandleObject global, bool use_invalid = false, JS::HandleObject parent_proto = JS::NullPtr()) {
        class_info<T> *info = class_info<T>::instance();
        using callbacks = details::ctor_callback<T, Args ...>;

        info->jsc_def->finalize = callbacks::dtor_callback;

        JSNative ctor_callback = callbacks::template callback<lt == UseJSLifetime>;
        if (use_invalid) {
            ctor_callback = callbacks::callback_invalid; }
        info->jsc_proto = JS_InitClass(info->context, global, parent_proto, info->jsc_def_proto,
                                       ctor_callback, 0, details::default_properties,
                                       details::default_funcs, nullptr, nullptr);
    }
};

}
}
}

#endif // MOZJS_SPDE_CONSTRUCTOR_HXX
