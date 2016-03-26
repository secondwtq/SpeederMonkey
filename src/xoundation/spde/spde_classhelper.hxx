//
// Created by secondwtq 15-5-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef HEADER_SPEEDER_MONKEY_CLS_HELPER
#define HEADER_SPEEDER_MONKEY_CLS_HELPER

#include "../thirdpt/js_engine.hxx"

#include "spde_common.hxx"
#include "spde_classinfo.hxx"
#include "spde_caster.hxx"
#include "spde_functionbind.hxx"

#include <string>
#include <type_traits>

#include "./details/spde_constructor.hxx"
#include "./details/spde_method_callback_wrapper.hxx"
#include "./details/spde_property_accessor.hxx"

namespace xoundation {
namespace spd {

// stackoverflow.com/questions/9831501/how-can-i-have-multiple-parameter-packs-in-a-variadic-template
//  or the compiler will reports some bullshit
template <typename ... Args>
struct argpack { };

template<typename T>
class class_helper {
    private:

    using info_t = class_info<T>;

    template <LifetimeType lt, typename ... Args>
    using class_def = details::class_def<T, lt, Args ...>;

    public:

    template <typename ... Args, LifetimeType lt = UseJSLifetime>
    inline SPD_PUBLICAPI class_helper<T> define(JS::HandleObject global, bool use_invalid = false) {
        class_def<lt, Args ...>()(global, use_invalid, JS::NullPtr());
        return *this;
    }

    template <LifetimeType lt = UseJSLifetime,
            template <typename ...> class packtype, typename ... Args>
    inline SPD_PUBLICAPI class_helper<T> define(JS::HandleObject global, packtype<Args ...> p, bool use_invalid = false) {
        class_def<lt, Args ...>()(global, use_invalid, JS::NullPtr());
        return *this;
    }

    // overloaded helper for inheritance
    template<typename ParentT, LifetimeType lt = UseJSLifetime,
            template <typename ...> class packtype, typename ... Args>
    inline SPD_PUBLICAPI class_helper<T> inherits(JS::HandleObject global, packtype<Args ...> p, bool use_invalid = false) {
        JS::RootedObject parent_proto(info_t::instance()->context,
                                      class_info<ParentT>::instance()->jsc_proto);
        // so how is it forwarded without this by default?
        class_def<lt, Args ...>()(global, use_invalid, parent_proto);
        return *this;
    }

    template<LifetimeType lt = UseJSLifetime,
            template <typename ...> class packtype, typename ... Args>
    inline SPD_PUBLICAPI class_helper<T> attach(const std::string& name, packtype<Args ...> p) {
        info_t *info = info_t::instance();
        JS::RootedObject proto(info->context, info->jsc_proto);
        JS::RootedObject ctor(info->context, JS_GetConstructor(info->context, proto));
        JS_DefineFunction(info->context, ctor, name.c_str(),
                details::ctor_callback<T, Args ...>::template native_attach_new<lt>,
                static_cast<unsigned int>(sizeof ... (Args)),
                JSPROP_PERMANENT | JSPROP_ENUMERATE | JSFUN_STUB_GSOPS);
        return *this;
    };

    inline SPD_PUBLICAPI class_helper<T> reproto(const std::string& name) {
        info_t *info = info_t::instance();
        JS::RootedObject proto(info->context, info->jsc_proto);
        JS::RootedObject ctor(info->context, JS_GetConstructor(info->context, proto));
        JS_DefineFunction(info->context, ctor, name.c_str(), details::native_reproto<T>,
                1, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSFUN_STUB_GSOPS);
        return *this;
    };

    // TODO: readonly feature
    template<typename PropT, PropT T::*AttrT>
    inline SPD_PUBLICAPI class_helper<T> property(const std::string& name) {
        info_t *info = info_t::instance();

        JS::RootedObject proto(info->context, info->jsc_proto);
        JS_DefineProperty(info->context, proto, name.c_str(), JS::UndefinedHandleValue,
                          JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_SHARED |
                          espwrap::PROP_NATIVE_ACCESSOR, reinterpret_cast<espwrap::TypeNativeGetter>
                          (&(details::property_accessor<T, PropT>::template default_getter<AttrT>)),
                          reinterpret_cast<espwrap::TypeNativeSetter>
                          (&(details::property_accessor<T, PropT>::template default_setter<AttrT>)));
        return *this;
    }

