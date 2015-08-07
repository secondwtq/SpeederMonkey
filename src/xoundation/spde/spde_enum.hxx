//
// Created by secondwtq 15-8-7.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_SPDE_ENUM_HXX
#define MOZJS_SPDE_ENUM_HXX

#include "../thirdpt/js_engine.hxx"

#include "details/spde_persistent_wrap.hxx"

#include <string>
#include <cassert>

namespace xoundation {
namespace spd {

template <typename EnumT>
class enumeration {
public:

    enumeration<EnumT> define(JSContext *c, JS::HandleObject global, const std::string& name) {
        assert(!internal().inited());
        this->context = c;
        JS::RootedObject enumobj(c);
        enumobj = JS_DefineObject(c, global, name.c_str(), nullptr, nullptr,
                                  JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY);
        internal().init_with_jsobj(c, enumobj);
        return *this;
    }

    template <EnumT element>
    enumeration<EnumT> enumerator(const std::string& name) {
        JS::RootedObject obj(this->context, *(internal().get()));

        JS::RootedValue valnum(this->context);
        valnum.setInt32(static_cast<int>(element));
        JS::RootedString valstr(this->context, JS_NewStringCopyN(this->context, name.c_str(), name.length()));

        JS_SetProperty(this->context, obj, name.c_str(), valnum);
        JS_SetElement(this->context, obj, static_cast<int>(element), valstr);
        return *this;
    }

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
