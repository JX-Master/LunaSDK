#pragma once

#include "../Runtime/Runtime.h"

#include <stdint.h>

#if defined(_WIN32)
#define LUNA_IMAGE_C_API __declspec(dllexport)
#else
#define LUNA_IMAGE_C_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LunaImageDesc
{
    uint32_t format;
    uint32_t width;
    uint32_t height;
} LunaImageDesc;

typedef struct LunaImageData
{
    void* data;
    uint64_t data_size;
    LunaImageDesc desc;
} LunaImageData;

LUNA_IMAGE_C_API luna_errcode_t luna_image_init_module(void);
LUNA_IMAGE_C_API luna_errcode_t luna_image_read_file_desc(const void* data, uint64_t data_size, LunaImageDesc* out_desc);
LUNA_IMAGE_C_API luna_errcode_t luna_image_read_file(const void* data, uint64_t data_size, uint32_t desired_format, LunaImageData* out_image);
LUNA_IMAGE_C_API void luna_image_free_data(LunaImageData* image);

#ifdef __cplusplus
}
#endif