    // maybe we need something like 'readonly'
    template<typename PropT, PropT (T::*Getter)()>
    inline SPD_PUBLICAPI class_helper<T> accessor(const std::string& name, bool readonly = false) {
        info_t *info = info_t::instance();

        JS::RootedObject proto(info->context, info->jsc_proto);
        JS_DefineProperty(info->context, proto, name.c_str(), JS::UndefinedHandleValue,
                JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_SHARED | espwrap::PROP_NATIVE_ACCESSOR | JSPROP_READONLY,
                reinterpret_cast<JSPropertyOp>
                (details::property_accessor_general<T, PropT>::template getter<Getter>), nullptr);

        return *this;
    }

    template<typename PropT, PropT (T::*Getter)() const>
    inline SPD_PUBLICAPI class_helper<T> accessor(const std::string& name, bool readonly = false) {
        info_t *info = info_t::instance();

        JS::RootedObject proto(info->context, info->jsc_proto);
        JS_DefineProperty(info->context, proto, name.c_str(), JS::UndefinedHandleValue,
                JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_SHARED | espwrap::PROP_NATIVE_ACCESSOR | JSPROP_READONLY,
                reinterpret_cast<JSPropertyOp>
                (details::property_accessor_general<T, PropT>::template getter<Getter>), nullptr);

        return *this;
    }

    // for now if you forced 'nullptr' on these functions
    //  it would lead to a crash of calling NULL
    //  when the corresponding access takes place
    //
    // what pleases me is that, the 'error message'
    //  (crash stacktrace effectively) is clear enough
    //  (though the symbols is a little bit tooo looong)
    template<typename PropT, PropT (T::*Getter)() const, void (T::*Setter)(PropT)>
    inline SPD_PUBLICAPI class_helper<T> accessor(const std::string& name, bool readonly = false) {
        info_t *info = info_t::instance();

        JS::RootedObject proto(info->context, info->jsc_proto);
        JS_DefineProperty(info->context, proto, name.c_str(), JS::UndefinedHandleValue,
            JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_SHARED | espwrap::PROP_NATIVE_ACCESSOR,
            reinterpret_cast<espwrap::TypeNativeGetter>
            (details::property_accessor_general<T, PropT>::template getter<Getter>),
            reinterpret_cast<espwrap::TypeNativeSetter>
            (details::property_accessor_general<T, PropT>::template setter<void, Setter>));

        return *this;
    }

    template<typename PropT, PropT (T::*Getter)(), void (T::*Setter)(PropT)>
    inline SPD_PUBLICAPI class_helper<T> accessor(const std::string& name, bool readonly = false) {
        info_t *info = info_t::instance();

        JS::RootedObject proto(info->context, info->jsc_proto);
        JS_DefineProperty(info->context, proto, name.c_str(), JS::UndefinedHandleValue,
                JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_SHARED | espwrap::PROP_NATIVE_ACCESSOR,
                reinterpret_cast<espwrap::TypeNativeGetter>
                (details::property_accessor_general<T, PropT>::template getter<Getter>),
                reinterpret_cast<espwrap::TypeNativeSetter>
                (details::property_accessor_general<T, PropT>::template setter<void, Setter>));

        return *this;
    }

    // we need support setters returning both void and T&
    template<typename PropT, PropT (T::*Getter)(), T& (T::*Setter)(PropT)>
    inline SPD_PUBLICAPI class_helper<T> accessor(const std::string& name, bool readonly = false) {
        info_t *info = info_t::instance();

        JS::RootedObject proto(info->context, info->jsc_proto);
        JS_DefineProperty(info->context, proto, name.c_str(), JS::UndefinedHandleValue,
                JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_SHARED | espwrap::PROP_NATIVE_ACCESSOR,
                reinterpret_cast<espwrap::TypeNativeGetter>
                (details::property_accessor_general<T, PropT>::template getter<Getter>),
                reinterpret_cast<espwrap::TypeNativeSetter>
                (details::property_accessor_general<T, PropT>::template setter<Setter>));

        return *this;
    }

