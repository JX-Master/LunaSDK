/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Device.hpp
* @author JXMaster
* @date 2023/10/16
*/
#pragma once
#include "../../Device.hpp"
#include "Common.hpp"
#include "AudioSource.hpp"
#include <Luna/Runtime/Mutex.hpp>

namespace Luna
{
    namespace AHI
    {
        struct Device : IDevice
        {
            lustruct("AHI::Device", "{86cc0475-a167-4be5-913a-b3fa650061ac}");
            luiimpl();

            Ref<IMutex> m_audio_sources_mutex;
            ma_device m_device;
            DeviceFlag m_flags;
            Vector<Ref<AudioSource>> m_audio_sources;

            RV init(const DeviceDesc& desc);
            ~Device();

            virtual u32 get_sample_rate() override { return m_device.sampleRate; }
            virtual DeviceFlag get_flags() override { return m_flags; }
            virtual u32 get_playback_num_channels() override { return m_device.playback.channels; }
            virtual BitDepth get_playback_bit_depth() override { return decode_bit_depth(m_device.playback.format); }
            virtual u32 get_capture_num_channels() override { return m_device.capture.channels; }
            virtual BitDepth get_capture_bit_depth() override { return decode_bit_depth(m_device.capture.format); }
            virtual void add_audio_source(IAudioSource* audio_source) override;
            virtual void remove_audio_source(IAudioSource* audio_source) override;
        };
    }
}