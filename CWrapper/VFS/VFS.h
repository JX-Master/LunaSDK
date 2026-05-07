#pragma once

#include "../Runtime/Runtime.h"

#include <stdint.h>

#if defined(_WIN32)
#define LUNA_VFS_C_API __declspec(dllexport)
#else
#define LUNA_VFS_C_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

LUNA_VFS_C_API luna_errcode_t luna_vfs_init_module(void);
LUNA_VFS_C_API luna_errcode_t luna_vfs_mount(const char* driver, const char* driver_path, const char* mount_path);
LUNA_VFS_C_API luna_errcode_t luna_vfs_unmount(const char* mount_path);
LUNA_VFS_C_API luna_errcode_t luna_vfs_remount(const char* from_path, const char* to_path);
LUNA_VFS_C_API luna_errcode_t luna_vfs_open_file(const char* path, uint32_t flags, uint32_t creation, LunaFileHandle* out_file);
LUNA_VFS_C_API luna_errcode_t luna_vfs_get_file_attribute(const char* path, LunaFileAttribute* out_attribute);
LUNA_VFS_C_API luna_errcode_t luna_vfs_copy_file(const char* from_path, const char* to_path);
LUNA_VFS_C_API luna_errcode_t luna_vfs_move_file(const char* from_path, const char* to_path);
LUNA_VFS_C_API luna_errcode_t luna_vfs_delete_file(const char* path);
LUNA_VFS_C_API luna_errcode_t luna_vfs_open_dir(const char* path, LunaFileIteratorHandle* out_iterator);
LUNA_VFS_C_API luna_errcode_t luna_vfs_create_dir(const char* path);
LUNA_VFS_C_API luna_errcode_t luna_vfs_get_native_path(const char* vfs_path, const char** out_path);
LUNA_VFS_C_API luna_errcode_t luna_vfs_get_platform_filesystem_driver(const char** out_name);

#ifdef __cplusplus
}
#endif
