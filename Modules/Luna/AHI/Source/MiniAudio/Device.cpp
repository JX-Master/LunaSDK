/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Device.cpp
* @author JXMaster
* @date 2023/10/16
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_AUDIO_API LUNA_EXPORT
#include "Adapter.hpp"
#include "Device.hpp"
#include <Luna/Runtime/Array.hpp>

namespace Luna
{
    namespace AHI
    {
        struct MixBuffer
        {
            void* data;
            u32 num_frames;
        };
        void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
        {
            Device* device = (Device*)pDevice->pUserData;
            if(test_flags(device->m_flags, DeviceFlag::playback))
            {
                MutexGuard guard(device->m_audio_sources_mutex);
                WaveFormat format;
                format.sample_rate = device->get_sample_rate();
                format.num_channels = device->get_playback_num_channels();
                format.bit_depth = device->get_playback_bit_depth();
                usize buffer_size = get_frame_size(format.bit_depth, format.num_channels) * (usize)frameCount;
                Array<MixBuffer> mix_buffers(device->m_audio_sources.size());
                for(usize i = 0; i < device->m_audio_sources.size(); ++i)
                {
                    auto& dst = device->m_audio_sources[i];
                    dst->read_source_data(format, frameCount, buffer_size);
                    mix_buffers[i].data = dst->m_buffer.data();
                    mix_buffers[i].num_frames = dst->m_valid_frames;
                }
                // Mix data.
                
            }
        }
        RV Device::init(const DeviceDesc& desc)
        {
            m_audio_sources_mutex = new_mutex();
            m_flags = desc.flags;
            ma_device_type type;
            if(test_flags(desc.flags, DeviceFlag::playback | DeviceFlag::capture)) type = ma_device_type_duplex;
            else if(test_flags(desc.flags, DeviceFlag::playback)) type = ma_device_type_playback;
            else if(test_flags(desc.flags, DeviceFlag::capture)) type = ma_device_type_capture;
            else return set_error(BasicError::bad_arguments(), "One of DeviceFlag::playback and DeviceFlag::capture must be set when creating audio devices");
            ma_device_config config = ma_device_config_init(type);
            if(test_flags(desc.flags, DeviceFlag::playback))
            {
                if(desc.playback.adapter)
                {
                    Adapter* adapter = cast_object<Adapter>(desc.playback.adapter->get_object());
                    config.playback.pDeviceID = &(adapter->m_info.id);
                }
                config.playback.format = encode_format(desc.playback.bit_depth);
                config.playback.channels = desc.playback.num_channels;
            }
            if(test_flags(desc.flags, DeviceFlag::capture))
            {
                if(desc.capture.adapter)
                {
                    Adapter* adapter = cast_object<Adapter>(desc.capture.adapter->get_object());
                    config.capture.pDeviceID = &(adapter->m_info.id);
                }
                config.capture.format = encode_format(desc.capture.bit_depth);
                config.capture.channels = desc.capture.num_channels;
            }
            config.sampleRate = desc.sample_rate;
            config.dataCallback = data_callback;
            config.pUserData = this;
            auto r = ma_device_init(&g_context, &config, &m_device);
            if(r != MA_SUCCESS)
            {
                return translate_ma_result(r);
            }
            r = ma_device_start(&m_device);
            if(r != MA_SUCCESS)
            {
                return translate_ma_result(r);
            }
            return ok;
        }
        Device::~Device()
        {
            ma_device_uninit(&m_device);
        }
        void Device::add_audio_source(IAudioSource* audio_source)
        {
            MutexGuard guard(m_audio_sources_mutex);
            AudioSource* source = cast_object<AudioSource>(audio_source->get_object());
            m_audio_sources.push_back(Ref<AudioSource>(source));
        }
        void Device::remove_audio_source(IAudioSource* audio_source)
        {
            MutexGuard guard(m_audio_sources_mutex);
            AudioSource* source = cast_object<AudioSource>(audio_source->get_object());
            for(auto iter = m_audio_sources.begin(); iter != m_audio_sources.end(); ++iter)
            {
                if(iter->get() == source)
                {
                    m_audio_sources.erase(iter);
                    break;
                }
            }
        }
        LUNA_AUDIO_API R<Ref<IDevice>> new_device(const DeviceDesc& desc)
        {
            Ref<Device> dev = new_object<Device>();
            lutry
            {
                luexp(dev->init(desc));
            }
            lucatchret;
            return dev;
        }
    }
}