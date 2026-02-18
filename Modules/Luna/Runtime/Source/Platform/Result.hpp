/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Result.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "../../Base.hpp"

namespace Luna
{
    namespace Platform
    {
        enum class Result : i32
        {
            success = 0,
            not_found,
            already_exists,
            not_unique,
            bad_arguments,
            bad_calling_time,
            out_of_memory,
            not_supported,
            bad_platform_call,
            access_denied,
            not_directory,
            is_directory,
            directory_not_empty,
            bad_file,
            io_error,
            timeout,
            data_too_long,
            insufficient_user_buffer,
            not_ready,
            out_of_range,
            out_of_resource,
            insufficient_system_buffer,
            format_error,
            interrupted,
            end_of_file,
            null_value,
            bad_cast,
            in_progress,
            version_dismatch,
            no_data,
            bad_data,
            bad_address,
            deadlock
        };
    }
}