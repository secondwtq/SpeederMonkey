//
// Created by secondwtq 15-5-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#include <memory>
#include <jsapi.h>

#include "xoundation/spde.hpp"

#include "xoundation/spde/spde_test_common.h"
#include "xoundation/native/node_native_fs.h"
#include "xoundation/native/node_module.h"

#include "xoundation/spde_helper.hxx"

using namespace xoundation;

SpdRuntime *srt = nullptr;

class vx_test {
    public:

    vx_test(int t) : test(t) {
        printf("VXX: vx_test constructing ... %d %lx\n", t, (unsigned long) this); }

    ~vx_test() { printf("VXX: vx_test descructing ... %d\n", this->test); }

    vx_test(const vx_test& o) {
        this->test_readonly = o.test_readonly;
        this->objref = o.objref;
        this->test = o.test;
        printf("vx_test copying ...\n");
    }

    int test_readonly = 2;

    vx_test *objref = nullptr;

    vx_test *get_objref() {
        printf("vx_test: get_objref called.\n");
        return this->objref;
    }

    void set_objref(vx_test *objref) {
        printf("vx_test: set_objref called.\n");
        this->objref = objref;
    }

    static vx_test *get_objref_static(vx_test *self) {
        printf("vx_test: get_objref_static called.\n");
        return self->objref;
    }

    int test = 3;

    int test_func(int a) {
//        printf("VXX: vx_test::test_func: %d %d\n", this->test, a);
        return a; }

    int test_func_objptr(int a, vx_test *o) {
        printf("VXX: test_func_objptr: %d ptr: %lx\n", a, o);
        return o->test; }

    static int test_static_func(int a) {
        printf("VXX: vx_test::test_static_func.\n");
        return a; }

    static std::shared_ptr<vx_test> sharedGlobal;

    static void setShared(std::shared_ptr<vx_test> src) {
        sharedGlobal = src; }

    static std::shared_ptr<vx_test> createShared(int t) {
        return std::make_shared<vx_test>(t); }
};

std::shared_ptr<vx_test> vx_test::sharedGlobal;

void test_funbind_void() {
    printf("VXX: test_funbind_void\n"); }

int test_funbind_objptr(int a, vx_test *obj) {
    printf("VXX: test_funbind_objptr: %d %d\n", a, obj->test);
    return 23333; }

class parent {
    public:

        parent() : data(srt->context()) { }

        int a = 0;

        virtual ~parent() { printf("parent destructing ...\n"); }

        std::string func_nonv() {
            return "I'm the parent and I'm not a virtual function.";
        }

        std::string func_parent() {
            return "I'm the parent."; }

        virtual std::string func() {
            std::string ret = "I'm the parent, I'm virtual.";
            ret += std::to_string(this->a);
            return ret; }

        parent *itsthis() { return this; }

    static int st;

    JS::PersistentRootedValue data;

    static int getst() { return st; }
};

class child : public parent {
    public:

        int b = 1;

        ~child() { printf("child destructing ...\n"); }

        std::string func_nonv() {
            return "I'm the child and I'm not a virtual function.";
        }

        std::string func_child() {
            return "I'm the child."; }

        virtual std::string func() {
            std::string ret = "I'm the child, I'm virtual.";
            ret += std::to_string(this->a);
            return ret; }
};

void call_data(JS::HandleValue p) {
    parent *raw = reinterpret_cast<spd::lifetime<parent> *>(JS_GetPrivate(p.toObjectOrNull()))
            ->get();
    JS::RootedValue ret_pre(*srt);
    JS::RootedObject self(*srt, p.toObjectOrNull());
    JS::RootedObject raw_data(*srt, raw->data.get().toObjectOrNull());
    JS::RootedValue t(*srt);
    JS_GetProperty(srt->context(), raw_data, "a", &t);
    printf("Data a: %d\n", t.toInt32());
    JS_CallFunctionValue(srt->context(), self, raw->data, JS::HandleValueArray::empty(), &ret_pre);
}

