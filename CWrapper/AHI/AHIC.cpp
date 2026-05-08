#include "AHI.h"

#include <Luna/AHI/AHI.hpp>
#include <Luna/AHI/AHIError.hpp>
#include <Luna/AHI/Adapter.hpp>
#include <Luna/AHI/Device.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Memory.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Object.hpp>
#include <Luna/Runtime/Result.hpp>

#include <cstring>

namespace
{
luna_errcode_t from_errcode(Luna::ErrCode code)
{
    return static_cast<luna_errcode_t>(code.code);
}

luna_errcode_t from_result(const Luna::RV& result)
{
    return from_errcode(result.errcode());
}

template <typename T>
T* object_as(luna_handle_t object)
{
    return object ? Luna::query_interface<T>(object) : nullptr;
}

LunaAhiWaveFormat from_wave_format(const Luna::AHI::WaveFormat& value)
{
    return LunaAhiWaveFormat
    {
        value.sample_rate,
        value.num_channels,
        static_cast<uint8_t>(value.bit_depth)
    };
}

Luna::AHI::WaveFormat to_wave_format(const LunaAhiWaveFormat& value)
{
    Luna::AHI::WaveFormat result;
    result.sample_rate = value.sample_rate;
    result.num_channels = value.num_channels;
    result.bit_depth = static_cast<Luna::AHI::BitDepth>(value.bit_depth);
    return result;
}

LunaAhiAdapterHandle from_adapter(Luna::Ref<Luna::AHI::IAdapter>&& adapter)
{
    LunaAhiAdapterHandle result{};
    luna_handle_t object = adapter.detach();
    auto* iadapter = object_as<Luna::AHI::IAdapter>(object);
    if(!iadapter)
    {
        Luna::object_release(object);
        return result;
    }
    result.object = object;
    result.iadapter = iadapter;
    return result;
}

LunaAhiDeviceHandle from_device(Luna::Ref<Luna::AHI::IDevice>&& device)
{
    LunaAhiDeviceHandle result{};
    luna_handle_t object = device.detach();
    auto* idevice = object_as<Luna::AHI::IDevice>(object);
    if(!idevice)
    {
        Luna::object_release(object);
        return result;
    }
    result.object = object;
    result.idevice = idevice;
    return result;
}

const char* duplicate_string(const char* source)
{
    if(!source)
    {
        return nullptr;
    }
    auto size = std::strlen(source);
    auto* buffer = static_cast<char*>(Luna::memalloc(size + 1));
    if(!buffer)
    {
        return nullptr;
    }
    std::memcpy(buffer, source, size + 1);
    return buffer;
}

Luna::AHI::DeviceDesc to_device_desc(const LunaAhiDeviceDesc& desc)
{
    Luna::AHI::DeviceDesc result;
    result.playback.adapter = reinterpret_cast<Luna::AHI::IAdapter*>(desc.playback.adapter);
    result.playback.num_channels = desc.playback.num_channels;
    result.playback.bit_depth = static_cast<Luna::AHI::BitDepth>(desc.playback.bit_depth);
    result.capture.adapter = reinterpret_cast<Luna::AHI::IAdapter*>(desc.capture.adapter);
    result.capture.num_channels = desc.capture.num_channels;
    result.capture.bit_depth = static_cast<Luna::AHI::BitDepth>(desc.capture.bit_depth);
    result.sample_rate = desc.sample_rate;
    result.flags = static_cast<Luna::AHI::DeviceFlag>(desc.flags);
    return result;
}
}

