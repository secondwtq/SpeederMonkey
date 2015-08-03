//
// Created by secondwtq 15-5-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_SPDE_TEST_COMMON_H
#define MOZJS_SPDE_TEST_COMMON_H

#include "../thirdpt/js_engine.hxx"

#include <string>
#include <cstdio>

namespace xoundation {

inline bool js_print(JSContext *context, unsigned int argc, JS::Value *vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    for (size_t i = 0; i < args.length(); i++) {
        const char *t = JS_EncodeString(context, JS::ToString(context, args[i]));
        printf("%s", t);
        JS_free(context, const_cast<char *>(t));
    }
    printf("\n");
    args.rval().setUndefined();

    return true;
}

inline std::string readfile(const std::string& filename) {
    // printf("::readfile - Reading %s ...\n", filename.c_str());
    FILE *fp = fopen(filename.c_str(), "r");

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);

    char *ret = new char[fsize + 1];
    fread(ret, static_cast<size_t>(fsize), 1, fp);
    ret[fsize] = '\0';
    std::string str_ret(ret);
    delete ret;
    fclose(fp);

    return str_ret;
}

static unsigned attrs_func_default = JSPROP_ENUMERATE | JSPROP_PERMANENT | JSFUN_STUB_GSOPS;

inline void spd_gc_callback(JSRuntime *rt, JSGCStatus status, void *data) {
    printf("mozjs - GC triggered, status %d\n", (int) status); }

}

#endif
