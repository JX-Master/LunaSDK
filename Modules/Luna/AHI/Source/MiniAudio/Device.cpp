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
#define LUNA_AHI_API LUNA_EXPORT
#include "Adapter.hpp"
#include "Device.hpp"
#include <Luna/Runtime/Array.hpp>
#include <Luna/Runtime/Math/Math.hpp>

namespace Luna
{
    namespace AHI
    {
        struct MixBuffer
        {
            void* data;
            u32 num_frames;
        };
        static void mix_u8(u8* dst, u32 num_channels, u32 num_frames, Span<MixBuffer> src_buffers)
        {
            for(u32 f = 0; f < num_frames; ++f)
            {
                for(u32 c = 0; c < num_channels; ++c)
                {
                    u32 sample = 0;
                    for(auto& src : src_buffers)
                    {
                        if(f < src.num_frames)
                        {
                            u8* src_data = (u8*)src.data;
                            sample += (u32)(*src_data);
                            src.data = src_data + 1;
                        }
                    }
                    sample = min<u32>(sample, 255);
                    *dst = (u8)sample;
                    ++dst;
                }
            }
        }
        static void mix_s16(i16* dst, u32 num_channels, u32 num_frames, Span<MixBuffer> src_buffers)
        {
            for(u32 f = 0; f < num_frames; ++f)
            {
                for(u32 c = 0; c < num_channels; ++c)
                {
                    i32 sample = 0;
                    for(auto& src : src_buffers)
                    {
                        if(f < src.num_frames)
                        {
                            i16* src_data = (i16*)src.data;
                            sample += (i32)(*src_data);
                            src.data = src_data + 1;
                        }
                    }
                    sample = clamp(sample, -32768, 32767);
                    *dst = (i16)sample;
                    ++dst;
                }
            }
        }
        static void mix_s24(u8* dst, u32 num_channels, u32 num_frames, Span<MixBuffer> src_buffers)
        {
            for(u32 f = 0; f < num_frames; ++f)
            {
                for(u32 c = 0; c < num_channels; ++c)
                {
                    i32 sample = 0;
                    for(auto& src : src_buffers)
                    {
                        if(f < src.num_frames)
                        {
                            i32 data = 0;
                            // read 3 bytes and compose s24.
                            u8* src_data_ptr = (u8*)src.data;
#ifdef LUNA_PLATFORM_LITTLE_ENDIAN
                            data = ((i32)(src_data_ptr[0])) + (((i32)src_data_ptr[1]) << 8) + (((i32)(src_data_ptr[2] & 0x7F)) << 16);
                            data = (src_data_ptr[2] & 0x80) ? -data : data;
#else
                            data = ((i32)(src_data_ptr[2])) + ((i32)src_data_ptr[1] << 8) + (((i32)(src_data_ptr[0] & 0x7F)) << 16);
                            data = (src_data_ptr[0] & 0x80) ? -data : data;
#endif
                            sample += data;
                            src.data = src_data_ptr + 3;
                        }
                    }
                    sample = clamp(sample, -8388608, 8388607);
#ifdef LUNA_PLATFORM_LITTLE_ENDIAN
                    dst[0] = (u8)(sample);
                    dst[1] = (u8)(sample >> 8);
                    dst[2] = (u8)((sample >> 16) & 0x7F) + (u8)(sample < 0 ? 0x80 : 0);
#else
                    dst[0] = (u8)((sample >> 16) & 0x7F) + (u8)(sample < 0 ? 0x80 : 0);
                    dst[1] = (u8)(sample >> 8);
                    dst[2] = (u8)(sample);
#endif
                    dst += 3;
                }
            }
        }
        static void mix_s32(i32* dst, u32 num_channels, u32 num_frames, Span<MixBuffer> src_buffers)
        {
            for(u32 f = 0; f < num_frames; ++f)
            {
                for(u32 c = 0; c < num_channels; ++c)
                {
                    i64 sample = 0;
                    for(auto& src : src_buffers)
                    {
                        if(f < src.num_frames)
                        {
                            i32* src_data = (i32*)src.data;
                            sample += (i64)(*src_data);
                            src.data = src_data + 1;
                        }
                    }
                    sample = clamp(sample, -2147483648, 2147483647);
                    *dst = (i32)sample;
                    ++dst;
                }
            }
        }
        static void mix_f32(f32* dst, u32 num_channels, u32 num_frames, Span<MixBuffer> src_buffers)
        {
            for(u32 f = 0; f < num_frames; ++f)
            {
                for(u32 c = 0; c < num_channels; ++c)
                {
                    f32 sample = 0;
                    for(auto& src : src_buffers)
                    {
                        if(f < src.num_frames)
                        {
                            f32* src_data = (f32*)src.data;
                            sample += (f32)(*src_data);
                            src.data = src_data + 1;
                        }
                    }
                    sample = clamp(sample, -1.0, 1.0);
                    *dst = (f32)sample;
                    ++dst;
                }
            }
        }
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
                usize num_mix_buffers = device->m_audio_sources.size();
                Array<MixBuffer> mix_buffer_array;
                MixBuffer* mix_buffers;
                if(num_mix_buffers > 32)
                {
                    mix_buffer_array = Array<MixBuffer>(num_mix_buffers);
                    mix_buffers = mix_buffer_array.data();
                }
                else
                {
                    mix_buffers = (MixBuffer*)alloca(sizeof(MixBuffer) * num_mix_buffers);
                }
                for(usize i = 0; i < device->m_audio_sources.size(); ++i)
                {
                    auto& dst = device->m_audio_sources[i];
                    dst.second.read_source_data(format, frameCount, buffer_size);
                    mix_buffers[i].data = dst.second.m_buffer.data();
                    mix_buffers[i].num_frames = dst.second.m_valid_frames;
                }
                // Mix data.
                switch(format.bit_depth)
                {
                case BitDepth::u8:
                    mix_u8((u8*)pOutput, format.num_channels, frameCount, {mix_buffers, num_mix_buffers});
                    break;
                case BitDepth::s16:
                    mix_s16((i16*)pOutput, format.num_channels, frameCount, {mix_buffers, num_mix_buffers});
                    break;
                case BitDepth::s24:
                    mix_s24((u8*)pOutput, format.num_channels, frameCount, {mix_buffers, num_mix_buffers});
                    break;
                case BitDepth::s32:
                    mix_s32((i32*)pOutput, format.num_channels, frameCount, {mix_buffers, num_mix_buffers});
                    break;
                case BitDepth::f32:
                    mix_f32((f32*)pOutput, format.num_channels, frameCount, {mix_buffers, num_mix_buffers});
                    break;
                default: lupanic();
                }
            }
            if(test_flags(device->m_flags, DeviceFlag::capture))
            {
                MutexGuard guard(device->m_capture_event_mutex);
                WaveFormat format;
                format.sample_rate = device->get_sample_rate();
                format.num_channels = device->get_capture_num_channels();
                format.bit_depth = device->get_capture_bit_depth();
                device->m_capture_event(pInput, format, frameCount);
            }
        }
        RV Device::init(const DeviceDesc& desc)
        {
            m_audio_sources_mutex = new_mutex();
            m_capture_event_mutex = new_mutex();
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
        LUNA_AHI_API R<Ref<IDevice>> new_device(const DeviceDesc& desc)
        {
            Ref<Device> dev = new_object<Device>();
            lutry
            {
                luexp(dev->init(desc));
            }
            lucatchret;
            return Ref<IDevice>(dev);
        }
    }
}