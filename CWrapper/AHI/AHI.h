#pragma once

#include "../Runtime/Runtime.h"

#include <stdint.h>

#if defined(_WIN32)
#define LUNA_AHI_C_API __declspec(dllexport)
#else
#define LUNA_AHI_C_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LunaAhiWaveFormat
{
    uint32_t sample_rate;
    uint32_t num_channels;
    uint8_t bit_depth;
} LunaAhiWaveFormat;

typedef struct LunaAhiAdapterHandle
{
    luna_handle_t object;
    void* iadapter;
} LunaAhiAdapterHandle;

typedef struct LunaAhiDeviceHandle
{
    luna_handle_t object;
    void* idevice;
} LunaAhiDeviceHandle;

typedef struct LunaAhiDeviceIoDesc
{
    void* adapter;
    uint32_t num_channels;
    uint8_t bit_depth;
} LunaAhiDeviceIoDesc;

typedef struct LunaAhiDeviceDesc
{
    LunaAhiDeviceIoDesc playback;
    LunaAhiDeviceIoDesc capture;
    uint32_t sample_rate;
    uint32_t flags;
} LunaAhiDeviceDesc;

typedef uint32_t (*LunaAhiPlaybackDataCallback)(void* dst_buffer, LunaAhiWaveFormat format, uint32_t num_frames, void* userdata);
typedef void (*LunaAhiCaptureDataCallback)(const void* src_buffer, LunaAhiWaveFormat format, uint32_t num_frames, void* userdata);

LUNA_AHI_C_API luna_errcode_t luna_ahi_init_module(void);
LUNA_AHI_C_API void luna_ahi_free_string(const char* text);

LUNA_AHI_C_API luna_errcode_t luna_ahi_get_adapters(
    LunaAhiAdapterHandle* out_playback_adapters,
    uint64_t playback_capacity,
    uint64_t* out_playback_count,
    LunaAhiAdapterHandle* out_capture_adapters,
    uint64_t capture_capacity,
    uint64_t* out_capture_count);

LUNA_AHI_C_API luna_errcode_t luna_ahi_iadapter_get_name(void* self, const char** out_name);
LUNA_AHI_C_API int32_t luna_ahi_iadapter_is_primary(void* self);
LUNA_AHI_C_API luna_errcode_t luna_ahi_iadapter_get_native_wave_formats(void* self, LunaAhiWaveFormat* out_formats, uint64_t capacity, uint64_t* out_count);

LUNA_AHI_C_API luna_errcode_t luna_ahi_new_device(const LunaAhiDeviceDesc* desc, LunaAhiDeviceHandle* out_device);
LUNA_AHI_C_API uint32_t luna_ahi_idevice_get_sample_rate(void* self);
LUNA_AHI_C_API uint32_t luna_ahi_idevice_get_flags(void* self);
LUNA_AHI_C_API uint32_t luna_ahi_idevice_get_playback_num_channels(void* self);
LUNA_AHI_C_API uint8_t luna_ahi_idevice_get_playback_bit_depth(void* self);
LUNA_AHI_C_API uint32_t luna_ahi_idevice_get_capture_num_channels(void* self);
LUNA_AHI_C_API uint8_t luna_ahi_idevice_get_capture_bit_depth(void* self);
LUNA_AHI_C_API luna_errcode_t luna_ahi_idevice_add_playback_data_callback(void* self, LunaAhiPlaybackDataCallback callback, void* userdata, uint64_t* out_handle);
LUNA_AHI_C_API void luna_ahi_idevice_remove_playback_data_callback(void* self, uint64_t handle);
LUNA_AHI_C_API luna_errcode_t luna_ahi_idevice_add_capture_data_callback(void* self, LunaAhiCaptureDataCallback callback, void* userdata, uint64_t* out_handle);
LUNA_AHI_C_API void luna_ahi_idevice_remove_capture_data_callback(void* self, uint64_t handle);

#ifdef __cplusplus
}
#endif
