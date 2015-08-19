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

#include <memory>

namespace xoundation {
namespace spd {

enum lifetime_construct {
    LIFETIME_PLACEMENT_CONSTRUCT };

struct lifetime_base {
    virtual void *raw_get() = 0;
    virtual void *raw_copy() = 0;
    virtual ~lifetime_base() { }
    bool is_intrusive = false;
};

// currently we support C++ Lifetime (through pointers / references)
//  and JS Lifetime (with values)
//  implementation just copied Userdata in LuaBridge
//  we does not, and may be never support shared lifetime.
//
template<typename T>
struct lifetime : lifetime_base {
    virtual T *get() = 0;
    virtual lifetime<T> *copy() = 0;

    void *raw_get() override {
        return this->get(); }
    void *raw_copy() override {
        return this->copy(); }
};

template<typename T>
struct lifetime_cxx : public lifetime<T> {

    lifetime_cxx(const T *ref) : m_ptr(const_cast<T *>(ref)) { }

    T *get() override { return m_ptr; }

    lifetime<T> *copy() override {
        lifetime_cxx<T> *ret = new lifetime_cxx<T>(get());
        return ret;
    }

private:
    T *m_ptr;
};

template<typename T>
struct lifetime_shared_std : public lifetime<T> {

    lifetime_shared_std(std::shared_ptr<T> ptr) : m_ptr(ptr) { }

    T *get() override { return m_ptr.get(); }
    std::shared_ptr<T> get_shared() { return m_ptr; }

    lifetime<T> *copy() override {
        lifetime_shared_std<T> *ret = new lifetime_shared_std<T>(get_shared());
        return ret;
    }

private:
    std::shared_ptr<T> m_ptr;
};

template<typename T>
struct lifetime_js : public lifetime<T> {

    struct datablock {
        // www.stroustrup.com/bs_faq2.html
        //  => 'Is there a "placement delete"?' <=
        ~datablock() {
            reinterpret_cast<T *>(m_data)->~T(); }
        char m_data[sizeof(T)];
    };

    lifetime_js(lifetime_construct): data(new datablock()) { }
    lifetime_js(const T& other): data(new datablock()) {
        new (this->get()) T(other); }

    T *get() override { return reinterpret_cast<T *>(data->m_data); }

    lifetime<T> *copy() override {
        lifetime_js<T> *ret = new lifetime_js<T>(data);
        return ret;
    }

private:
    lifetime_js(std::shared_ptr<datablock> data_ref): data(data_ref) { }

    std::shared_ptr<datablock> data;
};

}
}

#endif // MOZJS_SPDE_VIVALAVIDA_HXX
