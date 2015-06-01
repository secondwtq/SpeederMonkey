//
// Created by secondwtq <lovejay-lovemusic@outlook.com> 2015/06/01.
// Copyright (c) 2015 SCU ISDC All rights reserved.
//
// This file is part of ISDCNext.
//
// We have always treaded the borderland.
//

#ifndef MOZJS_SPDE_VIVALAVIDA_HXX
#define MOZJS_SPDE_VIVALAVIDA_HXX

namespace xoundation {
namespace spd {

enum lifetime_construct {
    LIFETIME_PLACEMENT_CONSTRUCT
};

template<typename T>
struct lifetime {

    virtual T *get() = 0;

    virtual ~lifetime() { }

};

template<typename T>
struct lifetime_cxx : public lifetime<T> {

    lifetime_cxx(const T *ref) : m_ptr(const_cast<T *>(ref)) { }

    T *get() override { return m_ptr; }

    private:
    T *m_ptr;

};

template<typename T>
struct lifetime_js : public lifetime<T> {

    lifetime_js(lifetime_construct) { }

    lifetime_js(const T& ref) {
        new (this->get()) T(ref); }

    T *get() override { return reinterpret_cast<T *>(m_data); }

    ~lifetime_js() {
        reinterpret_cast<T *>(m_data)->~T(); }

    private:

    char m_data[sizeof(T)];
};

}
}

#endif // MOZJS_SPDE_VIVALAVIDA_HXX
