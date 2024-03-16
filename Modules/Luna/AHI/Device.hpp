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
#include "Adapter.hpp"
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Result.hpp>

#ifndef LUNA_AHI_API
#define LUNA_AHI_API
#endif

namespace Luna
{
    namespace AHI
    {
        //! @addtogroup AHI
        //! @{
        
        //! Additional flags specified when creating one device.
        enum class DeviceFlag : u32
        {
            none = 0,
            //! Enable audio playback on this device.
            playback = 1,
            //! Enable audio capture on this device.
            capture = 2,
        };

        //! Describes properties of the playback or capture audio data stream of one device.
        struct DeviceIODesc
        {
            //! The adapter bound to this stream.
            //! This adapter will be used to playback audio data from this stream, or capture
            //! audio data and write to this stream.
            //! If this is `nullptr`, the primary playback or capture device will be used.
            IAdapter* adapter = nullptr;
            //! The number of channels for one audio frame.
            u32 num_channels;
            //! The bit depth of one audio sample in this stream.
            BitDepth bit_depth;
        };

        //! Describes one audio device.
        struct DeviceDesc
        {
            //! Describes the playback stream properties.
            //! This will be ignored if audio playback is disabled on this device 
            //! ( @ref DeviceFlag::playback is not set in `flags`).
            DeviceIODesc playback;
            //! Describes the capture stream properties.
            //! This will be ignored if audio capture is disabled on this device 
            //! ( @ref DeviceFlag::capture is not set in `flags`).
            DeviceIODesc capture;
            //! The sample rate of the playback and capture stream.
            u32 sample_rate;
            //! Additional device flags, like whether to enable playback/capture stream
            DeviceFlag flags;
        };

        //! Called when audio data is required by the audio driver. The user should write audio frames to 
        //! the provided audio buffer for playback.
        //! @details This callback function is invoked in a dedicated audio thread, so the user must use 
        //! synchronization mechanisms if needed. If this function takes too much time to return, the audio 
        //! playback may be paused since the audio driver could not take enough audio frames for playback.
        //! @param[in] dst_buffer The buffer to write audio frames to. The write data must not exceed 
        //! `num_frames * get_frame_size(format.bit_depth, format.num_channels)` bytes.
        //! @param[in] format The required wave format.
        //! @param[in] num_frames The required number of frames to write. If the actual number of frames
        //! written is smaller than required, frames that is not written will be muted (filled with 0).
        //! @return Returns the number of frames actually written.
        using playback_callback_t = u32(void* dst_buffer, const WaveFormat& format, u32 num_frames);

        //! Called when audio data is captured by the audio driver. The user should process such audio data 
        //! (like coping them to application memory) if needed. 
        //! @details This callback function is invoked in a dedicated audio thread, so the user must use 
        //! synchronization mechanisms if needed. If this function takes too much time to return, some captured
        //! audio frames may be lost if the internal driver buffer is full.
        //! @param[in] src_buffer The buffer to read audio frames from. The data read operation must not exceed 
        //! `num_frames * get_frame_size(format.bit_depth, format.num_channels)` bytes.
        //! @param[in] format The wave format of the audio data in `src_buffer`.
        //! @param[in] num_frames The number of frames in `src_buffer`.
        using capture_callback_t = void(const void* src_buffer, const WaveFormat& format, u32 num_frames);

        //! @interface IDevice
        //! Represents one audio device that can playback sounds.
        struct IDevice : virtual Interface
        {
            luiid("{85271f74-2990-41e5-81f1-7e74b128b1d3}");

            //! Gets the sample rate of the playback and capture stream.
            //! @return Returns the sample rate of the playback and capture stream.
            virtual u32 get_sample_rate() = 0;
            //! Gets device flags.
            //! @return Returns the device flags.
            virtual DeviceFlag get_flags() = 0;
            //! Gets the number of channels for the playback stream, which is also the number of audio
            //! samples in one audio frame.
            //! @return Returns the number of channels for the playback stream
            virtual u32 get_playback_num_channels() = 0;
            //! Gets the bit depth of one sample in playback stream.
            //! @return Returns the bit depth of one sample in playback stream.
            virtual BitDepth get_playback_bit_depth() = 0;
            //! Gets the number of channels for the capture stream, which is also the number of audio
            //! samples in one audio frame.
            //! @return Returns the number of channels for the capture stream
            virtual u32 get_capture_num_channels() = 0;
            //! Gets the bit depth of one sample in capture stream.
            //! @return Returns the bit depth of one sample in capture stream.
            virtual BitDepth get_capture_bit_depth() = 0;
            //! Adds a callback that will be called when audio data is required by the audio driver for playback.
            //! @remark See @ref playback_callback_t for details.
            //! @param[in] callback The callback to add.
            //! @return Returns one handle that can be used to remove the added callback.
            virtual usize add_playback_data_callback(const Function<playback_callback_t>& callback) = 0;
            //! Adds a callback that will be called when audio data is required by the audio driver for playback.
            //! @remark See @ref playback_callback_t for details.
            //! @param[in] callback The callback to add.
            //! @return Returns one handle that can be used to remove the added callback.
            virtual usize add_playback_data_callback(Function<playback_callback_t>&& callback) = 0;
            //! Removes one callback added by @ref add_playback_data_callback.
            //! @param[in] handle The handle of the callback to remove. 
            //! This handle is returned by @ref add_playback_data_callback when adding the callback.
            virtual void remove_playback_data_callback(usize handle) = 0;
            //! Adds a callback that will be called when audio data is captured by the audio driver.
            //! @remark See @ref capture_callback_t for details.
            //! @param[in] callback The callback to add.
            //! @return Returns one handle that can be used to remove the added callback.
            virtual usize add_capture_data_callback(const Function<capture_callback_t>& callback) = 0;
            //! Adds a callback that will be called when audio data is captured by the audio driver.
            //! @remark See @ref capture_callback_t for details.
            //! @param[in] callback The callback to add.
            //! @return Returns one handle that can be used to remove the added callback.
            virtual usize add_capture_data_callback(Function<capture_callback_t>&& callback) = 0;
            //! Removes one callback added by @ref add_capture_data_callback.
            //! @param[in] handle The handle of the callback to remove. 
            //! This handle is returned by @ref add_capture_data_callback when adding the callback.
            virtual void remove_capture_data_callback(usize handle) = 0;
        };

        //! Creates one new audio device.
        //! @param[in] desc The 
        LUNA_AHI_API R<Ref<IDevice>> new_device(const DeviceDesc& desc);

        //! @}
    }
}