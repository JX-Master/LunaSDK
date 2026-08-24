/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Error.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "../Error.hpp"
#include "Platform/Result.hpp"
#include "../Result.hpp"

namespace Luna
{
    inline RV encode_platform_result(Platform::Result res)
    {
        switch(res)
        {
            case Platform::Result::success:                     return ok;
            case Platform::Result::not_found:                   return E_NOT_FOUND;
            case Platform::Result::already_exists:              return E_ALREADY_EXISTS;
            case Platform::Result::not_unique:                  return E_NOT_UNIQUE;
            case Platform::Result::bad_arguments:               return E_BAD_ARGUMENTS;
            case Platform::Result::bad_calling_time:            return E_BAD_CALLING_TIME;
            case Platform::Result::out_of_memory:               return E_OUT_OF_MEMORY;
            case Platform::Result::not_supported:               return E_NOT_SUPPORTED;
            case Platform::Result::bad_platform_call:           return E_BAD_PLATFORM_CALL;
            case Platform::Result::access_denied:               return E_ACCESS_DENIED;
            case Platform::Result::not_directory:               return E_NOT_DIRECTORY;
            case Platform::Result::is_directory:                return E_IS_DIRECTORY;
            case Platform::Result::directory_not_empty:         return E_DIRECTORY_NOT_EMPTY;
            case Platform::Result::bad_file:                    return E_BAD_FILE;
            case Platform::Result::io_error:                    return E_IO_ERROR;
            case Platform::Result::timeout:                     return E_TIMEOUT;
            case Platform::Result::data_too_big:                return E_DATA_TOO_BIG;
            case Platform::Result::insufficient_user_buffer:    return E_INSUFFICIENT_USER_BUFFER;
            case Platform::Result::not_ready:                   return E_NOT_READY;
            case Platform::Result::out_of_range:                return E_OUT_OF_RANGE;
            case Platform::Result::out_of_resource:             return E_OUT_OF_RESOURCE;
            case Platform::Result::insufficient_system_buffer:  return E_INSUFFICIENT_SYSTEM_BUFFER;
            case Platform::Result::format_error:                return E_FORMAT_ERROR;
            case Platform::Result::interrupted:                 return E_INTERRUPTED;
            case Platform::Result::end_of_file:                 return E_END_OF_FILE;
            case Platform::Result::null_value:                  return E_NULL_VALUE;
            case Platform::Result::bad_cast:                    return E_BAD_CAST;
            case Platform::Result::in_progress:                 return E_IN_PROGRESS;
            case Platform::Result::version_dismatch:            return E_VERSION_DISMATCH;
            case Platform::Result::no_data:                     return E_NO_DATA;
            case Platform::Result::bad_data:                    return E_BAD_DATA;
            case Platform::Result::bad_memory_address:                 return E_BAD_MEMORY_ADDRESS;
            case Platform::Result::deadlock:                    return E_DEADLOCK;
            case Platform::Result::not_permitted:               return E_NOT_PERMITTED;
            case Platform::Result::busy:                        return E_BUSY;
            case Platform::Result::file_to_big:                 return E_FILE_TOO_BIG;
            case Platform::Result::not_configured:              return E_NOT_CONFIGURED;
            case Platform::Result::bad_pipe:                    return E_BAD_PIPE;
            case Platform::Result::path_too_long:               return E_PATH_TOO_LONG;
            case Platform::Result::loop:                        return E_LOOP;
            default: lupanic(); break;
        }
        return E_FAILURE;
    }
}