extern "C"
{
LUNA_AHI_C_API luna_errcode_t luna_ahi_init_module(void)
{
    Luna::Module* module = Luna::module_ahi();
    Luna::RV result = Luna::add_module(module);
    if(!result.valid())
    {
        return from_result(result);
    }
    result = Luna::init_module(module);
    return from_result(result);
}

LUNA_AHI_C_API void luna_ahi_free_string(const char* text)
{
    if(text)
    {
        Luna::memfree(const_cast<char*>(text));
    }
}

LUNA_AHI_C_API luna_errcode_t luna_ahi_get_adapters(
    LunaAhiAdapterHandle* out_playback_adapters,
    uint64_t playback_capacity,
    uint64_t* out_playback_count,
    LunaAhiAdapterHandle* out_capture_adapters,
    uint64_t capture_capacity,
    uint64_t* out_capture_count)
{
    if((!out_playback_adapters && playback_capacity) || (!out_capture_adapters && capture_capacity))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    Luna::Vector<Luna::Ref<Luna::AHI::IAdapter>> playback_adapters;
    Luna::Vector<Luna::Ref<Luna::AHI::IAdapter>> capture_adapters;
    auto result = Luna::AHI::get_adapters(out_playback_count ? &playback_adapters : nullptr, out_capture_count ? &capture_adapters : nullptr);
    if(!result.valid())
    {
        return from_result(result);
    }

    if(out_playback_count)
    {
        *out_playback_count = static_cast<uint64_t>(playback_adapters.size());
    }
    if(out_capture_count)
    {
        *out_capture_count = static_cast<uint64_t>(capture_adapters.size());
    }

    bool insufficient = false;
    if(out_playback_adapters)
    {
        auto count = Luna::min<Luna::usize>(static_cast<Luna::usize>(playback_capacity), playback_adapters.size());
        for(Luna::usize i = 0; i < count; ++i)
        {
            out_playback_adapters[i] = from_adapter(Luna::move(playback_adapters[i]));
        }
        insufficient |= count < playback_adapters.size();
    }
    if(out_capture_adapters)
    {
        auto count = Luna::min<Luna::usize>(static_cast<Luna::usize>(capture_capacity), capture_adapters.size());
        for(Luna::usize i = 0; i < count; ++i)
        {
            out_capture_adapters[i] = from_adapter(Luna::move(capture_adapters[i]));
        }
        insufficient |= count < capture_adapters.size();
    }

    return insufficient ? from_errcode(Luna::BasicError::insufficient_user_buffer()) : 0;
}

LUNA_AHI_C_API luna_errcode_t luna_ahi_iadapter_get_name(void* self, const char** out_name)
{
    if(!self || !out_name)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_name = nullptr;
    *out_name = duplicate_string(reinterpret_cast<Luna::AHI::IAdapter*>(self)->get_name());
    return *out_name ? 0 : from_errcode(Luna::BasicError::out_of_memory());
}

LUNA_AHI_C_API int32_t luna_ahi_iadapter_is_primary(void* self)
{
    return self && reinterpret_cast<Luna::AHI::IAdapter*>(self)->is_primary() ? 1 : 0;
}

LUNA_AHI_C_API luna_errcode_t luna_ahi_iadapter_get_native_wave_formats(void* self, LunaAhiWaveFormat* out_formats, uint64_t capacity, uint64_t* out_count)
{
    if(!self || !out_count || (!out_formats && capacity))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::usize count = static_cast<Luna::usize>(capacity);
    auto result = reinterpret_cast<Luna::AHI::IAdapter*>(self)->get_native_wave_formats(reinterpret_cast<Luna::AHI::WaveFormat*>(out_formats), &count);
    *out_count = static_cast<uint64_t>(count);
    return from_result(result);
}

LUNA_AHI_C_API luna_errcode_t luna_ahi_new_device(const LunaAhiDeviceDesc* desc, LunaAhiDeviceHandle* out_device)
{
    if(!desc || !out_device)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    out_device->object = nullptr;
    out_device->idevice = nullptr;
    auto result = Luna::AHI::new_device(to_device_desc(*desc));
    if(!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_device = from_device(Luna::move(result.get()));
    return out_device->object && out_device->idevice ? 0 : from_errcode(Luna::BasicError::bad_cast());
}

LUNA_AHI_C_API uint32_t luna_ahi_idevice_get_sample_rate(void* self)
{
    return self ? reinterpret_cast<Luna::AHI::IDevice*>(self)->get_sample_rate() : 0;
}

LUNA_AHI_C_API uint32_t luna_ahi_idevice_get_flags(void* self)
{
    return self ? static_cast<uint32_t>(reinterpret_cast<Luna::AHI::IDevice*>(self)->get_flags()) : 0;
}

LUNA_AHI_C_API uint32_t luna_ahi_idevice_get_playback_num_channels(void* self)
{
    return self ? reinterpret_cast<Luna::AHI::IDevice*>(self)->get_playback_num_channels() : 0;
}

LUNA_AHI_C_API uint8_t luna_ahi_idevice_get_playback_bit_depth(void* self)
{
    return self ? static_cast<uint8_t>(reinterpret_cast<Luna::AHI::IDevice*>(self)->get_playback_bit_depth()) : 0;
}

LUNA_AHI_C_API uint32_t luna_ahi_idevice_get_capture_num_channels(void* self)
{
    return self ? reinterpret_cast<Luna::AHI::IDevice*>(self)->get_capture_num_channels() : 0;
}

LUNA_AHI_C_API uint8_t luna_ahi_idevice_get_capture_bit_depth(void* self)
{
    return self ? static_cast<uint8_t>(reinterpret_cast<Luna::AHI::IDevice*>(self)->get_capture_bit_depth()) : 0;
}

LUNA_AHI_C_API luna_errcode_t luna_ahi_idevice_add_playback_data_callback(void* self, LunaAhiPlaybackDataCallback callback, void* userdata, uint64_t* out_handle)
{
    if(!self || !callback || !out_handle)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto handle = reinterpret_cast<Luna::AHI::IDevice*>(self)->add_playback_data_callback(
        [callback, userdata](void* dst_buffer, const Luna::AHI::WaveFormat& format, uint32_t num_frames) -> uint32_t
        {
            return callback(dst_buffer, from_wave_format(format), num_frames, userdata);
        });
    *out_handle = static_cast<uint64_t>(handle);
    return 0;
}

LUNA_AHI_C_API void luna_ahi_idevice_remove_playback_data_callback(void* self, uint64_t handle)
{
    if(!self)
    {
        return;
    }
    reinterpret_cast<Luna::AHI::IDevice*>(self)->remove_playback_data_callback(static_cast<Luna::usize>(handle));
}

LUNA_AHI_C_API luna_errcode_t luna_ahi_idevice_add_capture_data_callback(void* self, LunaAhiCaptureDataCallback callback, void* userdata, uint64_t* out_handle)
{
    if(!self || !callback || !out_handle)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto handle = reinterpret_cast<Luna::AHI::IDevice*>(self)->add_capture_data_callback(
        [callback, userdata](const void* src_buffer, const Luna::AHI::WaveFormat& format, uint32_t num_frames)
        {
            callback(src_buffer, from_wave_format(format), num_frames, userdata);
        });
    *out_handle = static_cast<uint64_t>(handle);
    return 0;
}

LUNA_AHI_C_API void luna_ahi_idevice_remove_capture_data_callback(void* self, uint64_t handle)
{
    if(!self)
    {
        return;
    }
    reinterpret_cast<Luna::AHI::IDevice*>(self)->remove_capture_data_callback(static_cast<Luna::usize>(handle));
}
}
