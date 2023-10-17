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
#include "../../AudioSource.hpp"
#include <Luna/Runtime/TSAssert.hpp>

namespace Luna
{
    namespace AHI
    {
        struct AudioSource : IAudioSource
        {
            lustruct("AHI::AudioSource", "{e1794262-ebb6-4286-aee7-cb9462f7e997}");
            luiimpl();
            lutsassert_lock();

            Blob m_buffer;
            u32 m_valid_frames = 0;
            Function<on_read_source_data_t> m_callback;

            void read_source_data(const WaveFormat& format, u32 num_frames, usize buffer_size);

            virtual void set_data_callback(const Function<on_read_source_data_t>& callback) override { m_callback = callback; }
            virtual void set_data_callback(Function<on_read_source_data_t>&& callback) override { m_callback = move(callback); }
        };
    }
}