/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Image.cpp
* @author JXMaster
* @date 2020/3/6
*/
#include "Image.hpp"
#include "IO/STBImage.hpp"
#include "IO/STBImageWrite.hpp"
#include <Runtime/Module.hpp>

namespace Luna
{
	namespace Image
	{
		inline int stbiw_get_comp(ImagePixelFormat format)
		{
			switch (format)
			{
			case ImagePixelFormat::r8_unorm:
			case ImagePixelFormat::r16_unorm:
			case ImagePixelFormat::r32_float:
				return 1;
			case ImagePixelFormat::rg8_unorm:
			case ImagePixelFormat::rg16_unorm:
			case ImagePixelFormat::rg32_float:
				return 2;
			case ImagePixelFormat::rgb8_unorm:
			case ImagePixelFormat::rgb16_unorm:
			case ImagePixelFormat::rgb32_float:
				return 3;
			case ImagePixelFormat::rgba8_unorm:
			case ImagePixelFormat::rgba16_unorm:
			case ImagePixelFormat::rgba32_float:
				return 4;
			default:
				lupanic();
				return 0;
			}
		}
		static ImageDesc stbi_make_desc(int w, int h, int comp, int is_hdr, int is_16bit)
		{
			luassert(comp >= 1 && comp <= 4);
			ImageDesc d;
			d.width = (u32)w;
			d.height = (u32)h;
			if (is_hdr)
			{
				switch (comp)
				{
				case 1: d.format = ImagePixelFormat::r32_float; break;
				case 2: d.format = ImagePixelFormat::rg32_float; break;
				case 3: d.format = ImagePixelFormat::rgb32_float; break;
				case 4: d.format = ImagePixelFormat::rgba32_float; break;
				default: lupanic(); break;
				}
			}
			else if (is_16bit)
			{
				switch (comp)
				{
				case 1: d.format = ImagePixelFormat::r16_unorm; break;
				case 2: d.format = ImagePixelFormat::rg16_unorm; break;
				case 3: d.format = ImagePixelFormat::rgb16_unorm; break;
				case 4: d.format = ImagePixelFormat::rgba16_unorm; break;
				default: lupanic(); break;
				}
			}
			else
			{
				switch (comp)
				{
				case 1: d.format = ImagePixelFormat::r8_unorm; break;
				case 2: d.format = ImagePixelFormat::rg8_unorm; break;
				case 3: d.format = ImagePixelFormat::rgb8_unorm; break;
				case 4: d.format = ImagePixelFormat::rgba8_unorm; break;
				default: lupanic(); break;
				}
			}
			return d;
		}
		LUNA_IMAGE_API R<ImageDesc> read_image_file_desc(const void* data, usize data_size)
		{
			int x, y, comp;
			int is_16_bit, is_hdr;
			if (!stbi_info_from_memory((const unsigned char*)data, (int)data_size, &x, &y, &comp))
			{
				// data corrupted.
				return ImageError::file_parse_error();
			}
			is_16_bit = stbi_is_16_bit_from_memory((const unsigned char*)data, (int)data_size);
			is_hdr = stbi_is_hdr_from_memory((const unsigned char*)data, (int)data_size);
			return stbi_make_desc(x, y, comp, is_hdr, is_16_bit);
		}
		inline bool is_hdr(ImagePixelFormat format)
		{
			switch (format)
			{
			case ImagePixelFormat::r32_float:
			case ImagePixelFormat::rg32_float:
			case ImagePixelFormat::rgb32_float:
			case ImagePixelFormat::rgba32_float:
				return true;
			default:
				return false;
			}
		}
		inline bool is_16_bit(ImagePixelFormat format)
		{
			switch (format)
			{
			case ImagePixelFormat::r16_unorm:
			case ImagePixelFormat::rg16_unorm:
			case ImagePixelFormat::rgb16_unorm:
			case ImagePixelFormat::rgba16_unorm:
				return true;
			default:
				return false;
			}
		}
		inline int get_comp(ImagePixelFormat format)
		{
			switch (format)
			{
			case ImagePixelFormat::r8_unorm:
			case ImagePixelFormat::r16_unorm:
			case ImagePixelFormat::r32_float:
				return 1;
			case ImagePixelFormat::rg8_unorm:
			case ImagePixelFormat::rg16_unorm:
			case ImagePixelFormat::rg32_float:
				return 2;
			case ImagePixelFormat::rgb8_unorm:
			case ImagePixelFormat::rgb16_unorm:
			case ImagePixelFormat::rgb32_float:
				return 3;
			case ImagePixelFormat::rgba8_unorm:
			case ImagePixelFormat::rgba16_unorm:
			case ImagePixelFormat::rgba32_float:
				return 4;
			default:
				lupanic();
				return 0;
			}
		}
		LUNA_IMAGE_API R<Blob> read_image_file(const void* data, usize data_size, ImagePixelFormat desired_format, ImageDesc& out_desc)
		{
			// Use stb_image library to load image.

			// Prefetch the data info.
			Blob ret;
			void* read_data;
			int out_x, out_y, out_comp;
			if (is_hdr(desired_format))
			{
				read_data = stbi_loadf_from_memory((const unsigned char*)data, (int)data_size, &out_x, &out_y, &out_comp, get_comp(desired_format));
			}
			else if (is_16_bit(desired_format))
			{
				read_data = stbi_load_16_from_memory((const unsigned char*)data, (int)data_size, &out_x, &out_y, &out_comp, get_comp(desired_format));
			}
			else
			{
				read_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &out_x, &out_y, &out_comp, get_comp(desired_format));
			}
			if (!read_data)
			{
				return ImageError::file_parse_error();
			}
			out_desc.width = out_x;
			out_desc.height = out_y;
			out_desc.format = desired_format;
			ret.resize(out_desc.width * out_desc.height * pixel_size(out_desc.format));
			memcpy(ret.data(), read_data, ret.size());
			stbi_image_free(read_data);
			return ret;
		}

