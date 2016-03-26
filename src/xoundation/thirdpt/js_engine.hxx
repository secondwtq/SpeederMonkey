//
// Created by secondwtq 15-8-3.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_JS_ENGINE_HXX
#define MOZJS_JS_ENGINE_HXX

//
// why SpiderMonkey instead of JavaScriptCore?
//  check it out: github.com/jxcore/jxcore/issues/186
//
// though I'm really planning something about JavaScriptCore
//  after all, WebKit seems more embeddable than Gecko as an UI Engine
//  and we are not targeting iOS, and we are ALREADY working on WebKit.
//
// again, FUCK V8.
//

#define SPD_ENGINE_VERMAJOR 31
//#define SPD_ENGINE_VERMAJOR 38

#if defined(SPD_ENGINE_VERMAJOR)
#if SPD_ENGINE_VERMAJOR == 31
#define SPD_MOZJS31 1
#elif SPD_ENGINE_VERMAJOR == 38
#define SPD_MOZJS38 1
#else

#endif
#else

#endif

// #include <jsapi.h>
#if defined(SPD_MOZJS31)
#include <mozjs-31/jsapi.h>
#endif

#if defined(SPD_MOZJS38)
#include <mozjs-38/jsapi.h>
#include <mozjs-38/js/Conversions.h>
#endif

#endif // MOZJS_JS_ENGINE_HXX
