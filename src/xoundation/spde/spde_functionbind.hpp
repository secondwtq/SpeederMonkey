//
// Created by secondwtq 15-5-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef HEADER_SPEEDER_MONKEY_BIND_FUNCTION
#define HEADER_SPEEDER_MONKEY_BIND_FUNCTION

#include <jsapi.h>

#include <cstddef>
#include <tuple>

#include "spde_heroes.hpp"
#include "spde_caster.hpp"

namespace xoundation {
namespace spd {

namespace details {

template<typename ... T, size_t ... N>
inline std::tuple<typename caster<T>::backT ...> construct_args(JSContext *context, const
        JS::CallArgs& args, indices<N ...>) {
    return std::forward_as_tuple<typename caster<T>::backT ...>(caster<T>::back(context, args[N]) ...); }

template<typename ... T>
std::tuple<typename caster<T>::backT ...> construct_args(JSContext *context, const JS::CallArgs&
args) {
    constexpr int count = sizeof ... (T);
    return construct_args<T ...>(context, args, typename indices_builder<count>::type());
}

template<typename ProtoT, ProtoT& func>
struct callback_wrapper;

template<typename ReturnT, typename ... Args, ReturnT (& func)(Args ...)>
struct callback_wrapper<ReturnT(Args ...), func> {
    template<size_t ... N>
    inline static bool callback(JSContext *context, const JS::CallArgs& call_args,
                                std::tuple<typename caster<Args>::backT ...> args, indices<N ...>) {
        call_args.rval().set(caster<ReturnT>::tojs(context, func(std::get<N>(args) ...)));
        return true;
    }
};

template<typename ... Args, void (& func)(Args ...)>
struct callback_wrapper<void(Args ...), func> {
    template<size_t ... N>
    inline static bool callback(JSContext *context, const JS::CallArgs& call_args,
                                std::tuple<typename caster<Args>::backT ...> args, indices<N ...>) {
        func(std::get<N>(args) ...);
        call_args.rval().setUndefined();
        return true;
    }
};

}

template<typename ProtoT, ProtoT& func>
struct function_callback_wrapper;

template<typename ReturnT, typename ... Args, ReturnT (& func)(Args ...)>
struct function_callback_wrapper<ReturnT(Args ...), func> {

    template<typename U = ReturnT>
    inline static bool callback(JSContext *context, unsigned int argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        auto args_tuple = details::construct_args<Args ...>(context, args);
        return details::callback_wrapper<ReturnT(Args ...), func>::callback(
                context, args, args_tuple, typename indices_builder<sizeof ... (Args)>::type());
    }

    inline static void register_func(JSContext *context, JS::HandleObject global, const
            std::string& name) {
        JS_DefineFunction(context, global, name.c_str(), callback, static_cast<unsigned int>
            (sizeof ... (Args)), JSPROP_ENUMERATE | JSPROP_PERMANENT | JSFUN_STUB_GSOPS);
    }

};

}
}

#endif
