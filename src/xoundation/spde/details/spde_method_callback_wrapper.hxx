//
// Created by secondwtq 15-8-3.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_SPDE_METHOD_CALLBACK_WRAPPER_HXX
#define MOZJS_SPDE_METHOD_CALLBACK_WRAPPER_HXX

#include "../../thirdpt/js_engine.hxx"

#include "../spde_heroes.hxx"
#include "../spde_caster.hxx"

#include <type_traits>

namespace xoundation {
namespace spd {

namespace details {

template<typename ProtoT, ProtoT func, bool is_void>
struct method_callback_wrapper;

template<typename T, typename ReturnT, typename ... Args, ReturnT (T::*func)(Args ...)>
struct method_callback_wrapper<ReturnT (T::*)(Args ...), func, false> {
    template<size_t ... N>
    inline static bool callback(T *self, JSContext *context, const JS::CallArgs& call_args,
                                std::tuple<typename caster<Args>::backT ...> args, indices<N ...>) {
        call_args.rval().set(caster<ReturnT>::tojs(context, (self->*func)(std::get<N>(args) ...)));
        return true;
    }
};

template<typename T, typename ReturnT, typename ... Args, ReturnT (T::*func)(Args ...) const>
struct method_callback_wrapper<ReturnT (T::*)(Args ...) const, func, false> {
    template<size_t ... N>
    inline static bool callback(const T *self, JSContext *context, const JS::CallArgs& call_args,
                                std::tuple<typename caster<Args>::backT ...> args, indices<N ...>) {
        call_args.rval().set(caster<ReturnT>::tojs(context, (self->*func)(std::get<N>(args) ...)));
        return true;
    }
};

template<typename T, typename ... Args, void (T::*func)(Args ...)>
struct method_callback_wrapper<void (T::*)(Args ...), func, true> {
    template<size_t ... N>
    inline static bool callback(T *self, JSContext *context, const JS::CallArgs& call_args,
                                std::tuple<typename caster<Args>::backT ...> args, indices<N ...>) {
        (self->*func)(std::get<N>(args) ...);
        call_args.rval().setUndefined();
        return true;
    }
};

template<typename T, typename ... Args, void (T::*func)(Args ...) const>
struct method_callback_wrapper<void (T::*)(Args ...) const, func, true> {
    template<size_t ... N>
    inline static bool callback(const T *self, JSContext *context, const JS::CallArgs& call_args,
                                std::tuple<typename caster<Args>::backT ...> args, indices<N ...>) {
        (self->*func)(std::get<N>(args) ...);
        call_args.rval().setUndefined();
        return true;
    }
};

template<typename ProtoT, ProtoT func, bool is_void>
struct raw_method_callback_wrapper;

template<typename T, typename ... Args, void func(T *, Args ...)>
struct raw_method_callback_wrapper<void (T *, Args ...), func, true> {
    template<size_t ... N>
    inline static bool callback(T *self, JSContext *context, const JS::CallArgs& call_args,
            std::tuple<typename caster<Args>::backT ...> args, indices<N ...>) {
        func(self, std::get<N>(args) ...);
        call_args.rval().setUndefined();
        return true;
    }
};

template<typename T, typename ... Args, void func(const T *, Args ...)>
struct raw_method_callback_wrapper<void (const T *, Args ...), func, true> {
    template<size_t ... N>
    inline static bool callback(const T *self, JSContext *context, const JS::CallArgs& call_args,
            std::tuple<typename caster<Args>::backT ...> args, indices<N ...>) {
        func(self, std::get<N>(args) ...);
        call_args.rval().setUndefined();
        return true;
    }
};

template<typename T, typename ReturnT, typename ... Args, ReturnT func(const T *, Args ...)>
struct raw_method_callback_wrapper<ReturnT (const T *, Args ...), func, false> {
    template<size_t ... N>
    inline static bool callback(const T *self, JSContext *context, const JS::CallArgs& call_args,
            std::tuple<typename caster<Args>::backT ...> args, indices<N ...>) {
        call_args.rval().set(caster<ReturnT>::tojs(context, func(self, std::get<N>(args) ...)));
        return true;
    }
};

template<typename T, typename ReturnT, typename ... Args, ReturnT func(T *, Args ...)>
struct raw_method_callback_wrapper<ReturnT (T *, Args ...), func, false> {
    template<size_t ... N>
    inline static bool callback(T *self, JSContext *context, const JS::CallArgs& call_args,
            std::tuple<typename caster<Args>::backT ...> args, indices<N ...>) {
        call_args.rval().set(caster<ReturnT>::tojs(context, func(self, std::get<N>(args) ...)));
        return true;
    }
};

}
}
}

#endif // MOZJS_SPDE_METHOD_CALLBACK_WRAPPER_HXX
