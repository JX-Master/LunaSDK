/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Errno.hpp
* @author JXMaster
* @date 2020/9/22
 */
#pragma once
#include <errno.h>
#include "../Result.hpp"

namespace Luna
{
    namespace Platform
    {
        inline Result encode_errno(int err)
        {
            switch(err)
            {
                case EPERM:
                    return Result::not_permitted;
                case ENOENT:
                    return Result::not_found;
                case ESRCH:
                    return Result::not_found;
                case EINTR:
                    return Result::interrupted;
                case EIO:
                    return Result::io_error;
                case ENXIO:
                    return Result::not_configured;
                case E2BIG:
                    return Result::data_too_big;
                case ENOEXEC:
                    return Result::not_found;
                case EBADF:
                    return Result::bad_file;
                case ECHILD:
                    return Result::not_found;
                case EDEADLK:
                    return Result::deadlock;
                case ENOMEM:
                    return Result::out_of_memory;
                case EACCES:
                    return Result::access_denied;
                case EFAULT:
                    return Result::bad_memory_address;
                case ENOTBLK:
                    return Result::bad_calling_time;
                case EBUSY:
                    return Result::busy;
                case EEXIST:
                    return Result::already_exists;
                case EXDEV:
                    return Result::not_supported;
                case ENODEV:
                    return Result::not_supported;
                case ENOTDIR:
                    return Result::not_directory;
                case EISDIR:
                    return Result::is_directory;
                case EINVAL:
                    return Result::bad_arguments;
                case ENFILE:
                    return Result::out_of_resource;
                case EMFILE:
                    return Result::out_of_resource;
                case ENOTTY:
                    return Result::not_supported;
                case ETXTBSY:
                    return Result::busy;
                case EFBIG:
                    return Result::file_to_big;
                case ENOSPC:
                    return Result::out_of_resource;
                case ESPIPE:
                    return Result::bad_arguments;
                case EROFS:
                    return Result::not_supported;
                case EMLINK:
                    return Result::out_of_resource;
                case EPIPE:
                    return Result::bad_pipe;
                case EDOM:
                    return Result::out_of_range;
                case ERANGE:
                    return Result::out_of_range;
                case EAGAIN:
                    return Result::not_ready;
                case EINPROGRESS:
                    return Result::in_progress;
                case EALREADY:
                    return Result::in_progress;
                case ENAMETOOLONG:
                    return Result::path_too_long;
                case ELOOP:
                    return Result::loop;
                default:
                    return Result::bad_platform_call;
            }
        }
    }
}