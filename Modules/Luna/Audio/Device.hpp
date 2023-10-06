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
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Result.hpp>

#ifndef LUNA_AUDIO_API
#define LUNA_AUDIO_API
#endif

namespace Luna
{
    namespace Audio
    {
        enum class BitDepth : u8
        {
            u8 = 0, // 8-bit unsigned integer.
            s16 = 1, // 16-bit signed integer.
            s24 = 2, // 24-bit signed integer.
            s32 = 3, // 32-bit signed integer.
            f32 = 4, // 32-bit normalized floating-point.
        };

        //! Describes properties of one sound wave.
        struct WaveProperty
        {
            //! The sample rate of the sound wave.
            u32 sample_rate;
            //! The number of channels of the sound wave.
            u16 num_channels;
            //! The bit depth of the sound wave.
            BitDepth bit_depth;
        };

        //! Represents one audio device that can playback sounds.
        struct IDevice : virtual Interface
        {
            luiid("{85271f74-2990-41e5-81f1-7e74b128b1d3}");


        };

        LUNA_AUDIO_API R<Ref<IDevice>> new_device(u64 adapter_id, const WaveProperty& wave_property);
    }
}