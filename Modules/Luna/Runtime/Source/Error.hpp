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
            case Platform::Result::not_found:                   return BasicError::not_found();
            case Platform::Result::already_exists:              return BasicError::already_exists();
            case Platform::Result::not_unique:                  return BasicError::not_unique();
            case Platform::Result::bad_arguments:               return BasicError::bad_arguments();
            case Platform::Result::bad_calling_time:            return BasicError::bad_calling_time();
            case Platform::Result::out_of_memory:               return BasicError::out_of_memory();
            case Platform::Result::not_supported:               return BasicError::not_supported();
            case Platform::Result::bad_platform_call:           return BasicError::bad_platform_call();
            case Platform::Result::access_denied:               return BasicError::access_denied();
            case Platform::Result::not_directory:               return BasicError::not_directory();
            case Platform::Result::is_directory:                return BasicError::is_directory();
            case Platform::Result::directory_not_empty:         return BasicError::directory_not_empty();
            case Platform::Result::bad_file:                    return BasicError::bad_file();
            case Platform::Result::io_error:                    return BasicError::io_error();
            case Platform::Result::timeout:                     return BasicError::timeout();
            case Platform::Result::data_too_big:                return BasicError::data_too_big();
            case Platform::Result::insufficient_user_buffer:    return BasicError::insufficient_user_buffer();
            case Platform::Result::not_ready:                   return BasicError::not_ready();
            case Platform::Result::out_of_range:                return BasicError::out_of_range();
            case Platform::Result::out_of_resource:             return BasicError::out_of_resource();
            case Platform::Result::insufficient_system_buffer:  return BasicError::insufficient_system_buffer();
            case Platform::Result::format_error:                return BasicError::format_error();
            case Platform::Result::interrupted:                 return BasicError::interrupted();
            case Platform::Result::end_of_file:                 return BasicError::end_of_file();
            case Platform::Result::null_value:                  return BasicError::null_value();
            case Platform::Result::bad_cast:                    return BasicError::bad_cast();
            case Platform::Result::in_progress:                 return BasicError::in_progress();
            case Platform::Result::version_dismatch:            return BasicError::version_dismatch();
            case Platform::Result::no_data:                     return BasicError::no_data();
            case Platform::Result::bad_data:                    return BasicError::bad_data();
            case Platform::Result::bad_memory_address:                 return BasicError::bad_memory_address();
            case Platform::Result::deadlock:                    return BasicError::deadlock();
            case Platform::Result::not_permitted:               return BasicError::not_permitted();
            case Platform::Result::busy:                        return BasicError::busy();
            case Platform::Result::file_to_big:                 return BasicError::file_to_big();
            case Platform::Result::not_configured:              return BasicError::not_configured();
            case Platform::Result::bad_pipe:                    return BasicError::bad_pipe();
            case Platform::Result::path_too_long:               return BasicError::path_too_long();
            case Platform::Result::loop:                        return BasicError::loop();
            default: lupanic(); break;
        }
        return BasicError::failure();
    }
}