void collectgarbage() {
    JS_GC(srt->runtime()); }

int parent::st = 2;

class TestClassValueWrap {
public:

    TestClassValueWrap() : data(*srt) { }

    ~TestClassValueWrap() {
        printf("Destructing TestClassValueWrap ...\n"); }

    JS::PersistentRootedValue data;

    void print_data(spd::context_reference context) const {
        char *t = JS_EncodeString(context, JS::ToString(context, this->data));
        printf("%s\n", t);
        JS_free(context, t);
    }

    void print_property(const std::string& name, spd::context_reference context) const {
        JS::RootedObject inner_data(context);
        JSObject *obj = data.get().toObjectOrNull();
        if (obj) {
            inner_data.set(obj);
            JS::RootedValue val(context);
            JS_GetProperty(context, inner_data, name.c_str(), &val);
            char *t = JS_EncodeString(context, JS::ToString(context, val));
            printf("%s\n", t);
            JS_free(context, t);
        }
    }

    static std::shared_ptr<TestClassValueWrap> createShared() {
        return std::make_shared<TestClassValueWrap>(); }

};

bool proxy_resolve(JSContext *context, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleObject objp) {
    std::string name = spd::tostring_jsid(context, id);
    printf("Stub: resolving %s \n", name.c_str());
    if (TestClassValueWrap *self = spd::get_wrapper_object<TestClassValueWrap>(obj)) {
        JS::RootedObject proto(context);
        JS::RootedValue val(context);
        JS_GetPrototype(context, obj, &proto);
        if (!JS_LookupPropertyById(context, proto, id, &val) || val.isUndefined()) {
            JSObject *objdata = self->data.get().toObjectOrNull();
            if (objdata) {
                printf("Stub: setting to data %s \n", name.c_str());
                objp.set(objdata);
                return true;
            }
        }
    }
    objp.set(obj);
    return true;
}

bool proxy_add_property(JSContext *context, JS::HandleObject obj, JS::HandleId id,
                        JS::MutableHandleValue vp) {
    char *name = JS_EncodeString(context, JSID_TO_STRING(id));
    printf("Stub: adding %s \n", name);
    JS_free(context, name);

    void *ltdata = JS_GetPrivate(obj);
    if (ltdata) {
        TestClassValueWrap *self = reinterpret_cast<spd::lifetime<TestClassValueWrap> *>(ltdata)->get();
        JSObject *objdatap = self->data.get().toObjectOrNull();
        if (objdatap) {
            printf("Stub: adding %s to data\n", name);
            JS::RootedObject objdata(context, objdatap);
            JS_SetPropertyById(context, objdata, id, vp);
            return true;
        }
    }
    return true;
}

static JSClass proxyClassDef = {
        "proxyClassValueWrap",
        JSCLASS_HAS_PRIVATE | JSCLASS_NEW_RESOLVE,
        proxy_add_property, JS_DeletePropertyStub,
        JS_PropertyStub, JS_StrictPropertyStub,
        JS_EnumerateStub, (JSResolveOp) proxy_resolve, JS_ConvertStub,
        nullptr, nullptr, nullptr, nullptr, nullptr,
};