    template<typename PropT, PropT (T::*Getter)() const, T& (T::*Setter)(PropT)>
    inline SPD_PUBLICAPI class_helper<T> accessor(const std::string& name, bool readonly = false) {
        info_t *info = info_t::instance();

        JS::RootedObject proto(info->context, info->jsc_proto);
        JS_DefineProperty(info->context, proto, name.c_str(), JS::UndefinedHandleValue,
                JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_SHARED | espwrap::PROP_NATIVE_ACCESSOR,
                reinterpret_cast<espwrap::TypeNativeGetter>
                (details::property_accessor_general<T, PropT>::template getter<Getter>),
                reinterpret_cast<espwrap::TypeNativeSetter>
                (details::property_accessor_general<T, PropT>::template setter<T&, Setter>));

        return *this;
    }

    template<typename ProtoT, ProtoT func>
    inline SPD_PUBLICAPI class_helper<T> method(const std::string& name) {
        method_callback_wrapper<ProtoT, func>::register_as(name);
        return *this;
    }

    template<typename ProtoT, ProtoT func>
    inline SPD_PUBLICAPI class_helper<T> ext_method(const std::string& name) {
        ext_method_callback_wrapper<ProtoT, func>::register_as(name);
        return *this;
    }

    // static members support, 150529 EVE
    template<typename PropT, PropT *AttrT>
    inline SPD_PUBLICAPI class_helper<T> static_prop(const std::string& name) {
        info_t *info = info_t::instance();
        JS::RootedObject proto(info->context, info->jsc_proto);
        JS::RootedObject ctor(info->context, JS_GetConstructor(info->context, proto));
        JS_DefineProperty(info->context, ctor, name.c_str(), JS::UndefinedHandleValue,
                          JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_SHARED | espwrap::PROP_NATIVE_ACCESSOR,
                          reinterpret_cast<espwrap::TypeNativeGetter>
                          (details::static_accessor<T, PropT>::template default_getter<AttrT>),
                          reinterpret_cast<espwrap::TypeNativeSetter>
                          (details::static_accessor<T, PropT>::template default_setter<AttrT>));

        return *this;
    }

    template<typename ProtoT, ProtoT& func>
    inline SPD_PUBLICAPI class_helper<T> static_func(const std::string& name) {
        info_t *info = info_t::instance();
        JS::RootedObject proto(info->context, info->jsc_proto);
        JS::RootedObject ctor(info->context, JS_GetConstructor(info->context, proto));
        function_callback_wrapper<ProtoT, func>::register_func(info->context, ctor, name.c_str());

        return *this;
    }

    template <bool func (JSContext *, unsigned, JS::Value *)>
    inline SPD_PUBLICAPI class_helper<T> raw_static(const std::string& name) {
        info_t *info = info_t::instance();
        JS::RootedObject proto(info->context, info->jsc_proto);
        JS::RootedObject ctor(info->context, JS_GetConstructor(info->context, proto));
        JS_DefineFunction(info->context, ctor, name.c_str(), func, 0,
                          JSPROP_PERMANENT | JSPROP_ENUMERATE | JSFUN_STUB_GSOPS);
        return *this;
    }

    template <bool (T::*func)(JSContext *, JS::CallArgs)>
    inline SPD_PUBLICAPI class_helper<T> raw_method(const std::string& name, size_t nargs = 0) {
        info_t *info = info_t::instance();
        JS::RootedObject proto(info->context, info->jsc_proto);
        JS::RootedObject ctor(info->context, JS_GetConstructor(info->context, proto));
        JS_DefineFunction(info->context, ctor, name.c_str(), raw_method_callback<func>, nargs,
                        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSFUN_STUB_GSOPS);

        return *this;
    }

private:

    template<typename ProtoT, ProtoT func>
    struct ext_method_callback_wrapper;

    template<typename ReturnT, typename ... Args, ReturnT (func)(T *, Args ...)>
    struct ext_method_callback_wrapper<ReturnT (T *, Args ...), func> {

        template<typename U = ReturnT>
        inline static bool callback(JSContext *context, unsigned int argc, JS::Value *vp) {
            JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
            auto args_tuple = details::construct_args<Args ...>(context, args);
            T *raw = reinterpret_cast<spd::lifetime<T> *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)))->get();
            return details::raw_method_callback_wrapper<ReturnT(T *, Args ...),
                    func, std::is_void<ReturnT>::value>::callback
                    (raw, context, args, args_tuple, typename indices_builder<sizeof ... (Args)>::type());
        }

