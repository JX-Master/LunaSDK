#include "Image.h"

#include <Luna/Image/Image.hpp>
#include <Luna/Runtime/Blob.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Memory.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Result.hpp>

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

LunaImageDesc from_image_desc(const Luna::Image::ImageDesc& desc)
{
    return LunaImageDesc
    {
        static_cast<uint32_t>(desc.format),
        desc.width,
        desc.height
    };
}

Luna::Image::ImageDesc to_image_desc(const LunaImageDesc& desc)
{
    return Luna::Image::ImageDesc
    {
        static_cast<Luna::Image::ImageFormat>(desc.format),
        desc.width,
        desc.height
    };
}

Luna::ISeekableStream* object_as_seekable_stream(luna_handle_t object)
{
    return object ? Luna::query_interface<Luna::ISeekableStream>(object) : nullptr;
}

void clear_image_data(LunaImageData& image)
{
    image.data = nullptr;
    image.data_size = 0;
    image.desc = LunaImageDesc{};
}
}

extern "C"
{
LUNA_IMAGE_C_API luna_errcode_t luna_image_init_module(void)
{
    Luna::Module* module = Luna::module_image();
    Luna::RV result = Luna::add_module(module);
    if (!result.valid())
    {
        return from_result(result);
    }
    result = Luna::init_module(module);
    return from_result(result);
}

LUNA_IMAGE_C_API luna_errcode_t luna_image_read_file_desc(const void* data, uint64_t data_size, LunaImageDesc* out_desc)
{
    if ((!data && data_size) || !out_desc)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    auto result = Luna::Image::read_image_file_desc(data, static_cast<Luna::usize>(data_size));
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_desc = from_image_desc(result.get());
    return 0;
}

LUNA_IMAGE_C_API luna_errcode_t luna_image_read_file(const void* data, uint64_t data_size, uint32_t desired_format, LunaImageData* out_image)
{
    if ((!data && data_size) || !out_image)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    clear_image_data(*out_image);

    Luna::Image::ImageDesc desc;
    auto result = Luna::Image::read_image_file(
        data,
        static_cast<Luna::usize>(data_size),
        static_cast<Luna::Image::ImageFormat>(desired_format),
        desc);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::Blob image = Luna::move(result.get());
    out_image->data_size = static_cast<uint64_t>(image.size());
    out_image->data = image.detach();
    out_image->desc = from_image_desc(desc);
    return 0;
}

LUNA_IMAGE_C_API luna_errcode_t luna_image_write_png_file(luna_handle_t stream, const LunaImageDesc* desc, const void* data, uint64_t data_size)
{
    if (!stream || !desc || (!data && data_size))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::ISeekableStream* native_stream = object_as_seekable_stream(stream);
    if (!native_stream)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::Image::write_png_file(native_stream, to_image_desc(*desc), data, static_cast<Luna::usize>(data_size)));
}

LUNA_IMAGE_C_API luna_errcode_t luna_image_write_bmp_file(luna_handle_t stream, const LunaImageDesc* desc, const void* data, uint64_t data_size)
{
    if (!stream || !desc || (!data && data_size))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::ISeekableStream* native_stream = object_as_seekable_stream(stream);
    if (!native_stream)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::Image::write_bmp_file(native_stream, to_image_desc(*desc), data, static_cast<Luna::usize>(data_size)));
}

LUNA_IMAGE_C_API luna_errcode_t luna_image_write_tga_file(luna_handle_t stream, const LunaImageDesc* desc, const void* data, uint64_t data_size)
{
    if (!stream || !desc || (!data && data_size))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::ISeekableStream* native_stream = object_as_seekable_stream(stream);
    if (!native_stream)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::Image::write_tga_file(native_stream, to_image_desc(*desc), data, static_cast<Luna::usize>(data_size)));
}

LUNA_IMAGE_C_API luna_errcode_t luna_image_write_jpg_file(luna_handle_t stream, const LunaImageDesc* desc, const void* data, uint64_t data_size, uint32_t quality)
{
    if (!stream || !desc || (!data && data_size))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::ISeekableStream* native_stream = object_as_seekable_stream(stream);
    if (!native_stream)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::Image::write_jpg_file(native_stream, to_image_desc(*desc), data, static_cast<Luna::usize>(data_size), quality));
}

LUNA_IMAGE_C_API luna_errcode_t luna_image_write_hdr_file(luna_handle_t stream, const LunaImageDesc* desc, const void* data, uint64_t data_size)
{
    if (!stream || !desc || (!data && data_size))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::ISeekableStream* native_stream = object_as_seekable_stream(stream);
    if (!native_stream)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::Image::write_hdr_file(native_stream, to_image_desc(*desc), data, static_cast<Luna::usize>(data_size)));
}

LUNA_IMAGE_C_API void luna_image_free_data(LunaImageData* image)
{
    if (!image)
    {
        return;
    }
    if (image->data)
    {
        Luna::memfree(image->data);
    }
    clear_image_data(*image);
}
}
