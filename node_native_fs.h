//
// Created by secondwtq 15-5-7.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_NODE_NATIVE_FS_H
#define MOZJS_NODE_NATIVE_FS_H

#include <string>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <stdlib.h>

#include <jsapi.h>
#include "spde/spde.hpp"
#include "spde_test_common.h"

namespace xoundation {

namespace node_native {

class stats {
    public:

    stats () { }
    stats (const struct stat *raw) : m_stat(*raw) { }

    bool is_file() {
        return S_ISREG(this->m_stat.st_mode); }
    bool is_directory() {
        return S_ISDIR(this->m_stat.st_mode); }
    bool is_block_dev() {
        return S_ISBLK(m_stat.st_mode); }
    bool is_char_dev() {
        return S_ISCHR(m_stat.st_mode); }
    bool is_symb_link() {
        return S_ISLNK(m_stat.st_mode); }
    bool is_fifo() {
        return S_ISFIFO(m_stat.st_mode); }
    bool is_socket() {
        return S_ISSOCK(m_stat.st_mode); }

    private:

    struct stat m_stat;
};

bool fs_exists_sync(const std::string& path) {
    if (access(path.c_str(), F_OK) != -1)
        return true;
    return false;
}

std::string os_platform() {
    return "darwin"; }

std::string fs_read_file_sync(const std::string& filename) {
    FILE *fp = fopen(filename.c_str(), "r");
    if (fp == nullptr) {
        /* error */ }

    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp);
    rewind(fp);

    char *ret = new char[fsize + 1];
    fread(ret, fsize, 1, fp);
    ret[fsize] = '\0';
    std::string str_ret(ret);
    delete ret;
    fclose(fp);

    return str_ret;
}

void fs_write_file_sync(const std::string& filename, const std::string& data,
                        const std::string& opt) {
    FILE *fp = fopen(filename.c_str(), "w");
    if (fp == nullptr) {
        /* error */ }

    fwrite(data.c_str(), data.length(), 1, fp);

    fclose(fp);
}

// fs.mkdirSync(path[, mode])
// no 'mode' support currently
void fs_mkdir_sync(const std::string& path) {
    mkdir(path.c_str(), 0777); }

// stat_sync
stats fs_stat_sync(const std::string& path) {
    struct stat stat_buf;
    stat(path.c_str(), &stat_buf);
    return stats(&stat_buf);
}

// lstat_sync
stats fs_lstat_sync(const std::string& path) {
    struct stat stat_buf;
    lstat(path.c_str(), &stat_buf);
    return stats(&stat_buf);
}

// readdir_sync
// fstat_sync
// write_sync
//      currently we only support simple fd and data args
int fs_write_sync(int fd, const std::string& data) {
    return write(fd, data.c_str(), data.length()); }

void register_interface_fs(JSContext *context, JS::Handle<JSObject *> parent) {

    JS::RootedObject node_fs(context, JS_DefineObject(context, parent, "_node_native_fs", nullptr, nullptr,
                                                      JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY));

    spd::class_info<stats>::inst_wrapper::set(new spd::class_info<stats>(context));
    spd::class_helper<stats>::ctor_wrapper<>::define("Stats", parent);
    spd::class_helper<stats>::method_callback_wrapper<decltype(&stats::is_file),
                                                    &stats::is_file>::register_as("isFile");
    spd::class_helper<stats>::method_callback_wrapper<decltype(&stats::is_directory),
                                                    &stats::is_directory>::register_as("isDirectory");

    JS_DefineFunction(context, node_fs, "existsSync", spd::function_callback_wrapper<decltype
                            (fs_exists_sync), fs_exists_sync>::callback, 1, attrs_func_default);
    JS_DefineFunction(context, node_fs, "readFileSync", spd::function_callback_wrapper<decltype
                            (fs_read_file_sync), fs_read_file_sync>::callback, 1, attrs_func_default);
    JS_DefineFunction(context, node_fs, "writeFileSync", spd::function_callback_wrapper<decltype
                            (fs_write_file_sync), fs_write_file_sync>::callback, 3, attrs_func_default);
    JS_DefineFunction(context, node_fs, "mkdirSync", spd::function_callback_wrapper<decltype
                            (fs_mkdir_sync), fs_mkdir_sync>::callback, 1, attrs_func_default);
    JS_DefineFunction(context, node_fs, "statSync", spd::function_callback_wrapper<decltype
                            (fs_stat_sync), fs_stat_sync>::callback, 1, attrs_func_default);
    JS_DefineFunction(context, node_fs, "lstatSync", spd::function_callback_wrapper<decltype
                            (fs_lstat_sync), fs_lstat_sync>::callback, 1, attrs_func_default);
    JS_DefineFunction(context, node_fs, "writeSync", spd::function_callback_wrapper<decltype
                            (fs_write_sync), fs_write_sync>::callback, 2, attrs_func_default);
}

void register_interface_os(JSContext *context, JS::Handle<JSObject *> parent) {
    JS::RootedObject node_os(context, JS_DefineObject(context, parent, "_node_native_os", nullptr,
                                                      nullptr, JSPROP_PERMANENT |
                                                               JSPROP_ENUMERATE | JSPROP_READONLY));

    JS_DefineFunction(context, node_os, "platform", spd::function_callback_wrapper<decltype
                                        (os_platform), os_platform>::callback, 0, attrs_func_default);

    // Windows: \r\n, UNIX: \n, Classic MacOS: \r
    JS::RootedString eol_str(context, JS_NewStringCopyZ(context, "\n"));
    JS_DefineProperty(context, node_os, "EOL", eol_str,
                      JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY);
}

void register_interface_path(JSContext *context, JS::Handle<JSObject *> parent) {
    JS::RootedObject node_path(context, JS_DefineObject(context, parent, "_node_native_path",
                                                        nullptr, nullptr, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT));
}

std::string process_cwd() {
    const size_t buffer_size = 256;
    char *buf = reinterpret_cast<char *>(malloc(sizeof(char) * (1 + buffer_size)));
    if (getcwd(buf, buffer_size) == nullptr) {
        /* error */ }
    std::string ret { buf };
    free(buf); // TODO: we need a helper class for such buffer
    buf = nullptr;
    return ret;
}

void process_exit(int status) {
    exit(status); }

void register_interface_process_args(JSContext *context, JS::HandleObject process, int argc,
                                     const char *argv[]) {
    JS::AutoValueVector at_vec(context);
    for (size_t i = 0; i < argc; i++)
        at_vec.append(spd::caster<const std::string&>::tojs(context, { argv[i] }));

    JS::RootedObject argv_array(context, JS_NewArrayObject(context, at_vec));

    JS_DefineProperty(context, process, "argv", argv_array, JSPROP_ENUMERATE | JSPROP_PERMANENT |
                                                            JSPROP_READONLY);
}

void register_interface_process(JSContext *context, JS::HandleObject parent, int argc, const char
                                * argv[]) {
    JS::RootedObject node_process(context, JS_DefineObject(context, parent, "process", nullptr, nullptr,
                                                           JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY));

    JS_DefineFunction(context, node_process, "cwd", spd::function_callback_wrapper<decltype
                                        (process_cwd), process_cwd>::callback, 0, attrs_func_default);
    JS_DefineFunction(context, node_process, "exit", spd::function_callback_wrapper<decltype
                                        (process_exit), process_exit>::callback, 1, attrs_func_default);

    register_interface_process_args(context, node_process, argc, argv);
}

}

}

#endif
