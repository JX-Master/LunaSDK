/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AudioSource.hpp
* @author JXMaster
* @date 2023/10/17
*/
#pragma once
#include <Luna/Runtime/Functional.hpp>
#include "Adapter.hpp"

#ifndef LUNA_AUDIO_API
#define LUNA_AUDIO_API
#endif

namespace Luna
{
    namespace AHI
    {
        //! @param[in] dst_buffer The buffer to write audio to. The write data must not exceed 
        //! `num_frames * get_frame_size(format.bit_depth, format.num_channels)` bytes.
        //! @param[in] format The required wave format.
        //! @param[in] num_frames The required number of frames to write. If the actual number of frames
        //! written is smaller than required, frames that is not written will be muted (filled with 0).
        //! @return Returns the number of frames actually written.
        using on_read_source_data_t = u32(void* dst_buffer, const WaveFormat& format, u32 num_frames);

        struct IAudioSource : virtual Interface
        {
            luiid("{0feac42f-b17d-48c5-b9f7-ca051483304d}");

            virtual void set_data_callback(const Function<on_read_source_data_t>& callback) = 0;
            virtual void set_data_callback(Function<on_read_source_data_t>&& callback) = 0;
        };

        LUNA_AUDIO_API Ref<IAudioSource> new_audio_source();
    }
}