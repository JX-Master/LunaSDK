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
#include <Luna/Runtime/Mutex.hpp>
#include <Luna/Runtime/Event.hpp>

namespace Luna
{
    namespace AHI
    {
        struct AudioSource
        {
            Blob m_buffer;
            u32 m_valid_frames = 0;
            Function<playback_callback_t> m_callback;

            void read_source_data(const WaveFormat& format, u32 num_frames, usize buffer_size)
            {
                m_valid_frames = 0;
                if(!m_callback) return;
                if(m_buffer.size() < buffer_size)
                {
                    m_buffer.resize(buffer_size, false);
                }
                m_valid_frames = m_callback(m_buffer.data(), format, num_frames);
            }
        };

        struct Device : IDevice
        {
            lustruct("AHI::Device", "{86cc0475-a167-4be5-913a-b3fa650061ac}");
            luiimpl();

            Ref<IMutex> m_audio_sources_mutex;
            Ref<IMutex> m_capture_event_mutex;
            ma_device m_device;
            DeviceFlag m_flags;
            Vector<Pair<usize, AudioSource>> m_audio_sources;
            usize m_next_audio_source = 0;
            Event<capture_callback_t> m_capture_event;

            RV init(const DeviceDesc& desc);
            ~Device();

            virtual u32 get_sample_rate() override { return m_device.sampleRate; }
            virtual DeviceFlag get_flags() override { return m_flags; }
            virtual u32 get_playback_num_channels() override { return m_device.playback.channels; }
            virtual BitDepth get_playback_bit_depth() override { return decode_bit_depth(m_device.playback.format); }
            virtual u32 get_capture_num_channels() override { return m_device.capture.channels; }
            virtual BitDepth get_capture_bit_depth() override { return decode_bit_depth(m_device.capture.format); }
            virtual usize add_playback_data_callback(const Function<playback_callback_t>& callback) override
            {
                MutexGuard guard(m_audio_sources_mutex);
                AudioSource source;
                source.m_callback = callback;
                m_audio_sources.push_back(make_pair(m_next_audio_source, move(source)));
                ++m_next_audio_source;
                return m_next_audio_source - 1;
            }
            virtual usize add_playback_data_callback(Function<playback_callback_t>&& callback) override
            {
                MutexGuard guard(m_audio_sources_mutex);
                AudioSource source;
                source.m_callback = move(callback);
                m_audio_sources.push_back(make_pair(m_next_audio_source, move(source)));
                ++m_next_audio_source;
                return m_next_audio_source - 1;
            }
            virtual void remove_playback_data_callback(usize handle) override
            {
                MutexGuard guard(m_audio_sources_mutex);
                for(auto iter = m_audio_sources.begin(); iter != m_audio_sources.end(); ++iter)
                {
                    if(iter->first == handle)
                    {
                        m_audio_sources.erase(iter);
                        return;
                    }
                }
            }
            virtual usize add_capture_data_callback(const Function<capture_callback_t>& callback) override
            { 
                MutexGuard guard(m_capture_event_mutex);
                return m_capture_event.add_handler(callback);
            }
            virtual usize add_capture_data_callback(Function<capture_callback_t>&& callback) override 
            {
                MutexGuard guard(m_capture_event_mutex);
                return m_capture_event.add_handler(move(callback));
            }
            virtual void remove_capture_data_callback(usize handle) override
            {
                MutexGuard guard(m_capture_event_mutex);
                m_capture_event.remove_handler(handle);
            }
        };
    }
}