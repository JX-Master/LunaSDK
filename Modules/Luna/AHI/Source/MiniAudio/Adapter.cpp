/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Adapter.cpp
* @author JXMaster
* @date 2023/10/15
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_AHI_API LUNA_EXPORT
#include "Adapter.hpp"

namespace Luna
{
    namespace AHI
    {
        RV Adapter::get_native_wave_formats(WaveFormat* out_formats, usize* num_formats)
        {
            if(!out_formats)
            {
                if(num_formats) *num_formats = m_info.nativeDataFormatCount;
                return ok;
            }
            if(!num_formats) return BasicError::bad_arguments();
            usize num_formats_to_write = min<usize>(*num_formats, m_info.nativeDataFormatCount);
            for(usize i = 0; i < num_formats_to_write; ++i)
            {
                WaveFormat& dst = out_formats[i];
                dst.sample_rate = m_info.nativeDataFormats[i].sampleRate;
                dst.num_channels = m_info.nativeDataFormats[i].channels;
                switch(m_info.nativeDataFormats[i].format)
                {
                    case ma_format_u8: dst.bit_depth = BitDepth::u8; break;
                    case ma_format_s16: dst.bit_depth = BitDepth::s16; break;
                    case ma_format_s24: dst.bit_depth = BitDepth::s24; break;
                    case ma_format_s32: dst.bit_depth = BitDepth::s32; break;
                    case ma_format_f32: dst.bit_depth = BitDepth::f32; break;
                    default: lupanic();
                }
            }
            *num_formats = num_formats_to_write;
            return num_formats_to_write == m_info.nativeDataFormatCount ? ok : BasicError::insufficient_user_buffer();
        }
        LUNA_AHI_API RV get_adapters(Vector<Ref<IAdapter>>* playback_adapters, Vector<Ref<IAdapter>>* capture_adapters)
        {
            ma_device_info* pPlaybackInfos;
            ma_uint32 playbackCount;
            ma_device_info* pCaptureInfos;
            ma_uint32 captureCount;
            auto r = ma_context_get_devices(&g_context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount);
            if(r != MA_SUCCESS)
            {
                return translate_ma_result(r);
            }
            if(playback_adapters)
            {
                for(ma_uint32 i = 0; i < playbackCount; ++i)
                {
                    Ref<Adapter> adapter = new_object<Adapter>();
                    memcpy(&(adapter->m_info), pPlaybackInfos + i, sizeof(ma_device_info));
                    playback_adapters->push_back(move(Ref<IAdapter>(adapter)));
                }
            }
            if(capture_adapters)
            {
                for(ma_uint32 i = 0; i < captureCount; ++i)
                {
                    Ref<Adapter> adapter = new_object<Adapter>();
                    memcpy(&(adapter->m_info), pCaptureInfos + i, sizeof(ma_device_info));
                    capture_adapters->push_back(move(Ref<IAdapter>(adapter)));
                }
            }
            return ok;
        }
    }
}