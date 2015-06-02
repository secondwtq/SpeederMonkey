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

namespace xoundation {
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

template <typename T, typename PropT>
struct property_accessor {

    template<PropT T::*AttrT>
    inline static bool default_getter(JSContext *context, unsigned int argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        T *raw = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)))->get();
        args.rval().set(caster<PropT>::tojs(context, raw->*AttrT));
        return true;
    }

    template<PropT T::*AttrT>
    inline static bool default_setter(JSContext *context, unsigned int argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        T *raw = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)))->get();
        raw->*AttrT = caster<PropT>::back(context, args[0]);

        return true;
    }
};

template <typename T>
struct property_accessor <T, JS::PersistentRootedValue> {

    template<JS::PersistentRootedValue T::*AttrT>
    inline static bool default_getter(JSContext *context, unsigned int argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        T *raw = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)))->get();
        args.rval().set(raw->*AttrT);
        return true;
    }

    template<JS::PersistentRootedValue T::*AttrT>
    inline static bool default_setter(JSContext *context, unsigned int argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        T *raw = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)))->get();
        raw->*AttrT = args[0];
        return true;
    }

};

}

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

    inline static void dtor_callback(JSFreeOp *op, JSObject *obj) {
        lifetime<T> *raw = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(obj));
        delete raw; }

    template<typename ... Args>
    struct ctor_wrapper {

        template<size_t ... N>
        inline static T *callback_internal(std::tuple<Args ...> args, indices<N ...>) {
            return new T(std::get<N>(args) ...); }

        // for placement new
        template<size_t ... N>
        inline static T *callback_internal(std::tuple<Args ...> args, indices<N ...>, void *ptr) {
            return new (ptr) T(std::get<N>(args) ...); }

        inline static bool callback(JSContext *context, unsigned int argc, JS::Value *vp) {
            JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
            auto args_tuple = details::construct_args<typename caster<Args>::backT ...>(context,
                                                                                        args);
            lifetime<T> *t = new lifetime_js<T>(LIFETIME_PLACEMENT_CONSTRUCT);
            callback_internal(args_tuple, typename indices_builder
                    <sizeof ... (Args)>::type(), t->get());
            JS::RootedObject proto(context, info_t::instance()->jsc_proto);
            JSObject *jsobj = JS_NewObject(context, info_t::instance()->jsc_def, proto, JS::NullPtr());
            JS_SetPrivate(jsobj, reinterpret_cast<void *>(t));
            args.rval().set(OBJECT_TO_JSVAL(jsobj));
            return true;
        }

        inline static void define(const std::string& name, JS::HandleObject global,
                                  JS::HandleObject parent_proto = JS::NullPtr()) {
            info_t *info = info_t::instance();

            JSClass *jsclass_def = reinterpret_cast<JSClass *>(malloc(sizeof(JSClass)));
            info->jsc_def = jsclass_def;
            memset(jsclass_def, 0, sizeof(JSClass)); // you *must* init it or it would
            // lead to undefined behaviour
            memcpy(jsclass_def, &details::default_class_def, sizeof(JSClass));
            info->jsc_def->finalize = dtor_callback;

            // TODO: dunno how the memory for the name string should be dealed with
            info->jsc_def->name = (char *) malloc((name.length() + 1) * sizeof(char));
            memcpy((char *) info->jsc_def->name, name.c_str(), name.length() * sizeof(char));
            ((char *) info->jsc_def->name)[name.length()] = '\0';

            info->jsc_proto = JS_InitClass(info->context, global, parent_proto, info->jsc_def,
                                           callback, 0, details::default_properties,
                                           details::default_funcs, nullptr, nullptr);
        }

        template<typename ParentT>
        inline static void define(const std::string& name, JS::HandleObject global) {
            JS::RootedObject parent_proto(info_t::instance()->context,
                                          class_info<ParentT>::instance()->jsc_proto);
            define(name, global, parent_proto);
        }

    };

    template<typename ProtoT, ProtoT func>
    struct method_callback_wrapper;

    template<typename ReturnT, typename ... Args, ReturnT (T::*func)(Args ...)>
    struct method_callback_wrapper<ReturnT (T::*)(Args ...), func> {

        template<typename U = ReturnT>
        inline static bool callback(JSContext *context, unsigned int argc, JS::Value *vp) {
            JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
            auto args_tuple = details::construct_args<typename caster<Args>::backT ...>(context,
                                                                                        args);
            T *raw = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)))->get();
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

    // convenient interface added 150529 EVE
    template <typename ... Args>
    inline class_helper<T> define(const std::string& name, JS::HandleObject global) {
        ctor_wrapper<Args ...>::define(name, global);
        return *this;
    }

    template <typename ParentT, typename ... Args>
    inline class_helper<T> inherits(const std::string& name, JS::HandleObject global) {
        ctor_wrapper<Args ...>::template define<ParentT>(name, global);
        return *this;
    }

    template<typename PropT, PropT T::*AttrT>
    inline class_helper<T> property(const std::string& name) {
        class_helper<T>::reg_property<PropT, AttrT>(name);
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
        JS_DefineProperty(info->context, proto, name.c_str(), JS::UndefinedHandleValue,
                          JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,
                          reinterpret_cast<JSPropertyOp>(default_getter_static<PropT, AttrT>),
                          reinterpret_cast<JSStrictPropertyOp>(default_setter_static<PropT, AttrT>));

        return *this;
    }

    template<typename ProtoT, ProtoT& func>
    inline class_helper<T> static_func(const std::string& name) {
        info_t *info = info_t::instance();
        JS::RootedObject proto(info->context, info->jsc_proto);
        function_callback_wrapper<ProtoT, func>::register_func(info->context, proto, name.c_str());

        return *this;
    }

};


}
}

#endif
