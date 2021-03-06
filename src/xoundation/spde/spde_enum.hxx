//
// Created by secondwtq 15-8-7.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_SPDE_ENUM_HXX
#define MOZJS_SPDE_ENUM_HXX

#include "../thirdpt/js_engine.hxx"

#include "spde_espwrap.hxx"
#include "details/spde_persistent_wrap.hxx"

#include <string>
#include <cassert>

// TODO: you know this module is relatively independent
namespace xoundation {
namespace spd {

template <typename EnumT>
class enumeration {
public:

    inline enumeration<EnumT> define(JSContext *c, JS::HandleObject global, const std::string& name) {
        assert(!internal().inited());
        this->context = c;
        JS::RootedObject enumobj(c);
        enumobj = espwrap::JSDefineObject(c, global, name.c_str(), nullptr, nullptr,
                                  JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY);
        internal().init_with_jsobj(c, enumobj);
        return *this;
    }

    inline enumeration<EnumT> enumerator(EnumT element, const std::string& name) {
        JS::RootedObject obj(this->context, *(internal().get()));

        JS::RootedValue valnum(this->context);
        valnum.setInt32(static_cast<int>(element));
        JS::RootedString valstr(this->context, JS_NewStringCopyN(this->context, name.c_str(), name.length()));

        JS_SetProperty(this->context, obj, name.c_str(), valnum);
        JS_SetElement(this->context, obj, static_cast<int>(element), valstr);
        return *this;
    }

    template <EnumT element>
    inline enumeration<EnumT> enumerator(const std::string& name) {
        return this->enumerator(element, name); }

private:

    JSContext *context = nullptr;

    details::persistent_rooted_wrap<JSObject *>& internal() {
        static details::persistent_rooted_wrap<JSObject *> impl;
        return impl;
    }
};

}
}

#endif // MOZJS_SPDE_ENUM_HXX
