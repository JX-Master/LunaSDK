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
#include "AudioSource.hpp"
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Result.hpp>

#ifndef LUNA_AUDIO_API
#define LUNA_AUDIO_API
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

        using on_process_capture_data = void(const void* src_buffer, const WaveFormat& format, u32 num_frames);

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
            virtual void add_audio_source(IAudioSource* audio_source) = 0;
            virtual void remove_audio_source(IAudioSource* audio_source) = 0;
            virtual usize add_process_capture_data_callback(const Function<on_process_capture_data>& callback) = 0;
            virtual usize add_process_capture_data_callback(Function<on_process_capture_data>&& callback) = 0;
            virtual void remove_process_capture_data_callback(usize handle) = 0;
        };

        LUNA_AUDIO_API R<Ref<IDevice>> new_device(const DeviceDesc& desc);
    }
}