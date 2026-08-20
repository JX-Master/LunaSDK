/*!
* This file is a portion of LunaSDK.
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
        inline ResultCode translate_ma_result(ma_result result)
        {
            switch(result)
            {
                case MA_SUCCESS: return ResultCode(0);
                case MA_INVALID_ARGS: return E_BAD_ARGUMENTS;
                case MA_INVALID_OPERATION: return E_NOT_SUPPORTED;
                case MA_OUT_OF_MEMORY: return E_OUT_OF_MEMORY;
                case MA_OUT_OF_RANGE: return E_OUT_OF_RANGE;
                case MA_ACCESS_DENIED: return E_ACCESS_DENIED;
                case MA_DOES_NOT_EXIST: return E_NOT_FOUND;
                case MA_ALREADY_EXISTS: return E_ALREADY_EXISTS;
                case MA_TOO_MANY_OPEN_FILES: return E_OUT_OF_RESOURCE;
                case MA_INVALID_FILE: return E_BAD_FILE;
                case MA_TOO_BIG: return E_DATA_TOO_BIG;
                case MA_PATH_TOO_LONG: return E_DATA_TOO_BIG;
                case MA_NAME_TOO_LONG: return E_DATA_TOO_BIG;
                case MA_NOT_DIRECTORY: return E_NOT_DIRECTORY;
                case MA_IS_DIRECTORY: return E_IS_DIRECTORY;
                case MA_DIRECTORY_NOT_EMPTY: return E_DIRECTORY_NOT_EMPTY;
                case MA_AT_END: return E_END_OF_FILE;
                case MA_NO_SPACE: return E_INSUFFICIENT_SYSTEM_BUFFER;
                case MA_BUSY: return E_NOT_READY;
                case MA_IO_ERROR: return E_IO_ERROR;
                case MA_INTERRUPT: return E_INTERRUPTED;
                case MA_UNAVAILABLE: return E_NOT_SUPPORTED;
                case MA_BAD_ADDRESS: return E_BAD_MEMORY_ADDRESS;
                case MA_DEADLOCK: return E_DEADLOCK;
                case MA_TOO_MANY_LINKS: return E_OUT_OF_RESOURCE;
                case MA_NOT_IMPLEMENTED: return E_NOT_SUPPORTED;
                case MA_NO_DATA_AVAILABLE: return E_NO_DATA;
                case MA_INVALID_DATA: return E_BAD_DATA;
                case MA_TIMEOUT: return E_TIMEOUT;
                case MA_NOT_UNIQUE: return E_NOT_UNIQUE;
                case MA_IN_PROGRESS: return E_NOT_READY;
                case MA_CANCELLED: return E_INTERRUPTED;
                case MA_FORMAT_NOT_SUPPORTED: return AHI::E_FORMAT_NOT_SUPPORTED;
                case MA_DEVICE_TYPE_NOT_SUPPORTED: return AHI::E_DEVICE_TYPE_NOT_SUPPORTED;
                case MA_SHARE_MODE_NOT_SUPPORTED: return AHI::E_SHARE_MODE_NOT_SUPPORTED;
                case MA_NO_BACKEND: return AHI::E_NO_BACKEND;
                case MA_NO_DEVICE: return AHI::E_NO_DEVICE;
                case MA_API_NOT_FOUND: return AHI::E_API_NOT_FOUND;
                case MA_INVALID_DEVICE_CONFIG: return AHI::E_BAD_DEVICE_CONFIG;
                case MA_LOOP: return AHI::E_LOOP;
                case MA_DEVICE_NOT_INITIALIZED: return E_BAD_CALLING_TIME;
                case MA_DEVICE_ALREADY_INITIALIZED: return E_BAD_CALLING_TIME;
                case MA_DEVICE_NOT_STARTED: return AHI::E_DEVICE_NOT_STARTED;
                case MA_DEVICE_NOT_STOPPED: return AHI::E_DEVICE_NOT_STOPPED;
                case MA_FAILED_TO_INIT_BACKEND: return AHI::E_FAILED_TO_INIT_BACKEND;
                case MA_FAILED_TO_OPEN_BACKEND_DEVICE: return AHI::E_FAILED_TO_OPEN_BACKEND_DEVICE;
                case MA_FAILED_TO_START_BACKEND_DEVICE: return AHI::E_FAILED_TO_START_BACKEND_DEVICE;
                case MA_FAILED_TO_STOP_BACKEND_DEVICE: return AHI::E_FAILED_TO_STOP_BACKEND_DEVICE;
                default: return E_FAILURE;
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