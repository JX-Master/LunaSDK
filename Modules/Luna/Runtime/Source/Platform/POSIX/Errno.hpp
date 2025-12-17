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

namespace Luna
{
    namespace OS
    {
        inline const c8* display_errno(int err)
        {
            switch(err)
            {
                case EPERM:
                    return "EPERM Operation not permitted";
                case ENOENT:
                    return "ENOENT No such file or directory";
                case ESRCH:
                    return "ESRCH No such process";
                case EINTR:
                    return "EINTR Interrupted system call";
                case EIO:
                    return "EIO Input/output error";
                case ENXIO:
                    return "ENXIO Device not configured";
                case E2BIG:
                    return "E2BIG Argument list too long";
                case ENOEXEC:
                    return "ENOEXEC Exec format error";
                case EBADF:
                    return "EBADF Bad file descriptor";
                case ECHILD:
                    return "ECHILD No child processes";
                case EDEADLK:
                    return "Resource deadlock avoided";
                case ENOMEM:
                    return "ENOMEM Cannot allocate memory";
                case EACCES:
                    return "EACCES Permission denied";
                case EFAULT:
                    return "EFAULT Bad address";
                case ENOTBLK:
                    return "ENOTBLK Block device required";
                case EBUSY:
                    return "EBUSY Device / Resource busy";
                case EEXIST:
                    return "EEXIST File exists";
                case EXDEV:
                    return "EXDEV Cross-device link";
                case ENODEV:
                    return "ENODEV Operation not supported by device";
                case ENOTDIR:
                    return "ENOTDIR Not a directory";
                case EISDIR:
                    return "EISDIR Is a directory";
                case EINVAL:
                    return "EINVAL Invalid argument";
                case ENFILE:
                    return "ENFILE Too many open files in system";
                case EMFILE:
                    return "EMFILE Too many open files";
                case ENOTTY:
                    return "ENOTTY Inappropriate ioctl for device";
                case ETXTBSY:
                    return "ETXTBSY Text file busy";
                case EFBIG:
                    return "EFBIG File too large";
                case ENOSPC:
                    return "ENOSPC No space left on device";
                case ESPIPE:
                    return "ESPIPE Illegal seek";
                case EROFS:
                    return "EROFS Read-only file system";
                case EMLINK:
                    return "EMLINK Too many links";
                case EPIPE:
                    return "EPIPE Broken pipe";
                case EDOM:
                    return "EDOM Numerical argument out of domain";
                case ERANGE:
                    return "ERANGE Result too large";
                case EAGAIN:
                    return "EAGAIN|EWOULDBLOCK Resource temporarily unavailable / Operation would block";
                case EINPROGRESS:
                    return "EINPROGRESS Operation now in progress";
                case EALREADY:
                    return "EALREADY Operation already in progress";
                default:
                    return "Undefined Error Code";
            }
        }
    }
}