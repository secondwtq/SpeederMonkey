//
// Created by secondwtq 15-8-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_SPDE_CASTER_STRING_HXX
#define MOZJS_SPDE_CASTER_STRING_HXX

#include "../../thirdpt/js_engine.hxx"

#include <string>

namespace xoundation {
namespace spd {

template <typename T>
struct caster;

template<>
struct caster<const char *> {

    using actualT = const char *;
    using backT = const char *;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        // should this be undefined?
        if (src == nullptr) {
            return JS::UndefinedValue(); }
        return STRING_TO_JSVAL(JS_NewStringCopyZ(c, src));
    }

};

template<>
struct caster<std::string> {

    using actualT = const std::string&;
    using backT = std::string;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return STRING_TO_JSVAL(JS_NewStringCopyZ(c, src.c_str())); }

    inline static backT back(JSContext *c, JS::HandleValue src) {
        return JS_EncodeString(c, src.toString()); }

};

template<>
struct caster<std::string&> {

    using actualT = const std::string&;
    using backT = std::string;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return STRING_TO_JSVAL(JS_NewStringCopyZ(c, src.c_str())); }

    inline static backT back(JSContext *c, JS::HandleValue src) {
        return JS_EncodeString(c, src.toString()); }

};

template<>
struct caster<const std::string&> {

    using actualT = const std::string&;
    using backT = std::string;
    using jsT = JS::Value;

    inline static jsT tojs(JSContext *c, actualT src) {
        return STRING_TO_JSVAL(JS_NewStringCopyZ(c, src.c_str())); }

    inline static backT back(JSContext *c, JS::HandleValue src) {
        const char *t = JS_EncodeString(c, src.toString());
        std::string ret(t);
        JS_free(c, const_cast<char *>(t));
        return ret;
    }

};

}
}

#endif // MOZJS_SPDE_CASTER_STRING_HXX
