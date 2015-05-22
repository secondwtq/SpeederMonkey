//
// Created by secondwtq 15-5-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#include <jsapi.h>

#include "xoundation/spde.hpp"

#include "xoundation/spde/spde_test_common.h"
#include "xoundation/native/node_native_fs.h"
#include "xoundation/native/node_module.h"

class vx_test {
    public:

    vx_test(int t) : test(t) {
        printf("VXX: vx_test constructing ... %d %lx\n", t, (unsigned long) this); }

    ~vx_test() { printf("VXX: vx_test descructing ...\n"); }

    vx_test(const vx_test& o) {
        this->test_readonly = o.test_readonly;
        this->objref = o.objref;
        this->test = o.test;
        printf("vx_test copying ...\n");
    }

    int test_readonly = 2;

    vx_test *objref;

    int test = 3;

    int test_func(int a) {
//        printf("VXX: vx_test::test_func: %d %d\n", this->test, a);
        return a; }

    int test_func_objptr(int a, vx_test *o) {
        return o->test; }

    static int test_static_func(int a) {
        printf("VXX: vx_test::test_static_func.\n");
        return a; }
};

void test_funbind_void() {
    printf("VXX: test_funbind_void\n"); }

int test_funbind_objptr(int a, vx_test *obj) {
    printf("VXX: test_funbind_objptr: %d %d\n", a, obj->test);
    return 23333; }

class parent {
    public:

        int a = 0;

        std::string func_nonv() {
            return "I'm the parent and I'm not a virtual function.";
        }

        std::string func_parent() {
            return "I'm the parent."; }

        virtual std::string func() {
            return "I'm the parent, I'm virtual."; }
};

class child : public parent {
    public:

        int b = 1;

        std::string func_nonv() {
            return "I'm the child and I'm not a virtual function.";
        }

        std::string func_child() {
            return "I'm the child."; }

        virtual std::string func() {
            return "I'm the child, I'm virtual."; }
};

int main(int argc, const char *argv[]) {

    if (!JS_Init()) return 1;
    JSRuntime *js_rt = JS_NewRuntime(32L * 1024L * 1024L,
    JSUseHelperThreads::JS_USE_HELPER_THREADS);

    JS::RuntimeOptionsRef(js_rt).setBaseline(true)
                            .setIon(true).setAsmJS(true);

    JS_SetNativeStackQuota(js_rt, 128 * sizeof(size_t) * 1024); // WTF ...

    JSContext *context = JS_NewContext(js_rt, 8192);
    if (!context) return 1;
    JS_SetErrorReporter(context, report_exception);

    {
        JSAutoRequest at_req(context);

        JS::RootedObject global(context);
        global = JS_NewGlobalObject(context, &global_class, nullptr, JS::DontFireOnNewGlobalHook);
        if (!global) return 1;

        JSAutoCompartment at_comp(context, global);
        if (!JS_InitStandardClasses(context, global)) return 1;
        JS_DefineFunction(context, global, "print", print, 5, attrs_func_default);

        spd::class_info<vx_test>::inst_wrapper::set(new spd::class_info<vx_test>(context));
        spd::class_helper<vx_test>::ctor_wrapper<int>::define("vx_test", global);
        spd::class_helper<vx_test>::property<vx_test *, &vx_test::objref>("objref");
        spd::class_helper<vx_test>::property<int, &vx_test::test>("test");
        spd::class_helper<vx_test>::method_callback_wrapper<decltype(&vx_test::test_func),
                &vx_test::test_func>::register_as("test_func");
        spd::class_helper<vx_test>::method_callback_wrapper<decltype(&vx_test::test_func_objptr),
                &vx_test::test_func_objptr>::register_as("test_func_objptr");

        spd::class_info<parent>::inst_wrapper::set(new spd::class_info<parent>(context));
        spd::class_helper<parent>::ctor_wrapper<>::define("parent", global);
        spd::class_helper<parent>::property<int, &parent::a>("a");
        spd::class_helper<parent>::method_callback_wrapper<decltype(&parent::func),
                &parent::func>::register_as("func");
        spd::class_helper<parent>::method_callback_wrapper<decltype(&parent::func_nonv),
                &parent::func_nonv>::register_as("func_nonv");
        spd::class_helper<parent>::method_callback_wrapper<decltype(&parent::func_parent),
                &parent::func_parent>::register_as("func_parent");

        spd::class_info<child>::inst_wrapper::set(new spd::class_info<child>(context));
        spd::class_helper<child>::ctor_wrapper<>::define<parent>("child", global);
        spd::class_helper<child>::property<int, &child::b>("b");
        spd::class_helper<child>::method_callback_wrapper<decltype(&child::func),
                &child::func>::register_as("func");
        spd::class_helper<child>::method_callback_wrapper<decltype(&child::func_child),
                &child::func_child>::register_as("func_child");

        JS_DefineFunction(context, global, "test_funbind_objptr",
                          spd::function_callback_wrapper<int (int, vx_test *), test_funbind_objptr>::callback,
                                                                              2, attrs_func_default);
        JS_DefineFunction(context, global, "test_funbind_void",
                          spd::function_callback_wrapper<void (), test_funbind_void>::callback, 0,
                                                                              attrs_func_default);

        JS_DefineFunction(context, global, "readfile", spd::function_callback_wrapper<decltype
                                    (readfile), readfile>::callback, 1, attrs_func_default);

        xoundation::native::register_interface_modules(context, global);
        xoundation::node_native::register_interface_process(context, global, argc, argv);
        xoundation::node_native::register_interface_os(context, global);
        xoundation::node_native::register_interface_fs(context, global);

        printf("loading ...\n");
        std::string source_pre = readfile("./lib/node_module.js");
        JS::RootedValue ret_pre(context);
//        enableTracing(context);
        JS_EvaluateScript(context, global, source_pre.c_str(), static_cast<unsigned  int>(source_pre
                                                        .length()), "node_module", 0, &ret_pre);

    }

    JS_DestroyContext(context);
    JS_DestroyRuntime(js_rt);
    JS_ShutDown();
    return 0;
}
