//
// Created by secondwtq 15-6-15.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#include <jsapi.h>

#include "xoundation/spde.hxx"
#include "xoundation/spde_helper.hxx"

#include "xoundation/native/speeder_native.hxx"
#include "xoundation/native/node_module.hxx"
#include "xoundation/native/node_native_fs.hxx"
#include "xoundation/native/node_buffer.hxx"
#include "xoundation/spde/spde_test_common.hxx"

using namespace xoundation;

namespace jssh {

void register_interfaces(SpdRuntime *srt, JS::HandleObject global, int argc, const char *argv[]) {
    native::register_interface_modules(*srt, global);
    node_native::register_interface_process(*srt, global, argc, argv);
    node_native::register_interface_os(*srt, global);
    node_native::register_interface_fs(*srt, global);
    native::register_interface_buffer(*srt, global);
    native::register_interface_speeder(*srt, global);
}

}

#include <iostream>

int main(int argc, const char *argv[]) {

    SpdRuntime *srt = nullptr;
    srt = new SpdRuntime;

    {
        JSAutoRequest at_req(*srt);

        JS::RootedObject global(*srt);
        global = JS_NewGlobalObject(*srt, &xoundation::cls_global, nullptr, JS::DontFireOnNewGlobalHook);
        if (!global) return 1;

        JSAutoCompartment at_comp(*srt, global);
        if (!JS_InitStandardClasses(*srt, global)) return 1;

        jssh::register_interfaces(srt, global, argc, argv);

        // TODO: a search path system
        std::string execpath = xoundation::node_native::get_execpath(argv);
        execpath = execpath.substr(0, execpath.rfind('/'));
        std::string source_pre = xoundation::readfile(execpath + "/lib/node_module.js");
        JS::RootedValue ret_pre(*srt);
        JS_EvaluateScript(*srt, global, source_pre.c_str(), static_cast<unsigned int>(source_pre
                                            .length()), "node_module", 0, &ret_pre);
    }

    delete srt;
    JS_ShutDown();
    return 0;
}
