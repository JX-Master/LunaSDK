/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AudioSource.cpp
* @author JXMaster
* @date 2023/10/17
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_AUDIO_API LUNA_EXPORT
#include "AudioSource.hpp"

namespace Luna
{
    namespace AHI
    {
        void AudioSource::read_source_data(const WaveFormat& format, u32 num_frames, usize buffer_size)
        {
            m_valid_frames = 0;
            if(!m_callback) return;
            if(m_buffer.size() < buffer_size)
            {
                m_buffer.resize(buffer_size, false);
            }
            m_valid_frames = m_callback(m_buffer.data(), format, num_frames);
        }
        LUNA_AUDIO_API Ref<IAudioSource> new_audio_source()
        {
            Ref<AudioSource> r = new_object<AudioSource>();
            return Ref<IAudioSource>(r);
        }
    }
}