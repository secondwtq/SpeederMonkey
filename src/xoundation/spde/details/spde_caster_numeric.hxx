//
// Created by secondwtq 15-8-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_SPDE_CASTER_NUMERIC_HXX_HXX
#define MOZJS_SPDE_CASTER_NUMERIC_HXX_HXX

#include "../../thirdpt/js_engine.hxx"

namespace xoundation {
namespace spd {

template <typename T>
struct caster;

template<>
struct caster<bool> {

    using actualT = bool;
    using backT = bool;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return BOOLEAN_TO_JSVAL(src); }

    inline static backT back(JSContext *c, JS::HandleValue src) {
        return src.toBoolean(); }

};

template<>
struct caster<int> {

    using actualT = int;
    using backT = int;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return INT_TO_JSVAL(src); }

    inline static backT back(JSContext *, JS::HandleValue src) {
        return src.toInt32(); }

};

template<>
struct caster<long> {

    using actualT = long;
    using backT = long;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return INT_TO_JSVAL(src); }

    inline static backT back(JSContext *, JS::HandleValue src) {
        return src.toInt32();
    } // does toInt32() proper here?

};

template<>
struct caster<size_t> {

    using actualT = size_t;
    using backT = size_t;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return UINT_TO_JSVAL(static_cast<unsigned int>(src)); }

    inline static backT back(JSContext *, JS::HandleValue src) {
        return src.toInt32(); }

};

}
}

#endif // MOZJS_SPDE_CASTER_NUMERIC_HXX_HXX
