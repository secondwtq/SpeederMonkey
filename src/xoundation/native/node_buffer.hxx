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

#include "xoundation/thirdpt/js_engine.hxx"
#include <xoundation/spde.hpp>
#include <xoundation/spde_helper.hxx>
#include <utility>
#include <algorithm>
#include <string.h>

namespace xoundation {

namespace native {

namespace helpers {

inline std::pair<bool, size_t> arg_arrayidx(JS::HandleValue arg, size_t def) {
    if (arg.isUndefined()) { return { true, def }; }
    int t = arg.isNumber() ? arg.toInt32() : def;
    if (t < 0) { return { false, 0 }; }
    return { true, static_cast<size_t>(t) };
}

}

#define CHECK_N_THROW(e, ext) { \
    std::pair<bool, size_t> t = (e); \
    if (!t.first) { JS_ReportError(ctx, "out of range index"); } \
    else { ext = t.second; } }

namespace strbytes {

enum encoding { ASCII, UTF8, BASE64, UCS2, BINARY, HEX, BUFFER };

size_t write(char *buf, size_t buflen, char *org, size_t orglen, enum encoding encoding) {
    int written = 0;
    size_t len = std::min(buflen, orglen);
    switch (encoding) {
        case ASCII:
        case BINARY:
        case BUFFER:
            memcpy(buf, org, len);
            written = len;
            break;

        case UTF8:
            memcpy(buf, org, len);
            break;

        case UCS2:
            memcpy(buf, org, len*2);
            written = len;
            len *= sizeof(unsigned int);
            break;

        case BASE64:
            break;

        case HEX:
            break;

        default:
            assert(0 && "unknown encoding");
            break;
    }

    return len;
}

}

class buffer {
    public:

    ~buffer() { this->_realloc(0); }

    size_t _realloc(size_t size) {
        if (m_data != nullptr) {
            delete m_data;
            m_data = nullptr;
            m_len = 0;
        }
        if (size > 0) {
            m_data = new char[size+4];
            m_len = size;
        }
    }

    size_t _truncate(size_t size) {
        if (size <= this->m_len) {
            this->m_len = size; }
    }

    inline size_t _length() { return this->m_len; }
    inline char *_data() { return this->m_data; }

    bool _copy(JSContext *ctx, JS::CallArgs args) {
        buffer *target = spd::caster<buffer *>::back(ctx, args[0]);

        size_t target_start = 0, source_start = 0, source_end = 0;
        CHECK_N_THROW(helpers::arg_arrayidx(args[1], 0), target_start);
        CHECK_N_THROW(helpers::arg_arrayidx(args[2], 0), source_start);
        CHECK_N_THROW(helpers::arg_arrayidx(args[3], this->_length()), source_end);

        if (target_start >= target->_length() || source_start >= source_end) {
            args.rval().setInt32(0); }
        if (source_start > this->_length()) {
            JS_ReportError(ctx, "out of range index"); }
        if (source_end - source_start > target->_length() - target_start) {
            source_end = source_start + target->_length() - target_start; }

        size_t to_copy = std::min({ source_end - source_start, target->_length() - target_start,
                                      this->_length() - source_start });
        memmove(target->_data() + target_start, this->_data() + source_start, to_copy);
        args.rval().setInt32(static_cast<int>(to_copy));
        return true;
    }

    void _fill_num(unsigned int _value, size_t start, size_t end) {
        size_t length = end - start;
        if (length + start > this->_length()) {
            /* TODO: error handling */ }

        int value = _value & 255;
        memset(this->_data() + start, value, length);
    }

    inline static int _compare(buffer *a, buffer *b) {
        char *pa = a->_data(), *pb = b->_data();
        size_t la = a->_length(), lb = b->_length();
        size_t lencmp = std::min(la, lb);

        int ret = memcmp(pa, pb, lencmp);

        if (!ret) {
            if (la > lb) ret = 1;
            else if (la < lb) ret = -1;
        } else if (ret > 0) { ret = 1; }
        else { ret = -1; }

        return ret;
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
                .method<decltype(&buffer::_truncate), &buffer::_truncate>("_truncate")
                .method<decltype(&buffer::_fill_num), &buffer::_fill_num>("_fill_num")
                .raw_method<&buffer::_copy>("_copy", 4)
                .static_func<decltype(buffer::_compare), buffer::_compare>("_compare");
}

}

}

#endif // MOZJS_NODE_BUFFER_HXX
