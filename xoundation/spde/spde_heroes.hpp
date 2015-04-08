//
// Created by secondwtq 15-5-5.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef HEADER_SPEEDER_MONKEY_HEROES
#define HEADER_SPEEDER_MONKEY_HEROES

#include <cstddef>

namespace spd {

template <size_t ... idxs>
struct indices { };

template <size_t N, size_t ... idxs>
struct indices_builder : indices_builder<N-1, N-1, idxs ...> { };

template <size_t ... idxs>
struct indices_builder<0, idxs ...>
    { using type = indices<idxs ...>; };

}

#endif
