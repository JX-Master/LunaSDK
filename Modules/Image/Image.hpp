/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Image.hpp
* @author JXMaster
* @date 2020/3/6
* @brief Image file format I/O library.
*/
#pragma once
#include <Runtime/Blob.hpp>
#include <Runtime/Result.hpp>
#include <Runtime/Stream.hpp>

#ifndef LUNA_IMAGE_API
#define LUNA_IMAGE_API
#endif

namespace Luna
{
	namespace Image
	{
		enum class ImagePixelFormat : u8
		{
			// Supported by all formats except .hdr
			r8_unorm,
			rg8_unorm,
			rgb8_unorm,
			rgba8_unorm,

			// Supported only by .png
			r16_unorm,
			rg16_unorm,
			rgb16_unorm,
			rgba16_unorm,

			// Supported only by .hdr
			r32_float,
			rg32_float,
			rgb32_float,
			rgba32_float,
		};

		inline constexpr usize pixel_size(ImagePixelFormat format)
		{
			switch (format)
			{
			case ImagePixelFormat::r8_unorm:
				return 1;
			case ImagePixelFormat::rg8_unorm:
			case ImagePixelFormat::r16_unorm:
				return 2;
			case ImagePixelFormat::rgb8_unorm:
				return 3;
			case ImagePixelFormat::rgba8_unorm:
			case ImagePixelFormat::rg16_unorm:
			case ImagePixelFormat::r32_float:
				return 4;
			case ImagePixelFormat::rgb16_unorm:
				return 6;
			case ImagePixelFormat::rgba16_unorm:
			case ImagePixelFormat::rg32_float:
				return 8;
			case ImagePixelFormat::rgb32_float:
				return 12;
			case ImagePixelFormat::rgba32_float:
				return 16;
			default:
				// Should not get here.
				return 0;
			}
		}

		struct ImageDesc
		{
			ImagePixelFormat format;
			u32 width;
			u32 height;
		};

		LUNA_IMAGE_API R<ImageDesc> read_image_file_desc(const void* data, usize data_size);
		LUNA_IMAGE_API R<Blob> read_image_file(const void* data, usize data_size, ImagePixelFormat desired_format, ImageDesc& out_desc);

		LUNA_IMAGE_API RV write_png_file(ISeekableStream* stream, const ImageDesc& desc, const Blob& image_data);
		LUNA_IMAGE_API RV write_bmp_file(ISeekableStream* stream, const ImageDesc& desc, const Blob& image_data);
		LUNA_IMAGE_API RV write_tga_file(ISeekableStream* stream, const ImageDesc& desc, const Blob& image_data);
		LUNA_IMAGE_API RV write_jpg_file(ISeekableStream* stream, const ImageDesc& desc, const Blob& image_data, u32 quality);
		LUNA_IMAGE_API RV write_hdr_file(ISeekableStream* stream, const ImageDesc& desc, const Blob& image_data);
	}

	namespace ImageError
	{
		LUNA_IMAGE_API errcat_t errtype();
		LUNA_IMAGE_API ErrCode file_parse_error();
	}
}