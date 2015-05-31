//
// Created by secondwtq 15-5-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

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

        parent()
//                data(new JS::RootedValue(srt->context())) { }
            {
                data = new JS::HandleValue(JS::UndefinedHandleValue);
            }
        int a = 0;

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

//    JS::RootedValue *data;
     JS::HandleValue *data;

    static int getst() { return st; }
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
            std::string ret = "I'm the child, I'm virtual.";
            ret += std::to_string(this->a);
            return ret; }
};

int parent::st = 2;

int main(int argc, const char *argv[]) {

    srt = new SpdRuntime;

    {
        JSAutoRequest at_req(*srt);

        JS::RootedObject global(*srt);
        global = JS_NewGlobalObject(*srt, &xoundation::cls_global, nullptr, JS::DontFireOnNewGlobalHook);
        if (!global) return 1;

        JSAutoCompartment at_comp(*srt, global);
        if (!JS_InitStandardClasses(*srt, global)) return 1;
        JS_DefineFunction(*srt, global, "print", print, 5, attrs_func_default);

        spd::class_info<vx_test>::inst_wrapper::set(new spd::class_info<vx_test>(*srt));
        spd::class_helper<vx_test>::ctor_wrapper<int>::define("vx_test", global);

        spd::class_helper<vx_test>::reg_property<vx_test *, &vx_test::objref>("objref");
        spd::class_helper<vx_test>::reg_property<int, &vx_test::test>("test");
        spd::class_helper<vx_test>::method_callback_wrapper<decltype(&vx_test::test_func),
                &vx_test::test_func>::register_as("test_func");
        spd::class_helper<vx_test>::method_callback_wrapper<decltype(&vx_test::test_func_objptr),
                &vx_test::test_func_objptr>::register_as("test_func_objptr");

        spd::class_info<parent>::inst_wrapper::set(new spd::class_info<parent>(*srt));
        klass<parent>().define<>("parent", global)
                    .property<int, &parent::a>("a")
                    .property<JS::HandleValue *, &parent::data>("data")
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

        JS_DefineFunction(*srt, global, "test_funbind_objptr",
                          spd::function_callback_wrapper<int (int, vx_test *), test_funbind_objptr>::callback,
                                                                              2, attrs_func_default);
        JS_DefineFunction(*srt, global, "test_funbind_void",
                          spd::function_callback_wrapper<void (), test_funbind_void>::callback, 0,
                                                                              attrs_func_default);

        JS_DefineFunction(*srt, global, "readfile", spd::function_callback_wrapper<decltype
                                    (readfile), readfile>::callback, 1, attrs_func_default);

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