int main(int argc, const char *argv[]) {

    srt = new SpdRuntime;
    JS_SetGCCallback(srt->runtime(), spd_gc_callback, NULL);

    {
        JSAutoRequest at_req(*srt);

        JS::RootedObject global(*srt);
        global = JS_NewGlobalObject(*srt, &xoundation::cls_global, nullptr, JS::DontFireOnNewGlobalHook);
        if (!global) return 1;

        JSAutoCompartment at_comp(*srt, global);
        if (!JS_InitStandardClasses(*srt, global)) return 1;
        JS_DefineFunction(*srt, global, "print", xoundation::js_print, 5, attrs_func_default);

        spd::class_info<vx_test>::inst_wrapper::set(new spd::class_info<vx_test>(*srt));
        klass<vx_test>().define<int>("vx_test", global)
                    .property<int, &vx_test::test>("test")
                    .property<vx_test *, &vx_test::objref>("objref")
                    .accessor<vx_test *, &vx_test::get_objref, &vx_test::set_objref>("objref_acc")
                    .method<decltype(&vx_test::test_func), &vx_test::test_func>("test_func")
                    .method<decltype(&vx_test::test_func_objptr), &vx_test::test_func_objptr>("test_func_objptr")
                    .static_func<decltype(vx_test::createShared), vx_test::createShared>("createShared")
                    .static_func<decltype(vx_test::setShared), vx_test::setShared>("setShared");

        spd::class_info<parent>::inst_wrapper::set(new spd::class_info<parent>(*srt));
        klass<parent>().define<>("parent", global)
                    .property<int, &parent::a>("a")
                    .property<JS::PersistentRootedValue, &parent::data>("data")
                    .method<decltype(&parent::func), &parent::func>("func")
                    .method<decltype(&parent::func_nonv), &parent::func_nonv>("func_nonv")
                    .method<decltype(&parent::func_parent), &parent::func_parent>("func_parent")
                    .method<decltype(&parent::itsthis), &parent::itsthis>("itsthis")
                    .static_prop<int, &parent::st>("st")
                    .static_func<decltype(parent::getst), parent::getst>("getst");

        spd::class_info<child>::inst_wrapper::set(new spd::class_info<child>(*srt));
        klass<child>().inherits<parent>("child", global)
                    .method<decltype(&child::func), &child::func>("func")
                    .method<decltype(&child::func_child), &child::func_child>("func_child")
                    .property<int, &child::b>("b");

        spd::class_info<TestClassValueWrap>::inst_wrapper::set(new spd::class_info<TestClassValueWrap>(*srt));
        klass<TestClassValueWrap>().define<>("TestClassValueWrap", global, false, &proxyClassDef)
                    .property<JS::PersistentRootedValue, &TestClassValueWrap::data>("data")
                    .method<decltype(&TestClassValueWrap::print_data), &TestClassValueWrap::print_data>("print_data")
                    .method<decltype(&TestClassValueWrap::print_property), &TestClassValueWrap::print_property>
                        ("print_property");

        JS_DefineFunction(*srt, global, "test_funbind_objptr",
                          spd::function_callback_wrapper<int (int, vx_test *), test_funbind_objptr>::callback,
                          2, attrs_func_default);
        JS_DefineFunction(*srt, global, "test_funbind_void",
                          spd::function_callback_wrapper<void (), test_funbind_void>::callback, 0,
                                                                              attrs_func_default);

        JS_DefineFunction(*srt, global, "readfile", spd::function_callback_wrapper<decltype
                                    (readfile), readfile>::callback, 1, attrs_func_default);

        JS_DefineFunction(*srt, global, "call_data", spd::function_callback_wrapper<decltype
            (call_data), call_data>::callback, 1, attrs_func_default);

        JS_DefineFunction(*srt, global, "collectgarbage", spd::function_callback_wrapper<decltype
            (collectgarbage), collectgarbage>::callback, 1, attrs_func_default);

        native::register_interface_modules(*srt, global);
        node_native::register_interface_process(*srt, global, argc, argv);
        node_native::register_interface_os(*srt, global);
        node_native::register_interface_fs(*srt, global);

        printf("loading ...\n");
        std::string source_pre = readfile("./lib/node_module.js");
        JS::RootedValue ret_pre(*srt);
        JS_EvaluateScript(*srt, global, source_pre.c_str(), static_cast<unsigned  int>(source_pre
                                                        .length()), "node_module", 0, &ret_pre);

    }

    delete srt;
    JS_ShutDown();
    return 0;
}
