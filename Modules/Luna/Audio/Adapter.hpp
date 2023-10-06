/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Adapter.hpp
* @author JXMaster
* @date 2023/10/6
*/
#pragma once
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Runtime/Name.hpp>

#ifndef LUNA_AUDIO_API
#define LUNA_AUDIO_API
#endif

namespace Luna
{
    namespace Audio
    {
        enum class AdapterType : u8
        {
            playback = 0,
            capture = 1,
        };

        enum class BitDepth : u8
        {
            u8 = 0, // 8-bit unsigned integer.
            s16 = 1, // 16-bit signed integer.
            s24 = 2, // 24-bit signed integer.
            s32 = 3, // 32-bit signed integer.
            f32 = 4, // 32-bit normalized floating-point.
        };

        //! Describes format of one sound wave.
        struct WaveFormat
        {
            //! The sample rate of the sound wave.
            u32 sample_rate;
            //! The number of channels of the sound wave.
            u16 num_channels;
            //! The bit depth of the sound wave.
            BitDepth bit_depth;
        };

        struct AdapterDesc
        {
            //! An opaque id that identify the device.
            u64 id;
            //! The name of the adapter.
            Name name;
            //! The type of the adapter.
            AdapterType type;
        };        

        //! Gets a list of adapters (driver-provided audio devices) present on the platform.
        //! Every time this is called, the audio system will fetch the adapters list from the system. So that adapter
        //! changes after the application start will also be detected.
        LUNA_AUDIO_API Vector<AdapterDesc> get_adapters();
    }
}