        inline static void register_as(const std::string& name) {
            info_t *info = info_t::instance();
            JS::RootedObject proto(info->context, info->jsc_proto);
            JS_DefineFunction(info->context, proto, name.c_str(),
                    ext_method_callback_wrapper<ReturnT (T *, Args ...), func>::callback, static_cast<unsigned
                    int>(sizeof ... (Args)), JSPROP_PERMANENT | JSPROP_ENUMERATE | JSFUN_STUB_GSOPS);
        }

    };

    template<typename ReturnT, typename ... Args, ReturnT (func)(const T *, Args ...)>
    struct ext_method_callback_wrapper<ReturnT (const T *, Args ...), func> {

        template<typename U = ReturnT>
        inline static bool callback(JSContext *context, unsigned int argc, JS::Value *vp) {
            JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
            auto args_tuple = details::construct_args<Args ...>(context, args);
            T *raw = reinterpret_cast<spd::lifetime<T> *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)))->get();
            return details::raw_method_callback_wrapper<ReturnT(const T *, Args ...),
                    func, std::is_void<ReturnT>::value>::callback
                    (raw, context, args, args_tuple, typename indices_builder<sizeof ... (Args)>::type());
        }

        inline static void register_as(const std::string& name) {
            info_t *info = info_t::instance();
            JS::RootedObject proto(info->context, info->jsc_proto);
            JS_DefineFunction(info->context, proto, name.c_str(),
                    ext_method_callback_wrapper<ReturnT (const T *, Args ...), func>::callback, static_cast<unsigned
                    int>(sizeof ... (Args)), JSPROP_PERMANENT | JSPROP_ENUMERATE | JSFUN_STUB_GSOPS);
        }

    };

    template <bool (T::*func)(JSContext *, JS::CallArgs)>
    inline static bool raw_method_callback(JSContext *context, unsigned int argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        T *raw = reinterpret_cast<spd::lifetime<T> *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)))->get();
        return (raw->*func)(context, args);
    }

    template<typename ProtoT, ProtoT func>
    struct method_callback_wrapper;

    template<typename ReturnT, typename ... Args, ReturnT (T::*func)(Args ...)>
    struct method_callback_wrapper<ReturnT (T::*)(Args ...), func> {

        template<typename U = ReturnT>
        inline static bool callback(JSContext *context, unsigned int argc, JS::Value *vp) {
            JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
            auto args_tuple = details::construct_args<Args ...>(context, args);
            T *raw = reinterpret_cast<spd::lifetime<T> *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)))->get();
            return details::method_callback_wrapper<ReturnT(T::*)(Args ...), func, std::is_void<ReturnT>::value>::callback
                    (raw, context, args, args_tuple, typename indices_builder<sizeof ... (Args)
                    >::type());
        }

        inline static void register_as(const std::string& name) {
            info_t *info = info_t::instance();
            JS::RootedObject proto(info->context, info->jsc_proto);
            JS_DefineFunction(info->context, proto, name.c_str(),
                              method_callback_wrapper<ReturnT (T::*)(Args ...), func>::callback, static_cast<unsigned
                    int>(sizeof ... (Args)), JSPROP_PERMANENT | JSPROP_ENUMERATE | JSFUN_STUB_GSOPS);
        }

    };

    template<typename ReturnT, typename ... Args, ReturnT (T::*func)(Args ...) const>
    struct method_callback_wrapper<ReturnT (T::*)(Args ...) const, func> {

        template<typename U = ReturnT>
        inline static bool callback(JSContext *context, unsigned int argc, JS::Value *vp) {
            JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
            auto args_tuple = details::construct_args<Args ...>(context, args);
            const T *raw = reinterpret_cast<spd::lifetime<T> *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)))->get();
            return details::method_callback_wrapper<ReturnT(T::*)(Args ...) const, func, std::is_void<ReturnT>::value>::callback
                    (raw, context, args, args_tuple, typename indices_builder<sizeof ... (Args)>::type());
        }

        inline static void register_as(const std::string& name) {
            info_t *info = info_t::instance();
            JS::RootedObject proto(info->context, info->jsc_proto);
            JS_DefineFunction(info->context, proto, name.c_str(),
                              method_callback_wrapper<ReturnT (T::*)(Args ...) const, func>::callback,
                              static_cast<unsigned int>(sizeof ... (Args)),
                              JSPROP_PERMANENT | JSPROP_ENUMERATE | JSFUN_STUB_GSOPS);
        }

    };

};


}
}

#endif
