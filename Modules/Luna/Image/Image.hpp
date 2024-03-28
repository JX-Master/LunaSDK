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
#include <Luna/Runtime/Blob.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Stream.hpp>

#ifndef LUNA_IMAGE_API
#define LUNA_IMAGE_API
#endif

namespace Luna
{
    namespace Image
    {
        //! @addtogroup Image Image
        //! Image module provides functions to parse and save image files.
        //! @{

        //! Formats that can be saved in one image file (except DDS, which is identified by @ref DDSFormat).
        enum class ImageFormat : u8
        {
            //! The image format is unknown.
            unknown,
            //! Format with one component of 8-bit normalized unsigned integer.
            //! Supported by all formats except .hdr
            r8_unorm,
            //! Format with two components of 8-bit normalized unsigned integer.
            //! Supported by all formats except .hdr
            rg8_unorm,
            //! Format with three components of 8-bit normalized unsigned integer.
            //! Supported by all formats except .hdr
            rgb8_unorm,
            //! Format with four components of 8-bit normalized unsigned integer.
            //! Supported by all formats except .hdr
            rgba8_unorm,

            //! Format with one component of 16-bit normalized unsigned integer.
            //! Supported only by .png
            r16_unorm,
            //! Format with two components of 16-bit normalized unsigned integer.
            //! Supported only by .png
            rg16_unorm,
            //! Format with three components of 16-bit normalized unsigned integer.
            //! Supported only by .png
            rgb16_unorm,
            //! Format with four components of 16-bit normalized unsigned integer.
            //! Supported only by .png
            rgba16_unorm,

            //! Format with one component of 32-bit floating-point number.
            //! Supported only by .hdr
            r32_float,
            //! Format with two components of 32-bit floating-point number.
            //! Supported only by .hdr
            rg32_float,
            //! Format with three components of 32-bit floating-point number.
            //! Supported only by .hdr
            rgb32_float,
            //! Format with four components of 32-bit floating-point number.
            //! Supported only by .hdr
            rgba32_float,
        };

        //! Gets the size of one pixel of the specified format in bytes.
        //! @param[in] format The format to query.
        //! @return Returns the size of one pixel in bytes.
        inline constexpr u32 pixel_size(ImageFormat format)
        {
            switch (format)
            {
            case ImageFormat::r8_unorm:
                return 1;
            case ImageFormat::rg8_unorm:
            case ImageFormat::r16_unorm:
                return 2;
            case ImageFormat::rgb8_unorm:
                return 3;
            case ImageFormat::rgba8_unorm:
            case ImageFormat::rg16_unorm:
            case ImageFormat::r32_float:
                return 4;
            case ImageFormat::rgb16_unorm:
                return 6;
            case ImageFormat::rgba16_unorm:
            case ImageFormat::rg32_float:
                return 8;
            case ImageFormat::rgb32_float:
                return 12;
            case ImageFormat::rgba32_float:
                return 16;
            default:
                // Should not get here.
                return 0;
            }
        }

        //! Describes one image file (except DDS, which is described by @ref DDSImageDesc).
        struct ImageDesc
        {
            //! The format of the image.
            ImageFormat format;
            //! The width of the image in pixels.
            u32 width;
            //! The height of the image in pixels.
            u32 height;
        };

        //! Reads image description from image file data.
        //! @param[in] data The image file data. Image file formats are detected from data automatically.
        //! @param[in] data_size The size of the image file data in bytes.
        //! @return Returns the image description.
        LUNA_IMAGE_API R<ImageDesc> read_image_file_desc(const void* data, usize data_size);
        //! Reads image description and pixel data from image file data.
        //! @param[in] data The image file data. Image file formats are detected from data automatically.
        //! @param[in] data_size The size of the image file data in bytes.
        //! @param[in] desired_format The desired pixel format for data in the blob returned by this function. 
        //! If this does not matches the actual format of the file, pixel format conversion will be performed automatically.
        //! @param[out] out_desc The image description for the returned image data.
        //! @return Returns one blob that contains the image pixel data.
        //! Pixels are arranged in row-major order, and there is no padding between every two rows of data.
        LUNA_IMAGE_API R<Blob> read_image_file(const void* data, usize data_size, ImageFormat desired_format, ImageDesc& out_desc);

        //! Writes the image data to one PNG file.
        //! @param[in] stream The stream to write file data to.
        //! @param[in] desc The image description.
        //! @param[in] data The image pixel data. Pixels are arranged in row-major order, and there is no padding between every two rows of data.
        //! @param[in] data_size The image pixel data size in bytes.
        LUNA_IMAGE_API RV write_png_file(ISeekableStream* stream, const ImageDesc& desc, const void* data, usize data_size);

        //! Writes the image data to one BMP file.
        //! @param[in] stream The stream to write file data to.
        //! @param[in] desc The image description.
        //! @param[in] data The image pixel data. Pixels are arranged in row-major order, and there is no padding between every two rows of data.
        //! @param[in] data_size The image pixel data size in bytes.
        LUNA_IMAGE_API RV write_bmp_file(ISeekableStream* stream, const ImageDesc& desc, const void* data, usize data_size);

        //! Writes the image data to one TGA file.
        //! @param[in] stream The stream to write file data to.
        //! @param[in] desc The image description.
        //! @param[in] data The image pixel data. Pixels are arranged in row-major order, and there is no padding between every two rows of data.
        //! @param[in] data_size The image pixel data size in bytes.
        LUNA_IMAGE_API RV write_tga_file(ISeekableStream* stream, const ImageDesc& desc, const void* data, usize data_size);

        //! Writes the image data to one JPEG file.
        //! @param[in] stream The stream to write file data to.
        //! @param[in] desc The image description.
        //! @param[in] data The image pixel data. Pixels are arranged in row-major order, and there is no padding between every two rows of data.
        //! @param[in] data_size The image pixel data size in bytes.
        //! @param[in] quality The file compression quality. This value is between 1 and 100, higher quality looks better but results in a bigger image.
        LUNA_IMAGE_API RV write_jpg_file(ISeekableStream* stream, const ImageDesc& desc, const void* data, usize data_size, u32 quality);

        //! Writes the image data to one HDR file.
        //! @param[in] stream The stream to write file data to.
        //! @param[in] desc The image description.
        //! @param[in] data The image pixel data. Pixels are arranged in row-major order, and there is no padding between every two rows of data.
        //! @param[in] data_size The image pixel data size in bytes.
        LUNA_IMAGE_API RV write_hdr_file(ISeekableStream* stream, const ImageDesc& desc, const void* data, usize data_size);

        //! @}
    }
    //! @addtogroup Image
    //! @{
    //! @defgroup ImageError Image Errors
    //! @}
    namespace ImageError
    {
        //! @addtogroup ImageError
        //! @{

        LUNA_IMAGE_API errcat_t errtype();

        //! Failed to parse image file.
        LUNA_IMAGE_API ErrCode file_parse_error();

        //! @}
    }

    struct Module;
    LUNA_IMAGE_API Module* module_image();
}
