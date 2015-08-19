//
// Created by secondwtq 15-5-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#include <memory>
#include <jsapi.h>

#include "xoundation/spde.hxx"

#include "xoundation/spde/spde_test_common.hxx"
#include "xoundation/native/node_native_fs.hxx"
#include "xoundation/native/node_module.hxx"

#include "xoundation/spde_helper.hxx"
#include "xoundation/spde/spde_enum.hxx"

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
        printf("VXX: test_func_objptr: %d ptr: %p\n", a, o);
        return o->test; }

    static int test_static_func(int a) {
        printf("VXX: vx_test::test_static_func.\n");
        return a; }

    static std::shared_ptr<vx_test> sharedGlobal;

    static void setShared(std::shared_ptr<vx_test> src) {
        sharedGlobal = src; }

    static std::shared_ptr<vx_test> createShared(int t) {
        return std::make_shared<vx_test>(t); }

    static std::shared_ptr<vx_test> getShared() {
        return sharedGlobal; }
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

bool proxy_resolve(JSContext *context, JS::HandleObject obj,
                   JS::HandleId id, JS::MutableHandleObject objp) {
    std::string name = spd::tostring_jsid(context, id);
    printf("Stub: resolving %s\n", name.c_str());

    if (TestClassValueWrap *self = spd::get_wrapper_object<TestClassValueWrap>(obj)) {
        JS::RootedObject proto(context);
        JS::RootedValue val(context);
        if (!JS_GetPrototype(context, obj, &proto)) {
            return true; }
        if (!JS_LookupPropertyById(context, proto, id, &val) || val.isUndefined()) {
            JSObject *objdatap = self->data.get().toObjectOrNull();
            if (objdatap) {
                JS::RootedObject objdata(context, objdatap);
                if (!JS_LookupPropertyById(context, objdata, id, &val) || val.isUndefined()) {
                    printf("Lookup failed, adding %s\n", name.c_str());
                    JS::RootedValue v(context, JSVAL_TRUE);
                    JS_SetProperty(context, objdata, name.c_str(), v);
                }
                objp.set(objdata);
                return true;
            }
        }
    }
    objp.set(obj);
    return true;
}

bool proxy_add_property(JSContext *context, JS::HandleObject obj,
                        JS::HandleId id, JS::MutableHandleValue vp) {
    std::string name = spd::tostring_jsid(context, id);
    printf("Stub: adding %s \n", name.c_str());

    if (TestClassValueWrap *self = spd::get_wrapper_object<TestClassValueWrap>(obj)) {
        JSObject *objdatap = self->data.get().toObjectOrNull();
        if (objdatap) {
            JS::RootedObject objdata(context, objdatap);
            return JS_SetPropertyById(context, objdata, id, vp);
        }
    }
    return true;
}

bool proxy_set_property(JSContext *context, JS::HandleObject obj,
                        JS::HandleId id, bool strict, JS::MutableHandleValue vp) {
    std::string name = spd::tostring_jsid(context, id);
    printf("Stub: setting %s\n", name.c_str());

    if (TestClassValueWrap *self = spd::get_wrapper_object<TestClassValueWrap>(obj)) {
        JSObject *objdatap = self->data.get().toObjectOrNull();
        if (objdatap) {
            JS::RootedObject objdata(context, objdatap);
            return JS_SetPropertyById(context, objdata, id, vp);
        }
    }
    return true;
}