		inline bool check_png_format(ImagePixelFormat pixel_format)
		{
			return (
				(pixel_format == ImagePixelFormat::r8_unorm) ||
				(pixel_format == ImagePixelFormat::rg8_unorm) ||
				(pixel_format == ImagePixelFormat::rgba8_unorm) ||
				(pixel_format == ImagePixelFormat::r16_unorm) ||
				(pixel_format == ImagePixelFormat::rg16_unorm) ||
				(pixel_format == ImagePixelFormat::rgba16_unorm)
				) ? true : false;
		}

		inline bool check_bmp_tga_jpg_format(ImagePixelFormat pixel_format)
		{
			return (
				(pixel_format == ImagePixelFormat::r8_unorm) ||
				(pixel_format == ImagePixelFormat::rg8_unorm) ||
				(pixel_format == ImagePixelFormat::rgba8_unorm)
				) ? true : false;
		}

		inline bool check_hdr_format(ImagePixelFormat pixel_format)
		{
			return (
				(pixel_format == ImagePixelFormat::r32_float) ||
				(pixel_format == ImagePixelFormat::rg32_float) ||
				(pixel_format == ImagePixelFormat::rgb32_float) ||
				(pixel_format == ImagePixelFormat::rgba32_float)
				) ? true : false;
		}

		LUNA_IMAGE_API RV write_png_file(ISeekableStream* stream, const ImageDesc& desc, const Blob& image_data)
		{
			if (!check_png_format(desc.format))
			{
				return set_error(BasicError::bad_arguments(), "The specified encode format does not support the image pixel format.");
			}
			int comp = stbiw_get_comp(desc.format);
			int res = stbi_write_png_to_func(stbi_write_func, (void*)&stream, desc.width, desc.height, comp, image_data.data(), (u32)pixel_size(desc.format) * desc.width);
			if (!res)
			{
				return ImageError::file_parse_error();
			}
			return ok;
		}
		LUNA_IMAGE_API RV write_bmp_file(ISeekableStream* stream, const ImageDesc& desc, const Blob& image_data)
		{
			if (!check_bmp_tga_jpg_format(desc.format))
			{
				return set_error(BasicError::bad_arguments(), "The specified encode format does not support the image pixel format.");
			}
			int comp = stbiw_get_comp(desc.format);
			int res = stbi_write_bmp_to_func(stbi_write_func, (void*)&stream, desc.width, desc.height, comp, image_data.data());
			if (!res)
			{
				return ImageError::file_parse_error();
			}
			return ok;
		}
		LUNA_IMAGE_API RV write_tga_file(ISeekableStream* stream, const ImageDesc& desc, const Blob& image_data)
		{
			if (!check_bmp_tga_jpg_format(desc.format))
			{
				return set_error(BasicError::bad_arguments(), "The specified encode format does not support the image pixel format.");
			}
			int comp = stbiw_get_comp(desc.format);
			int res = stbi_write_tga_to_func(stbi_write_func, (void*)&stream, desc.width, desc.height, comp, image_data.data());
			if (!res)
			{
				return ImageError::file_parse_error();
			}
			return ok;
		}
		LUNA_IMAGE_API RV write_jpg_file(ISeekableStream* stream, const ImageDesc& desc, const Blob& image_data, u32 quality)
		{
			if (!check_bmp_tga_jpg_format(desc.format))
			{
				return set_error(BasicError::bad_arguments(), "The specified encode format does not support the image pixel format.");
			}
			int comp = stbiw_get_comp(desc.format);
			int res = stbi_write_jpg_to_func(stbi_write_func, (void*)&stream, desc.width, desc.height, comp, image_data.data(), quality);
			if (!res)
			{
				return ImageError::file_parse_error();
			}
			return ok;
		}
		LUNA_IMAGE_API RV write_hdr_file(ISeekableStream* stream, const ImageDesc& desc, const Blob& image_data)
		{
			if (!check_hdr_format(desc.format))
			{
				return set_error(BasicError::bad_arguments(), "The specified encode format does not support the image pixel format.");
			}
			int comp = stbiw_get_comp(desc.format);
			int res = stbi_write_hdr_to_func(stbi_write_func, (void*)&stream, desc.width, desc.height, comp, (f32*)image_data.data());
			if (!res)
			{
				return ImageError::file_parse_error();
			}
			return ok;
		}

		void deinit() {}

		RV init()
		{
			stbi_init();
			return ok;
		}

		StaticRegisterModule m("Image", "", init, deinit);
	}

	namespace ImageError
	{
		LUNA_IMAGE_API errcat_t errtype()
		{
			static errcat_t e = get_error_category_by_name("ImageError");
			return e;
		}
		LUNA_IMAGE_API ErrCode file_parse_error()
		{
			static ErrCode e = get_error_code_by_name("ImageError", "file_parse_error");
			return e;
		}
	}
}