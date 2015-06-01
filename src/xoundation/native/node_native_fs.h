//
// Created by secondwtq 15-5-7.
// Copyright (c) 2015 The Xoundation Project All rights reserved.
//

#ifndef MOZJS_NODE_NATIVE_FS_H
#define MOZJS_NODE_NATIVE_FS_H

#include <xoundation/platform.h>

#include <string>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <dirent.h>

#include <limits.h>

// PATH_MAX
#ifdef CUBE_PLATFORM_BSD_LIKE
#include <sys/syslimits.h>
#elif defined(CUBE_PLATFORM_LINUX)
// stackoverflow.com/questions/9449241/where-is-path-max-defined-in-linux
#include <linux/limits.h>
#endif

#include <jsapi.h>
#include <xoundation/spde.hpp>
#include <xoundation/spde/spde_test_common.h>

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

// used in CoffeeScript cake
bool fs_exists_sync(const std::string& path) {
    if (access(path.c_str(), F_OK) != -1)
        return true;
    return false;
}

std::string os_platform() {
    return "darwin"; }

// used in CoffeeScript cake
std::string fs_read_file_sync(const std::string& filename) {
    FILE *fp = fopen(filename.c_str(), "r");
    if (fp == nullptr) {
        /* error */ }

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
stats *fs_stat_sync(const std::string& path) {
    struct stat stat_buf;
    stat(path.c_str(), &stat_buf);
    return new stats(&stat_buf);
}

// lstat_sync
stats fs_lstat_sync(const std::string& path) {
    struct stat stat_buf;
    lstat(path.c_str(), &stat_buf);
    return stats(&stat_buf);
}

// fstat_sync
// stat
// watch
// exists
// mkdir
// write_file
// write_sync
// open_sync
// read_sync
// close
// process.stdout
// child_process
// vm.runInThisContext, vm.runInContext
// repl
// process.env
//      currently we only support simple fd and data args
long fs_write_sync(int fd, const std::string& data) {
    return write(fd, data.c_str(), data.length()); }

// no 'cache' support currently
//      used in node.js/lib/module::tryFile
//      CoffeeScript cake
std::string fs_realpath_sync(const std::string& path) {

    constexpr size_t len_execpath = 2 * PATH_MAX;
    char abspath[len_execpath] = { '\0' };

    realpath(path.c_str(), abspath);

    std::string ret(abspath);
    return ret;
}

// watch_file
//  used in TypeScript, but not sure why
// TODO: WIP since it needs a little more work.
bool fs_watch_file(JSContext *c, unsigned int argc, JS::Value *vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    std::string filename = spd::caster<const std::string&>::back(c, args[0]);
    (void) filename;

    return true;
}

// readdir_sync
// TODO: we need a wrapper class for JSAPI Arrays to generalize this function.
// P.S. : libuv@node.js uses scandir() with a filter function instead of opendir()
bool fs_readdir_sync(JSContext *c, unsigned int argc, JS::Value *vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    std::string path = spd::caster<const std::string&>::back(c, args[0]);

    DIR *dir = opendir(path.c_str());
    if (dir == 0) {
        return false; }

    struct dirent *entry;
    JS::AutoValueVector files_vec(c);
    while ((entry = readdir(dir)) != 0) {

        switch (entry->d_name[0]) {
            case '.':
                switch (entry->d_name[1]) {
                    case '\0': continue; // '.'
                    case '.':
                        switch (entry->d_name[2]) {
                            case '\0': continue; // '..'
                            default: break; // '\.\..*'
                        }
                    default: break;
                }
            default: break;
        }

        files_vec.append(spd::caster<const char *>::tojs(c, entry->d_name));
    }
    args.rval().setObject(*JS_NewArrayObject(c, files_vec));

    return true;
}

void fs_unlink_sync(const std::string& path) {
    if (unlink(path.c_str())) {
        // TODO: error handling of unlink_sync
    }
}

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
    JS_DefineFunction(context, node_fs, "realpathSync", spd::function_callback_wrapper<decltype
                            (fs_realpath_sync), fs_realpath_sync>::callback, 1, attrs_func_default);
    JS_DefineFunction(context, node_fs, "readdirSync", fs_readdir_sync, 1, attrs_func_default);
    JS_DefineFunction(context, node_fs, "unlinkSync", spd::function_callback_wrapper<decltype
                            (fs_unlink_sync), fs_unlink_sync>::callback, 1, attrs_func_default);
    JS_DefineFunction(context, node_fs, "watchFile", fs_watch_file, 2, attrs_func_default);
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

// used in TypeScript
// node.js/lib/path
// CoffeeScript cake
std::string process_cwd() {
    const size_t buffer_size = 256;
    char *buf = reinterpret_cast<char *>(malloc(sizeof(char) * (1 + buffer_size)));
    if (getcwd(buf, buffer_size) == nullptr) {
        /* error */ }
    std::string ret { buf };
    free(buf); // TODO: we need a helper class for such buffer
    buf = nullptr; (void) buf;
    return ret;
}

// used in CoffeeScript cake
// TODO: exception not implemented.
void process_chdir(const std::string& path) {
    if (chdir(path.c_str())) {
        printf("process::chdir failed!\n");
    }
}

// used in TypeScript
// used in CoffeeScript cake
void process_exit(int status) {
    exit(status); }

#ifdef CUBE_PLATFORM_MACH
#include <mach-o/dyld.h> // _NSGetExecutablePath
#endif

void register_interface_process_args(JSContext *context, JS::HandleObject process, int argc,
                                     const char *argv[]) {
    JS::AutoValueVector at_vec(context);
    for (size_t i = 0; i < argc; i++)
        at_vec.append(spd::caster<const std::string&>::tojs(context, { argv[i] }));

    JS::RootedObject argv_array(context, JS_NewArrayObject(context, at_vec));

    JS_DefineProperty(context, process, "argv", argv_array, JSPROP_ENUMERATE | JSPROP_PERMANENT |
                                                            JSPROP_READONLY);

    constexpr size_t len_execpath = 2 * PATH_MAX;
    char execpath[len_execpath] = { '\0' };
    char abspath[len_execpath] = { '\0' };

    // TODO: platform specific part for Windows!
    #ifdef CUBE_PLATFORM_MACH
    uint32_t darwin_exepath_len = len_execpath;
    if (_NSGetExecutablePath(execpath, &darwin_exepath_len) == 0 &&
            realpath(execpath, abspath) == abspath && strlen(abspath) > 0) {
        memcpy(execpath, abspath, strlen(abspath) + 1);
        memcpy(execpath, abspath, strlen(abspath) + 1);
    } else strcpy(execpath, argv[0]);
    #else
    size_t n = readlink("/proc/self/exe", execpath, len_execpath-1);
    if (n == -1) printf("Failed to get executable path\n"); // should be a LOG
    execpath[n] = '\0';
    #endif

    JS::RootedString exec_path_handle(context, JS_NewStringCopyZ(context, execpath));
    JS_DefineProperty(context, process, "execPath", exec_path_handle,
                      JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY);

}

void register_interface_process(JSContext *context, JS::HandleObject parent, int argc, const char
                                * argv[]) {
    JS::RootedObject node_process(context, JS_DefineObject(context, parent, "process", nullptr, nullptr,
                                                           JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY));

    JS_DefineFunction(context, node_process, "cwd", spd::function_callback_wrapper<std::string (),
                      process_cwd>::callback, 0, attrs_func_default);
    JS_DefineFunction(context, node_process, "exit", spd::function_callback_wrapper<decltype
                                        (process_exit), process_exit>::callback, 1, attrs_func_default);

    register_interface_process_args(context, node_process, argc, argv);
}

}

}

#endif
