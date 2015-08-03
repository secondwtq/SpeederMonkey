//
// Created by secondwtq 15-8-3.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_SPDE_PROPERTY_ACCESSOR_HXX
#define MOZJS_SPDE_PROPERTY_ACCESSOR_HXX

#include <jsapi.h>

#include "../spde_vivalavida.hxx"
#include "../spde_caster.hxx"

namespace xoundation {
namespace spd {

namespace details {

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

template <typename T, typename PropT>
struct property_accessor_custom_base {

    template<JS::Value (*func)(JSContext *, unsigned int, JS::Value *, JS::CallArgs&, T *, bool *)>
    inline static bool getter(JSContext *context, unsigned int argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        T *raw = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)))->get();
        bool succeeded = true;
        args.rval().set((*func)(context, argc, vp, args, raw, &succeeded));
        return succeeded;
    }

    template<void (*func)(JSContext *, unsigned int, JS::Value *, JS::CallArgs&, T *, JS::HandleValue, bool *)>
    inline static bool setter(JSContext *context, unsigned int argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        T *raw = reinterpret_cast<lifetime<T> *>(JS_GetPrivate(JS_THIS_OBJECT(context, vp)))->get();
        bool succeeded = true;
        (*func)(context, argc, vp, args, raw, args[0], &succeeded);
        return succeeded;
    }
};

template <typename T, typename PropT>
struct property_accessor_general {

    template<PropT (T::*func)()>
    inline static JS::Value _getter(JSContext *ctx, unsigned int, JS::Value *, JS::CallArgs&, T *self, bool *) {
        return caster<PropT>::tojs(ctx, (self->*func)()); }

    template<void (T::*func)(PropT)>
    inline static void _setter(JSContext *ctx, unsigned int, JS::Value *,
                               JS::CallArgs&, T *self, JS::HandleValue value, bool *) {
        return (self->*func)(caster<PropT>::back(ctx, value)); }

    template<PropT (T::*func)()>
    inline static bool getter(JSContext *context, unsigned int argc, JS::Value *vp) {
        return property_accessor_custom_base<T, PropT>::template getter<_getter<func>>(context, argc, vp); }

    template<void (T::*func)(PropT)>
    inline static bool setter(JSContext *context, unsigned int argc, JS::Value *vp) {
        return property_accessor_custom_base<T, PropT>::template setter<_setter<func>>(context, argc, vp); }
};

}

}
}

#endif // MOZJS_SPDE_PROPERTY_ACCESSOR_HXX
