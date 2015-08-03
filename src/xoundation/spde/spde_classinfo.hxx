#ifndef HEADER_SPEEDER_MONKEY_CLS_INFO
#define HEADER_SPEEDER_MONKEY_CLS_INFO

#include "../thirdpt/js_engine.hxx"

namespace xoundation {
namespace spd {

namespace details {

static JSPropertySpec default_properties[] = { JS_PS_END };
static JSFunctionSpec default_funcs[] = { JS_FS_END };

static JSClass default_class_def = {
        "$speeder_default_class",
        JSCLASS_HAS_PRIVATE,
        JS_PropertyStub, JS_DeletePropertyStub,
        JS_PropertyStub, JS_StrictPropertyStub,
        JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,
        nullptr, nullptr, nullptr, nullptr, nullptr,
};

}

template<typename T>
class class_info {

    public:

    struct inst_wrapper {
        private:
        static class_info<T> *_ret;

        public:

        inline static class_info<T> *get() { return _ret; }

        inline static class_info<T> *set(class_info<T> *src) {
            return (_ret = src);
        }

    };

    inline static class_info<T> *instance() { return inst_wrapper::get(); }

    class_info(JSContext *ct) : context(ct) { }

    JSContext *context = nullptr;
    // TODO: replace with JS::PersistentRooted
    JSClass *jsc_def = nullptr;
    JSObject *jsc_proto = nullptr;

};

template<typename T>
class_info<T> *class_info<T>::inst_wrapper::_ret = nullptr;

}
}

#endif