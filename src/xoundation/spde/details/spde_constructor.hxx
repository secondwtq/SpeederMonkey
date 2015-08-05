//
// Created by secondwtq 15-8-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_SPDE_CONSTRUCTOR_HXX
#define MOZJS_SPDE_CONSTRUCTOR_HXX

#include "../../thirdpt/js_engine.hxx"

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

template <typename T, bool is_intrusive = std::is_base_of<intrusive_object<T>, T>::value>
struct ctor_addon_intrusive {
    inline static void callback(JSContext *c, T *src, JS::HandleObject obj) { } };

template <typename T>
struct ctor_addon_intrusive<T, true> {
    inline static void callback(JSContext *c, T *src, JS::HandleObject obj) {
        get_instrusive_wrapper(src).init(c);
        get_instrusive_wrapper(src).get()->set(obj);
    }
};

template <typename T, typename ... Args>
struct ctor_callback {

    inline static void dtor_callback(JSFreeOp *op, JSObject *obj) {
        lifetime<T> *raw = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(obj));
        delete raw; }

    inline static bool callback_invalid(JSContext *context, unsigned int argc, JS::Value *vp) {
        return true; }

    inline static bool callback(JSContext *c, unsigned int argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        auto args_tuple = details::construct_args<typename caster<Args>::backT ...>(c, args);

        lifetime<T> *t = new lifetime_js<T>(LIFETIME_PLACEMENT_CONSTRUCT);
        ctor_internal<T, Args ...>::callback(args_tuple, typename indices_builder
                <sizeof ... (Args)>::type(), t->get());

        JS::RootedObject proto(c, class_info<T>::instance()->jsc_proto);
        JS::RootedObject jsobj(c, JS_NewObject(c, class_info<T>::instance()->jsc_def, proto, JS::NullPtr()));
        JS_SetPrivate(jsobj, reinterpret_cast<void *>(t));
        ctor_addon_intrusive<T>::callback(c, t->get() , jsobj);

        args.rval().set(OBJECT_TO_JSVAL(jsobj));
        return true;
    }
};

}
}
}

#endif // MOZJS_SPDE_CONSTRUCTOR_HXX
