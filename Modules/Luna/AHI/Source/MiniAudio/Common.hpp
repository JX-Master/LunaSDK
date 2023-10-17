/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Common.hpp
* @author JXMaster
* @date 2023/10/15
*/
#pragma once
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_ENGINE
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NODE_GRAPH
#include <miniaudio.h>
#include "../../AHIError.hpp"
#include "../../Adapter.hpp"

namespace Luna
{
    namespace AHI
    {
        inline ErrCode translate_ma_result(ma_result result)
        {
            switch(result)
            {
                case MA_SUCCESS: return ErrCode(0);
                case MA_INVALID_ARGS: return BasicError::bad_arguments();
                case MA_INVALID_OPERATION: return BasicError::not_supported();
                case MA_OUT_OF_MEMORY: return BasicError::out_of_memory();
                case MA_OUT_OF_RANGE: return BasicError::out_of_range();
                case MA_ACCESS_DENIED: return BasicError::access_denied();
                case MA_DOES_NOT_EXIST: return BasicError::not_found();
                case MA_ALREADY_EXISTS: return BasicError::already_exists();
                case MA_TOO_MANY_OPEN_FILES: return BasicError::out_of_resource();
                case MA_INVALID_FILE: return BasicError::bad_file();
                case MA_TOO_BIG: return BasicError::data_too_long();
                case MA_PATH_TOO_LONG: return BasicError::data_too_long();
                case MA_NAME_TOO_LONG: return BasicError::data_too_long();
                case MA_NOT_DIRECTORY: return BasicError::not_directory();
                case MA_IS_DIRECTORY: return BasicError::is_directory();
                case MA_DIRECTORY_NOT_EMPTY: return BasicError::directory_not_empty();
                case MA_AT_END: return BasicError::end_of_file();
                case MA_NO_SPACE: return BasicError::insufficient_system_buffer();
                case MA_BUSY: return BasicError::not_ready();
                case MA_IO_ERROR: return BasicError::io_error();
                case MA_INTERRUPT: return BasicError::interrupted();
                case MA_UNAVAILABLE: return BasicError::not_supported();
                case MA_BAD_ADDRESS: return BasicError::bad_address();
                case MA_DEADLOCK: return BasicError::deadlock();
                case MA_TOO_MANY_LINKS: return BasicError::out_of_resource();
                case MA_NOT_IMPLEMENTED: return BasicError::not_supported();
                case MA_NO_DATA_AVAILABLE: return BasicError::no_data();
                case MA_INVALID_DATA: return BasicError::bad_data();
                case MA_TIMEOUT: return BasicError::timeout();
                case MA_NOT_UNIQUE: return BasicError::not_unique();
                case MA_IN_PROGRESS: return BasicError::not_ready();
                case MA_CANCELLED: return BasicError::interrupted();
                case MA_CRC_MISMATCH: return BasicError::bad_data();
                case MA_FORMAT_NOT_SUPPORTED: return AHIError::format_not_supported();
                case MA_DEVICE_TYPE_NOT_SUPPORTED: return AHIError::device_type_not_supported();
                case MA_SHARE_MODE_NOT_SUPPORTED: return AHIError::share_mode_not_supported();
                case MA_NO_BACKEND: return AHIError::no_backend();
                case MA_NO_DEVICE: return AHIError::no_device();
                case MA_API_NOT_FOUND: return AHIError::api_not_found();
                case MA_INVALID_DEVICE_CONFIG: return AHIError::bad_device_config();
                case MA_LOOP: return AHIError::loop();
                case MA_BACKEND_NOT_ENABLED: return AHIError::backend_not_enabled();
                case MA_DEVICE_NOT_INITIALIZED: return BasicError::bad_calling_time();
                case MA_DEVICE_ALREADY_INITIALIZED: return BasicError::bad_calling_time();
                case MA_DEVICE_NOT_STARTED: return AHIError::device_not_started();
                case MA_DEVICE_NOT_STOPPED: return AHIError::device_not_stopped();
                case MA_FAILED_TO_INIT_BACKEND: return AHIError::failed_to_init_backend();
                case MA_FAILED_TO_OPEN_BACKEND_DEVICE: return AHIError::failed_to_open_backend_device();
                case MA_FAILED_TO_START_BACKEND_DEVICE: return AHIError::failed_to_start_backend_device();
                case MA_FAILED_TO_STOP_BACKEND_DEVICE: return AHIError::failed_to_stop_backend_device();
                default: return BasicError::failure();
            }
        }

        inline ma_format encode_format(BitDepth bit_depth)
        {
            switch(bit_depth)
            {
                case BitDepth::unspecified: return ma_format_unknown;
                case BitDepth::u8: return ma_format_u8;
                case BitDepth::s16: return ma_format_s16;
                case BitDepth::s24: return ma_format_s24;
                case BitDepth::s32: return ma_format_s32;
                case BitDepth::f32: return ma_format_f32;
                default: break;
            }
            lupanic();
            return ma_format_unknown;
        }

        inline BitDepth decode_bit_depth(ma_format format)
        {
            switch(format)
            {
                case ma_format_unknown: return BitDepth::unspecified;
                case ma_format_u8: return BitDepth::u8;
                case ma_format_s16: return BitDepth::s16;
                case ma_format_s24: return BitDepth::s24;
                case ma_format_s32: return BitDepth::s32;
                case ma_format_f32: return BitDepth::f32;
                default: break;
            }
            lupanic();
            return BitDepth::unspecified;
        }

        extern ma_context g_context;
    }
}