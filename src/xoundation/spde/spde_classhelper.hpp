//
// Created by secondwtq 15-5-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef HEADER_SPEEDER_MONKEY_CLS_HELPER
#define HEADER_SPEEDER_MONKEY_CLS_HELPER

#include <jsapi.h>

#include "spde_classinfo.hpp"
#include "spde_caster.hpp"
#include "spde_functionbind.hpp"

#include <string>

namespace spd {

namespace details {

template<typename ProtoT, ProtoT func>
struct method_callback_wrapper;

template<typename T, typename ReturnT, typename ... Args, ReturnT (T::*func)(Args ...)>
struct method_callback_wrapper<ReturnT (T::*)(Args ...), func> {
    template<size_t ... N>
    inline static bool callback(T *self, JSContext *context, const JS::CallArgs& call_args,
                                std::tuple<typename caster<Args>::backT ...> args, indices<N ...>) {
        call_args.rval().set(caster<ReturnT>::tojs(context, (self->*func)(std::get<N>(args) ...)));
        return true;
    }
};

template<typename T, typename ... Args, void (T::*func)(Args ...)>
struct method_callback_wrapper<void (T::*)(Args ...), func> {
    template<size_t ... N>
    inline static bool callback(T *self, JSContext *context, const JS::CallArgs& call_args,
                                std::tuple<typename caster<Args>::backT ...> args, indices<N ...>) {
        (self->*func)(std::get<N>(args) ...);
        call_args.rval().setUndefined();
        return true;
    }
};

}

template<typename T>
class class_helper {
    private:

        using info_t = class_info<T>;

        template <typename PropT, PropT T::*AttrT>
        inline static bool default_getter(JSContext *context, unsigned int argc, JS::Value *vp) {
            JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
            T *raw = reinterpret_cast<T *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)));
            args.rval().set(caster<PropT>::tojs(context, raw->*AttrT));
            return true;
        }

        template <typename PropT, PropT T::*AttrT>
        inline static bool default_setter(JSContext *context, unsigned int argc, JS::Value *vp) {
            JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
            T *raw = reinterpret_cast<T *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)));
            raw->*AttrT = caster<PropT>::back(context, args[0].address());

            return true;
        }

    // never used since we have custom ctor now
    /*
        inline static bool ctor_default(JSContext *context, unsigned int argc, JS::Value *vp) {
            JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
            T *raw = new T();
            args.rval().set(caster<T *>::tojs(context, raw));
            return true; // do not forget this!
        }
    */

    public:

        // TODO: readonly feature
        template<typename PropT, PropT T::*AttrT>
        inline static void property(const std::string& name, bool readonly = false) {
            info_t *info = info_t::instance();

            JS::RootedObject proto(info->context, info->jsc_proto);
            JS_DefineProperty(info->context, proto, name.c_str(), JS::UndefinedHandleValue,
                    JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,
                              reinterpret_cast<JSPropertyOp>(default_getter<PropT, AttrT>),
                              reinterpret_cast<JSStrictPropertyOp>(default_setter<PropT, AttrT>));
        }

        template <typename ... Args>
        struct ctor_wrapper {

            template <size_t ... N>
            inline static T *callback_internal(std::tuple<Args ...> args, indices<N ...>) {
                return new T(std::get<N>(args) ...); }

            inline static bool callback(JSContext *context, unsigned int argc, JS::Value *vp) {
                JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
                auto args_tuple = details::construct_args<typename caster<Args>::backT ...>(context, args);
                T *raw = callback_internal(args_tuple, typename indices_builder
                                            <sizeof ... (Args)>::type());
                args.rval().set(caster<T *>::tojs(context, raw));
                return true;
            }

            inline static void define(const std::string& name, JS::HandleObject global) {
                info_t *info = info_t::instance();

                JSClass *jsclass_def = reinterpret_cast<JSClass *>(malloc(sizeof(JSClass)));
                info->jsc_def = jsclass_def;
                memset(jsclass_def, 0, sizeof(JSClass)); // you *must* init it or it would
                                                            // lead to undefined behaviour
                memcpy(jsclass_def, &details::default_class_def, sizeof(JSClass));

                // TODO: dunno how the memory for the name string should be dealed with
                info->jsc_def->name = (char *)malloc((name.length() + 1) * sizeof(char));
                memcpy((char *) info->jsc_def->name, name.c_str(), name.length() * sizeof(char));
                ((char *) info->jsc_def->name)[name.length()] = '\0';

                info->jsc_proto = JS_InitClass(info->context, global, JS::NullPtr(), info->jsc_def,
                                               callback, 0, details::default_properties,
                                               details::default_funcs, nullptr, nullptr);
            }

        };

    template <typename ProtoT, ProtoT func>
    struct method_callback_wrapper;

    template <typename ReturnT, typename ... Args, ReturnT (T::*func) (Args ...)>
    struct method_callback_wrapper<ReturnT (T::*)(Args ...), func> {

        template <typename U = ReturnT>
        inline static bool callback(JSContext *context, unsigned int argc, JS::Value *vp) {
            JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
            auto args_tuple = details::construct_args<typename caster<Args>::backT ...>(context, args);
            T *raw = reinterpret_cast<T *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)));
            return details::method_callback_wrapper<ReturnT(T::*)(Args ...), func>::callback
                    (raw, context, args, args_tuple, typename indices_builder<sizeof ... (Args)
                    >::type());
        }

        static void register_as(const std::string& name) {
            info_t *info = info_t::instance();
            JS::RootedObject proto(info->context, info->jsc_proto);
            JS_DefineFunction(info->context, proto, name.c_str(), callback, static_cast<unsigned
                 int>(sizeof ... (Args)), JSPROP_PERMANENT | JSPROP_ENUMERATE | JSFUN_STUB_GSOPS);
        }

    };
};


}

#endif