bool proxy_get_property(JSContext *context, JS::HandleObject obj,
                        JS::HandleId id, JS::MutableHandleValue vp) {
    std::string name = spd::tostring_jsid(context, id);
    printf("Stub: getting %s \n", name.c_str());

    if (TestClassValueWrap *self = spd::get_wrapper_object<TestClassValueWrap>(obj)) {
        JSObject *objdatap = self->data.get().toObjectOrNull();
        if (objdatap) {
            JS::RootedObject objdata(context, objdatap);
            return JS_GetPropertyById(context, objdata, id, vp);
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

class TestIntrusiveObjectForControl {
public:

    int internalData = 0;
    virtual ~TestIntrusiveObjectForControl() {
        printf("Destructing TestIntrusiveObjectForControl %p ...\n", this); }

};

class TestIntrusiveObject : public TestIntrusiveObjectForControl,
                            public spd::intrusive_object<TestIntrusiveObject> {
public:

    int getExternalData(spd::context_reference context) {
        JS::RootedObject intr(context, spd::get_intrusive_object(this));
        JS::RootedValue ret(context);
        JS_GetProperty(context, intr, "externalData", &ret);
        return ret.toInt32();
    }

    virtual ~TestIntrusiveObject() {
        printf("Destructing TestIntrusiveObject %p ...\n", this); }

};

template <typename T>
inline T *passAround(T *src) { return src; }

enum EnumTest {
    Foundation,
    Pressure,
    Reliable
};

inline EnumTest returnsEnum() {
    return EnumTest::Foundation; }

inline EnumTest passAroundEnum(EnumTest src) {
    return src; }

class CopyTestCopied {
public:
    CopyTestCopied() {
        printf("CopyTestCopied constructed. %p\n", this); }
    CopyTestCopied(const CopyTestCopied&) {
        printf("CopyTestCopied copied. %p\n", this); }
    CopyTestCopied(CopyTestCopied&&) {
        printf("CopyTestCopied moved. %p\n", this); }
    ~CopyTestCopied() {
        printf("CopyTestCopied destructed. %p\n", this); }
};

class CopyTest {
public:
    CopyTest(CopyTestCopied arg) {
        printf("CopyTest constructed. this: %p, CopyTestCopied: %p\n", this, &arg); }
    CopyTest(const CopyTest&) {
        printf("CopyTest copied. %p\n", this); }
    CopyTest(CopyTest&&) {
        printf("CopyTest moved. %p\n", this); }

    void testCopy(CopyTestCopied arg) {
        printf("testCopy called. this: %p, CopyTestCopied: %p\n", this, &arg); }

    ~CopyTest() {
        printf("CopyTest destructed. %p\n", this); }
};

int main(int argc, const char *argv[]) {

    srt = new SpdRuntime;
    srt->set_gc_callback(spd_gc_callback);

    {
        JSAutoRequest at_req(*srt);
        JS::RootedObject global(*srt, srt->create_global());
        JSAutoCompartment at_comp(*srt, global);

        JS_DefineFunction(*srt, global, "print", xoundation::js_print, 5, attrs_func_default);

        spd::class_info<vx_test>::inst_wrapper::set(new spd::class_info<vx_test>(*srt, "vx_test"));
        klass<vx_test>().define<int>(global)
                    .property<int, &vx_test::test>("test")
                    .property<vx_test *, &vx_test::objref>("objref")
                    .accessor<vx_test *, &vx_test::get_objref, &vx_test::set_objref>("objref_acc")
                    .method<decltype(&vx_test::test_func), &vx_test::test_func>("test_func")
                    .method<decltype(&vx_test::test_func_objptr), &vx_test::test_func_objptr>("test_func_objptr")
                    .static_prop<decltype(vx_test::sharedGlobal), &vx_test::sharedGlobal>("sharedGlobal")
                    .static_func<decltype(vx_test::createShared), vx_test::createShared>("createShared")
                    .static_func<decltype(vx_test::setShared), vx_test::setShared>("setShared")
                    .static_func<decltype(vx_test::getShared), vx_test::getShared>("getShared");

        spd::class_info<parent>::inst_wrapper::set(new spd::class_info<parent>(*srt, "parent"));
        klass<parent>().define<>(global)
                    .property<int, &parent::a>("a")
                    .property<JS::PersistentRootedValue, &parent::data>("data")
                    .method<decltype(&parent::func), &parent::func>("func")
                    .method<decltype(&parent::func_nonv), &parent::func_nonv>("func_nonv")
                    .method<decltype(&parent::func_parent), &parent::func_parent>("func_parent")
                    .method<decltype(&parent::itsthis), &parent::itsthis>("itsthis")
                    .static_prop<int, &parent::st>("st")
                    .static_func<decltype(parent::getst), parent::getst>("getst");

        spd::class_info<child>::inst_wrapper::set(new spd::class_info<child>(*srt, "child"));
        klass<child>().inherits<parent>(global, spd::argpack<>())
                    .method<decltype(&child::func), &child::func>("func")
                    .method<decltype(&child::func_child), &child::func_child>("func_child")
                    .property<int, &child::b>("b");

        spd::class_info<TestClassValueWrap>::inst_wrapper::set(new spd::class_info<TestClassValueWrap>(*srt,
                                           "TestClassValueWrap", &proxyClassDef, spd::get_default_classdef()));
        klass<TestClassValueWrap>().define<>(global)
                    .property<JS::PersistentRootedValue, &TestClassValueWrap::data>("data")
                    .method<decltype(&TestClassValueWrap::print_data), &TestClassValueWrap::print_data>("print_data")
                    .method<decltype(&TestClassValueWrap::print_property), &TestClassValueWrap::print_property>
                        ("print_property")
                    .static_func<decltype(TestClassValueWrap::createShared), TestClassValueWrap::createShared>("createShared");

        spd::class_info<TestIntrusiveObjectForControl>::inst_wrapper::set(
                new spd::class_info<TestIntrusiveObjectForControl>(*srt, "TestIntrusiveObjectForControl"));
        klass<TestIntrusiveObjectForControl>().define<>(global)
                .property<int, &TestIntrusiveObjectForControl::internalData>("internalData");

        spd::class_info<TestIntrusiveObject>::inst_wrapper::set(
                new spd::class_info<TestIntrusiveObject>(*srt, "TestIntrusiveObject"));
        klass<TestIntrusiveObject>().inherits<TestIntrusiveObjectForControl>(global, spd::argpack<>())
                .method<decltype(&TestIntrusiveObject::getExternalData), &TestIntrusiveObject::getExternalData>
                        ("getExternalData");

        JS_DefineFunction(*srt, global, "passAroundTestIntrusiveControl",
            spd::function_callback_wrapper<decltype(passAround<TestIntrusiveObjectForControl>),
                    passAround<TestIntrusiveObjectForControl>>::callback, 1, attrs_func_default);
        JS_DefineFunction(*srt, global, "passAroundTestIntrusive",
                          spd::function_callback_wrapper<decltype(passAround<TestIntrusiveObject>),
                                  passAround<TestIntrusiveObject>>::callback, 1, attrs_func_default);

        spd::enumeration<EnumTest>().define(*srt, global, "EnumTest")
                .enumerator<EnumTest::Foundation>("Foundation")
                .enumerator<EnumTest::Pressure>("Pressure")
                .enumerator(EnumTest::Reliable, "Reliable");

        spd::class_info<CopyTest>::inst_wrapper::set(new spd::class_info<CopyTest>(*srt, "CopyTest"));
        klass<CopyTest>().define(global, spd::argpack<CopyTestCopied>())
                .attach("attachNew", spd::argpack<CopyTestCopied>())
                .reproto("reproto")
                .method<decltype(&CopyTest::testCopy), &CopyTest::testCopy>("testCopy");

        spd::class_info<CopyTestCopied>::inst_wrapper::set(new spd::class_info<CopyTestCopied>(*srt, "CopyTestCopied"));
        klass<CopyTestCopied>().define(global);

        JS_DefineFunction(*srt, global, "returnsEnum", spd::function_callback_wrapper<decltype(returnsEnum),
                returnsEnum>::callback, 0, attrs_func_default);
        JS_DefineFunction(*srt, global, "passAroundEnum", spd::function_callback_wrapper<decltype(passAroundEnum),
                passAroundEnum>::callback, 1, attrs_func_default);

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
