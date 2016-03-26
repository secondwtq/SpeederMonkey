#ifndef HEADER_SPEEDER_MONKEY_CLS_INFO
#define HEADER_SPEEDER_MONKEY_CLS_INFO

#include "../thirdpt/js_engine.hxx"

#include "spde_espwrap.hxx"

#include <assert.h>

namespace xoundation {
namespace spd {

namespace details {

static JSPropertySpec default_properties[] = { JS_PS_END };
static JSFunctionSpec default_funcs[] = { JS_FS_END };

static JSClass default_class_def = {
        "$speeder_default_class",
        JSCLASS_HAS_PRIVATE,
        JS_PropertyStub, spd::espwrap::StubDelProp,
        JS_PropertyStub, JS_StrictPropertyStub,
        spd::espwrap::StubEnum, spd::espwrap::StubResolve, spd::espwrap::StubConv,
        nullptr, nullptr, nullptr, nullptr, nullptr,
};

class class_info_base {
public:
    virtual ~class_info_base() { }
};

}

template<typename T>
class class_info : public details::class_info_base {

    public:

    struct inst_wrapper {
        private:
        static class_info<T> *_ret;

        public:

        inline static class_info<T> *get() { return _ret; }
        inline static class_info<T> *set(class_info<T> *src) {
            return (_ret = src); }

    };

    inline static class_info<T> *instance() { return inst_wrapper::get(); }

    class_info(JSContext *ct, const char *name, JSClass *classdef = &details::default_class_def,
        JSClass *proto_classdef = &details::default_class_def) : class_info(ct, name, nullptr, classdef, proto_classdef) { }

    class_info(JSContext *ct, const char *name, const char *protoname,
               JSClass *classdef, JSClass *proto_classdef) :
            context(ct), jsc_def(classdef_cloner(classdef).get()),
            jsc_def_proto(classdef_cloner(proto_classdef).get()) {

        char *new_name = (char *) malloc((strlen(name) + 1) * sizeof(char));
        strcpy(new_name, name);
        new_name[strlen(new_name)] = '\0';

        jsc_def->name = new_name;
        jsc_def_proto->name = new_name; // new_protoname;
    }

    // TODO: destruction of class_info
    ~class_info() { }

    JSContext *context = nullptr;
    // TODO: replace with JS::PersistentRooted
    JSClass *jsc_def = nullptr;
    JSClass *jsc_def_proto = nullptr;
    JSObject *jsc_proto = nullptr;

private:

    struct classdef_cloner {

        classdef_cloner(JSClass *src) :
                m_ref(reinterpret_cast<JSClass *>(malloc(sizeof(JSClass )))) {
            assert(src);
            assert(this->m_ref);

            memset(m_ref, 0, sizeof(JSClass));
            memcpy(m_ref, src, sizeof(JSClass));
        }

        JSClass *get() {
            return this->m_ref; }

    private:

        JSClass *m_ref = nullptr;
    };

};

template<typename T>
class_info<T> *class_info<T>::inst_wrapper::_ret = nullptr;

}
}

#endif