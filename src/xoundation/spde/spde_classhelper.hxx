//
// Created by secondwtq 15-5-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef HEADER_SPEEDER_MONKEY_CLS_HELPER
#define HEADER_SPEEDER_MONKEY_CLS_HELPER

#include "../thirdpt/js_engine.hxx"

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

template<typename T>
class class_helper {
    private:

    using info_t = class_info<T>;
    template<typename PropT, PropT *AttrT>
    inline static bool default_getter_static(JSContext *context, unsigned int argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        args.rval().set(caster<PropT>::tojs(context, *AttrT));
        return true;
    }

    template<typename PropT, PropT *AttrT>
    inline static bool default_setter_static(JSContext *context, unsigned argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        *AttrT = caster<PropT>::back(context, args[0]);
        return true;
    }

    public:

    // TODO: readonly feature
    template<typename PropT, PropT T::*AttrT>
    inline static void reg_property(const std::string& name, bool readonly = false) {
        info_t *info = info_t::instance();

        JS::RootedObject proto(info->context, info->jsc_proto);
        JS_DefineProperty(info->context, proto, name.c_str(), JS::UndefinedHandleValue,
                          JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_SHARED |
                          JSPROP_NATIVE_ACCESSORS, reinterpret_cast<JSPropertyOp>
                          (&(details::property_accessor<T, PropT>::template default_getter<AttrT>)),
                          reinterpret_cast<JSStrictPropertyOp>
                          (&(details::property_accessor<T, PropT>::template default_setter<AttrT>)));
    }

    template<typename ProtoT, ProtoT func>
    struct method_callback_wrapper;

    template<typename ReturnT, typename ... Args, ReturnT (T::*func)(Args ...)>
    struct method_callback_wrapper<ReturnT (T::*)(Args ...), func> {

        template<typename U = ReturnT>
        inline static bool callback(JSContext *context, unsigned int argc, JS::Value *vp) {
            JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
            auto args_tuple = details::construct_args<typename caster<Args>::backT ...>(context, args);
            T *raw = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)))->get();
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
            auto args_tuple = details::construct_args<typename caster<Args>::backT ...>(context, args);
            const T *raw = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)))->get();
            return details::method_callback_wrapper<ReturnT(T::*)(Args ...) const, func, std::is_void<ReturnT>::value>::callback
                    (raw, context, args, args_tuple, typename indices_builder<sizeof ... (Args)>::type());
        }

        inline static void register_as(const std::string& name) {
            info_t *info = info_t::instance();
            JS::RootedObject proto(info->context, info->jsc_proto);
            JS_DefineFunction(info->context, proto, name.c_str(),
                              method_callback_wrapper<ReturnT (T::*)(Args ...) const, func>::callback, static_cast<unsigned
                    int>(sizeof ... (Args)), JSPROP_PERMANENT | JSPROP_ENUMERATE | JSFUN_STUB_GSOPS);
        }

    };

    template <bool (T::*func)(JSContext *, JS::CallArgs)>
    inline static bool raw_method_callback(JSContext *context, unsigned int argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        T *raw = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)))->get();
        return (raw->*func)(context, args);
    }

    template <typename ... Args>
    inline class_helper<T> define(JS::HandleObject global, bool use_invalid = false, JS::HandleObject parent_proto = JS::NullPtr()) {
        info_t *info = info_t::instance();
        using callbacks = details::ctor_callback<T, Args ...>;

        info->jsc_def->finalize = callbacks::dtor_callback;

        JSNative ctor_callback = callbacks::callback;
        if (use_invalid) {
            ctor_callback = callbacks::callback_invalid; }
        info->jsc_proto = JS_InitClass(info->context, global, parent_proto, info->jsc_def_proto,
                                       ctor_callback, 0, details::default_properties,
                                       details::default_funcs, nullptr, nullptr);

        return *this;
    }

    // overloaded helper for inheritance
    template<typename ParentT, typename ... Args>
    inline class_helper<T> inherits(JS::HandleObject global, bool use_invalid = false) {
        JS::RootedObject parent_proto(info_t::instance()->context,
                                      class_info<ParentT>::instance()->jsc_proto);
        // so how is it forwarded without this by default?
        define<Args ...>(global, use_invalid, parent_proto);

        return *this;
    }

    template<typename PropT, PropT T::*AttrT>
    inline class_helper<T> property(const std::string& name) {
        class_helper<T>::reg_property<PropT, AttrT>(name);
        return *this;
    }

    template<typename PropT, PropT (T::*Getter)(), void (T::*Setter)(PropT)>
    inline class_helper<T> accessor(const std::string& name, bool readonly = false) {
        info_t *info = info_t::instance();

        JS::RootedObject proto(info->context, info->jsc_proto);
        JS_DefineProperty(info->context, proto, name.c_str(), JS::UndefinedHandleValue,
            JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,
            reinterpret_cast<JSPropertyOp>
            (&(details::property_accessor_general<T, PropT>::template getter<Getter>)),
            reinterpret_cast<JSStrictPropertyOp>
            (&(details::property_accessor_general<T, PropT>::template setter<Setter>)));

        return *this;
    }

    template<typename ProtoT, ProtoT func>
    inline class_helper<T> method(const std::string& name) {
        method_callback_wrapper<ProtoT, func>::register_as(name);
        return *this;
    }

    // static members support, 150529 EVE
    template<typename PropT, PropT *AttrT>
    inline class_helper<T> static_prop(const std::string& name) {
        info_t *info = info_t::instance();
        JS::RootedObject proto(info->context, info->jsc_proto);
        JS::RootedObject ctor(info->context, JS_GetConstructor(info->context, proto));
        JS_DefineProperty(info->context, ctor, name.c_str(), JS::UndefinedHandleValue,
                          JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,
                          reinterpret_cast<JSPropertyOp>(default_getter_static<PropT, AttrT>),
                          reinterpret_cast<JSStrictPropertyOp>(default_setter_static<PropT, AttrT>));

        return *this;
    }

    template<typename ProtoT, ProtoT& func>
    inline class_helper<T> static_func(const std::string& name) {
        info_t *info = info_t::instance();
        JS::RootedObject proto(info->context, info->jsc_proto);
        JS::RootedObject ctor(info->context, JS_GetConstructor(info->context, proto));
        function_callback_wrapper<ProtoT, func>::register_func(info->context, ctor, name.c_str());

        return *this;
    }

    template <bool func (JSContext *, unsigned, JS::Value *)>
    inline class_helper<T> raw_static(const std::string& name) {
        info_t *info = info_t::instance();
        JS::RootedObject proto(info->context, info->jsc_proto);
        JS::RootedObject ctor(info->context, JS_GetConstructor(info->context, proto));
        JS_DefineFunction(info->context, ctor, name.c_str(), func, 0,
                          JSPROP_PERMANENT | JSPROP_ENUMERATE | JSFUN_STUB_GSOPS);
        return *this;
    }

    template <bool (T::*func)(JSContext *, JS::CallArgs)>
    inline class_helper<T> raw_method(const std::string& name, size_t nargs = 0) {
        info_t *info = info_t::instance();
        JS::RootedObject proto(info->context, info->jsc_proto);
        JS::RootedObject ctor(info->context, JS_GetConstructor(info->context, proto));
        JS_DefineFunction(info->context, ctor, name.c_str(), raw_method_callback<func>, nargs,
                        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSFUN_STUB_GSOPS);

        return *this;
    }

};


}
}

#endif
