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

    // 150819: fixed duplicated copying in argument forwarding of ctor
    template<size_t ... N>
    inline static T *callback(const std::tuple<typename caster<Args>::backT ...>& args, indices<N ...>) {
        return new T(std::get<N>(args) ...); }

    // for placement new
    template<size_t ... N>
    inline static T *callback(const std::tuple<typename caster<Args>::backT ...>& args, indices<N ...>, void *ptr) {
        return new (ptr) T(std::get<N>(args) ...); }
};

template <typename T, bool is_intrusive = class_is_intrusive<T>::value>
struct ctor_addon_intrusive {
    inline static void callback(JSContext *c, T *src, JS::HandleObject obj, lifetime<T> *lt) { } };

template <typename T>
struct ctor_addon_intrusive<T, true> {
    inline static void callback(JSContext *c, T *src, JS::HandleObject obj, lifetime<T> *lt) {
        get_instrusive_wrapper(src).init(c);
        get_instrusive_wrapper(src).get()->set(obj);
        lt->is_intrusive = true;
    }
};

// template template parameter ...
template <typename T, template <typename> class LifetimeT, typename ... Args>
struct lifetime_creation {
    inline lifetime<T> *operator () (const std::tuple<typename caster<Args>::backT ...>& args); };

template <typename T, typename ... Args>
struct lifetime_creation<T, lifetime_js, Args ...> {
    inline lifetime<T> *operator () (const std::tuple<typename caster<Args>::backT ...>& args) {
        lifetime<T> *t = new lifetime_js<T>(LIFETIME_PLACEMENT_CONSTRUCT);
        ctor_internal<T, Args ...>::callback(args, typename indices_builder
                <sizeof ... (Args)>::type(), t->get());
        return t;
    }
};

template <typename T, typename ... Args>
struct lifetime_creation<T, lifetime_cxx, Args ...> {
    inline lifetime<T> *operator () (const std::tuple<typename caster<Args>::backT ...>& args) {
        T *inside = ctor_internal<T, Args ...>::callback(args, typename indices_builder
                <sizeof ... (Args)>::type());
        return new lifetime_cxx<T>(inside);
    }
};

template <typename T, typename ... Args>
struct lifetime_creation<T, lifetime_shared_std, Args ...> {
    inline lifetime<T> *operator () (std::tuple<typename caster<Args>::backT ...> args) {
        std::shared_ptr<T> inside(ctor_internal<T, Args ...>::callback(args, typename indices_builder
                <sizeof ... (Args)>::type()));
        return new lifetime_shared_std<T>(inside);
    }
};

template <typename T, LifetimeType lt, typename ... Args>
struct lifetime_enumtype {
    typedef lifetime_creation<T, lifetime_js, Args ...> type; };

template <typename T, typename ... Args>
struct lifetime_enumtype<T, UseCXXLifetime, Args ...> {
    typedef lifetime_creation<T, lifetime_cxx, Args ...> type; };

template <typename T, typename ... Args>
struct lifetime_enumtype<T, UseSharedLifetime, Args ...> {
    typedef lifetime_creation<T, lifetime_shared_std, Args ...> type; };

template <typename T>
inline static bool native_reproto(JSContext *c, unsigned int argc, JS::Value *vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject self(c, JS_THIS_OBJECT(c, vp));
    JS::RootedObject proto(c, args[0].toObjectOrNull());

    args.rval().setObject(*JS_NewObjectWithGivenProto(c, class_info<T>::instance()->jsc_def, proto, JS::NullPtr()));
    return true;
}

template <typename T, typename ... Args>
struct ctor_callback {

    inline static void dtor_callback(JSFreeOp *op, JSObject *obj) {
        delete reinterpret_cast<lifetime<T> *>(JS_GetPrivate(obj)); }

    inline static bool callback_invalid(JSContext *context, unsigned int argc, JS::Value *vp) {
        return true; }

    template <LifetimeType lt>
    using lifetime_creation = typename details::lifetime_enumtype<T, lt, Args ...>::type;
    template <LifetimeType lt>
    inline static void attach_new_native(JSContext *c, JS::HandleObject self, JS::CallArgs args) {
        auto args_tuple = details::construct_args<Args ...>(c, args);
        lifetime<T> *t = lifetime_creation<lt>()(args_tuple);
        JS_SetPrivate(self, reinterpret_cast<void *>(t));
        ctor_addon_intrusive<T>::callback(c, t->get(), self, t);
    }

    template <LifetimeType lt>
    inline static bool native_attach_new(JSContext *c, unsigned int argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        JS::RootedObject self(c, JS_THIS_OBJECT(c, vp));

        attach_new_native<lt>(c, self, args);
        args.rval().set(OBJECT_TO_JSVAL(self));
        return true;
    }

    template <LifetimeType lt> // removed default parameter since it's to be used internally
    inline static bool callback(JSContext *c, unsigned int argc, JS::Value *vp) {
        JS::RootedObject proto(c, class_info<T>::instance()->jsc_proto);
        JS::RootedObject jsobj(c, JS_NewObject(c, class_info<T>::instance()->jsc_def, proto, JS::NullPtr()));

        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        attach_new_native<lt>(c, jsobj, args);

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

        JSNative ctor_callback = callbacks::template callback<lt>;
        if (use_invalid) {
            ctor_callback = callbacks::callback_invalid; }
        JS::RootedObject proto(info->context, JS_InitClass(info->context, global, parent_proto, info->jsc_def_proto,
                ctor_callback, 0, details::default_properties, details::default_funcs, nullptr, nullptr));

        info->jsc_proto = proto.get();
        JS::RootedObject ctor(info->context, JS_GetConstructor(info->context, proto));
        // TODO: it's the correct way? maybe not, it's just a magic.
        JS_SetReservedSlot(ctor, 1, OBJECT_TO_JSVAL(proto));
    }
};

}
}
}

#endif // MOZJS_SPDE_CONSTRUCTOR_HXX
