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
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Ref.hpp>

#ifndef LUNA_AHI_API
#define LUNA_AHI_API
#endif

namespace Luna
{
    namespace AHI
    {
        enum class BitDepth : u8
        {
            unspecified = 0,
            u8 = 1, // 8-bit unsigned integer.
            s16 = 2, // 16-bit signed integer.
            s24 = 3, // 24-bit signed integer.
            s32 = 4, // 32-bit signed integer.
            f32 = 5, // 32-bit normalized floating-point.
        };

        //! Describes format of one sound wave.
        struct WaveFormat
        {
            //! The sample rate of the sound wave.
            u32 sample_rate;
            //! The number of channels of the sound wave.
            u32 num_channels;
            //! The bit depth of the sound wave.
            BitDepth bit_depth;
        };

        inline constexpr usize get_frame_size(BitDepth bit_depth, u32 num_channels)
        {
            usize ret = 0;
            switch(bit_depth)
            {
                case BitDepth::u8: ret = 1; break;
                case BitDepth::s16: ret = 2; break;
                case BitDepth::s24: ret = 3; break;
                case BitDepth::s32:
                case BitDepth::f32: ret = 4; break;
                default: ret = 0; break;
            }
            return ret * (usize)num_channels;
        }
        
        //! Represents one audio adapter that can be used to create one device.
        struct IAdapter : virtual Interface
        {
            luiid("{e19367b1-0f70-4839-8b66-c3a0411d9c29}");

            //! Gets the name of this adapter.
            virtual const c8* get_name() = 0;

            //! Checks if this adapter is the primary adapter.
            virtual bool is_primary() = 0;

            //! Gets a list of native wave formats supported by this adapter.
            //! @param[out] out_formats A pointer to the buffer that receives native data formats, or `nullptr` to check the 
            //! number of native wave formats.
            //! @param[in,out] num_formats If @ref out_formats is not `nullptr`, specify the maximum number of @ref WaveFormat instances
            //! that can be written to the buffer. The system updates this value to the number of @ref WaveFormat instances actually written.
            //! If @ref out_formats is `nullptr`, the system updates this value to the number of native formats supported by the adapter, so 
            //! that the user can allocate a buffer for retrieving formats.
            virtual RV get_native_wave_formats(WaveFormat* out_formats, usize* num_formats) = 0;
        };

        //! Gets a list of adapters (driver-provided audio devices) present on the platform.
        //! @param[out] playback_adapters If not `nullptr`, returns all playback adapaters on the platform.
        //! @param[out] capture_adapters If not `nullptr`, returns all capture adapaters on the platform.
        LUNA_AHI_API RV get_adapters(Vector<Ref<IAdapter>>* playback_adapters, Vector<Ref<IAdapter>>* capture_adapters);
    }
}