/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Device.hpp
* @author JXMaster
* @date 2023/10/6
*/
#pragma once
#include "Adapter.hpp"
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Result.hpp>

#ifndef LUNA_AHI_API
#define LUNA_AHI_API
#endif

namespace Luna
{
    namespace AHI
    {
        enum class DeviceFlag : u32
        {
            none = 0,
            //! This device supports audio playback.
            playback = 1,
            //! This device supports audio capture.
            capture = 2,
        };

        struct DeviceIODesc
        {
            IAdapter* adapter = nullptr;
            u32 num_channels;
            BitDepth bit_depth;
        };

        struct DeviceDesc
        {
            DeviceIODesc playback;
            DeviceIODesc capture;
            u32 sample_rate;
            DeviceFlag flags;
        };

        //! @param[in] dst_buffer The buffer to write audio to. The write data must not exceed 
        //! `num_frames * get_frame_size(format.bit_depth, format.num_channels)` bytes.
        //! @param[in] format The required wave format.
        //! @param[in] num_frames The required number of frames to write. If the actual number of frames
        //! written is smaller than required, frames that is not written will be muted (filled with 0).
        //! @return Returns the number of frames actually written.
        using playback_callback_t = u32(void* dst_buffer, const WaveFormat& format, u32 num_frames);

        using capture_callback_t = void(const void* src_buffer, const WaveFormat& format, u32 num_frames);

        //! Represents one audio device that can playback sounds.
        struct IDevice : virtual Interface
        {
            luiid("{85271f74-2990-41e5-81f1-7e74b128b1d3}");

            virtual u32 get_sample_rate() = 0;
            virtual DeviceFlag get_flags() = 0;
            virtual u32 get_playback_num_channels() = 0;
            virtual BitDepth get_playback_bit_depth() = 0;
            virtual u32 get_capture_num_channels() = 0;
            virtual BitDepth get_capture_bit_depth() = 0;
            virtual usize add_playback_data_callback(const Function<playback_callback_t>& callback) = 0;
            virtual usize add_playback_data_callback(Function<playback_callback_t>&& callback) = 0;
            virtual void remove_playback_data_callback(usize handle) = 0;
            virtual usize add_capture_data_callback(const Function<capture_callback_t>& callback) = 0;
            virtual usize add_capture_data_callback(Function<capture_callback_t>&& callback) = 0;
            virtual void remove_capture_data_callback(usize handle) = 0;
        };

        LUNA_AHI_API R<Ref<IDevice>> new_device(const DeviceDesc& desc);
    }
}