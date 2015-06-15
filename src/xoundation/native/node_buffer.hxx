//
// Created by secondwtq <lovejay-lovemusic@outlook.com> 2015/06/15.
// Copyright (c) 2015 SCU ISDC All rights reserved.
//
// This file is part of ISDCNext.
//
// We have always treaded the borderland.
//

#ifndef MOZJS_NODE_BUFFER_HXX
#define MOZJS_NODE_BUFFER_HXX

#include <jsapi.h>
#include <xoundation/spde.hpp>
#include <xoundation/spde_helper.hxx>

namespace xoundation {

namespace native {

class buffer {
    public:

    size_t _realloc(size_t size) {
        if (m_data != nullptr) {
            delete m_data;
            m_data = nullptr;
            m_len = 0;
        }
        m_data = new char[size+4];
        m_len = size;
    }

    size_t _length() { return this->m_len; }
    char *_data() { return this->m_data; }
    bool _copy(JSContext *ctx, JS::CallArgs args) {

    }


    private:

        char *m_data = nullptr;
        size_t m_len = 0;
};

void register_interface_buffer(JSContext *context, JS::HandleObject parent) {
    spd::class_info<buffer>::inst_wrapper::set(new spd::class_info<buffer>(context));
    klass<buffer>().define<>("_native_buffer", parent)
                .method<decltype(&buffer::_length), &buffer::_length>("_length")
                .method<decltype(&buffer::_data), &buffer::_data>("_data")
                .method<decltype(&buffer::_realloc), &buffer::_realloc>("_realloc")
                .raw_method<&buffer::_copy>("_copy", 4);
}

}

}

#endif // MOZJS_NODE_BUFFER_HXX
