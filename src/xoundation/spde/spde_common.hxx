//
// Created by secondwtq 15-8-17.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_SPDE_COMMON_HXX
#define MOZJS_SPDE_COMMON_HXX

#define JS_NATIVEFUNC
#define SPD_PUBLICAPI

namespace xoundation {
namespace spd {

enum LifetimeType {
    UseJSLifetime,
    UseCXXLifetime,
    UseSharedLifetime
};

}
}

#endif // MOZJS_SPDE_COMMON_HXX
