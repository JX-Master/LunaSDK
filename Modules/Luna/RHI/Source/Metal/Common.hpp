/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Common.hpp
* @author JXMaster
* @date 2023/7/12
*/
#pragma once
#include "../../RHI.hpp"
#include "../RHI.hpp"
#include "Metal.hpp"
namespace Luna
{
    namespace RHI
    {
		struct AutoreleasePool
		{
			NS::AutoreleasePool* pool;

			AutoreleasePool() :
				pool(NS::AutoreleasePool::alloc()->init()) {}
			~AutoreleasePool()
			{
				pool->release();
			}
		};

		template <typename _Ty>
		class NSPtr
		{
			_Ty* obj;

			void internal_addref()
			{
				if(obj) obj->retain();
			}
			void internal_clear()
			{
				if(obj) obj->release();
			}
		public:
			NSPtr() : obj(nullptr) {}
			NSPtr(const NSPtr& rhs) : obj(rhs.obj) { internal_addref(); }
			NSPtr(NSPtr&& rhs) : obj(rhs.obj) { rhs.obj = nullptr; }
			NSPtr& operator=(const NSPtr& rhs)
			{
				internal_clear();
				obj = rhs.obj;
				internal_addref();
				return *this;
			}
			NSPtr& operator=(NSPtr&& rhs)
			{
				internal_clear();
				obj = rhs.obj;
				rhs.obj = nullptr;
				return *this;
			}
			~NSPtr() { internal_clear(); }
			_Ty* get() const { return obj; }
			_Ty* operator->() const { return get(); }
			void attach(_Ty* ptr)
			{
				internal_clear();
				obj = ptr;
			}
			_Ty* detach()
			{
				_Ty* r = obj;
				obj = nullptr;
				return r;
			}
			bool valid() const { return obj != nullptr; }
			operator bool() const { return valid(); }
			void reset() { internal_clear(); obj = nullptr; }
		};
		// Creates one NSPtr from pointer without adding reference count.
		// Used for pointers returned from `allocXXX`, `newXXX`, `copyXXX`, `mutableCopyXXX`.
		template <typename _Ty>
		inline NSPtr<_Ty> box(_Ty* ptr)
		{
			NSPtr<_Ty> r;
			r.attach(ptr);
			return r;
		}
		// Creates one NSPtr from pointer and adds its reference count.
		// Used for pointers returned from all other functions that `box` is not suitable.
		template <typename _Ty>
		inline NSPtr<_Ty> retain(_Ty* ptr)
		{
			NSPtr<_Ty> r;
			r.attach(ptr);
			r->retain();
			return r;
		}

		template <typename _Ty>
		inline void set_object_name(_Ty* obj, const c8* name)
		{
			AutoreleasePool pool;
            NS::String* label = NS::String::string(name, NS::StringEncoding::UTF8StringEncoding);
            obj->setLabel(label);
		}

        inline MTL::PixelFormat encode_pixel_format(Format f)
        {
            switch(f)
            {
            case Format::unknown: 	return MTL::PixelFormat::PixelFormatInvalid;
			case Format::r8_unorm: 	return MTL::PixelFormat::PixelFormatR8Unorm;
			case Format::r8_snorm: 	return MTL::PixelFormat::PixelFormatR8Snorm;
			case Format::r8_uint: 	return MTL::PixelFormat::PixelFormatR8Uint;
			case Format::r8_sint: 	return MTL::PixelFormat::PixelFormatR8Sint;

			case Format::r16_unorm: return MTL::PixelFormat::PixelFormatR16Unorm;
			case Format::r16_snorm: return MTL::PixelFormat::PixelFormatR16Snorm;
			case Format::r16_uint: 	return MTL::PixelFormat::PixelFormatR16Uint;
			case Format::r16_sint: 	return MTL::PixelFormat::PixelFormatR16Sint;
			case Format::r16_float: return MTL::PixelFormat::PixelFormatR16Float;
			case Format::rg8_unorm: return MTL::PixelFormat::PixelFormatRG8Unorm;
			case Format::rg8_snorm: return MTL::PixelFormat::PixelFormatRG8Snorm;
			case Format::rg8_uint: 	return MTL::PixelFormat::PixelFormatRG8Uint;
			case Format::rg8_sint: 	return MTL::PixelFormat::PixelFormatRG8Sint;

			case Format::r32_uint: 	return MTL::PixelFormat::PixelFormatR32Uint;
			case Format::r32_sint: 	return MTL::PixelFormat::PixelFormatR32Sint;
			case Format::r32_float: return MTL::PixelFormat::PixelFormatR32Float;

			case Format::rg16_unorm:		return MTL::PixelFormat::PixelFormatRG16Unorm;
			case Format::rg16_snorm: 		return MTL::PixelFormat::PixelFormatRG16Snorm;
			case Format::rg16_uint: 		return MTL::PixelFormat::PixelFormatRG16Uint;
			case Format::rg16_sint: 		return MTL::PixelFormat::PixelFormatRG16Sint;
			case Format::rg16_float: 		return MTL::PixelFormat::PixelFormatRG16Float;
			case Format::rgba8_unorm: 		return MTL::PixelFormat::PixelFormatRGBA8Unorm;
			case Format::rgba8_unorm_srgb: 	return MTL::PixelFormat::PixelFormatRGBA8Unorm_sRGB;
			case Format::rgba8_snorm: 		return MTL::PixelFormat::PixelFormatRGBA8Snorm;
			case Format::rgba8_uint: 		return MTL::PixelFormat::PixelFormatRGBA8Uint;
			case Format::rgba8_sint: 		return MTL::PixelFormat::PixelFormatRGBA8Sint;
			case Format::bgra8_unorm: 		return MTL::PixelFormat::PixelFormatBGRA8Unorm;
			case Format::bgra8_unorm_srgb: 	return MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB;
			case Format::rg32_uint: 		return MTL::PixelFormat::PixelFormatRG32Uint;
			case Format::rg32_sint: 		return MTL::PixelFormat::PixelFormatRG32Sint;
			case Format::rg32_float: 		return MTL::PixelFormat::PixelFormatRG32Float;
			case Format::rgba16_unorm: 		return MTL::PixelFormat::PixelFormatRGBA16Unorm;
			case Format::rgba16_snorm: 		return MTL::PixelFormat::PixelFormatRGBA16Snorm;
			case Format::rgba16_uint: 		return MTL::PixelFormat::PixelFormatRGBA16Uint;
			case Format::rgba16_sint: 		return MTL::PixelFormat::PixelFormatRGBA16Sint;
			case Format::rgba16_float: 		return MTL::PixelFormat::PixelFormatRGBA16Float;
			case Format::rgba32_uint: 		return MTL::PixelFormat::PixelFormatRGBA32Uint;
			case Format::rgba32_sint: 		return MTL::PixelFormat::PixelFormatRGBA32Sint;
			case Format::rgba32_float: 		return MTL::PixelFormat::PixelFormatRGBA32Float;

			case Format::b5g6r5_unorm: 		return MTL::PixelFormat::PixelFormatB5G6R5Unorm;
			case Format::bgr5a1_unorm: 		return MTL::PixelFormat::PixelFormatBGR5A1Unorm;

			case Format::rgb10a2_unorm: 	return MTL::PixelFormat::PixelFormatRGB10A2Unorm;
			case Format::rgb10a2_uint: 		return MTL::PixelFormat::PixelFormatRGB10A2Uint;
			case Format::rg11b10_float: 	return MTL::PixelFormat::PixelFormatRG11B10Float;
			case Format::rgb9e5_float: 		return MTL::PixelFormat::PixelFormatRGB9E5Float;

			case Format::d16_unorm: 			return MTL::PixelFormat::PixelFormatDepth16Unorm;
			case Format::d32_float: 			return MTL::PixelFormat::PixelFormatDepth32Float;
			case Format::d24_unorm_s8_uint: 	return MTL::PixelFormat::PixelFormatDepth24Unorm_Stencil8;
			case Format::d32_float_s8_uint_x24: return MTL::PixelFormat::PixelFormatDepth32Float_Stencil8;

			case Format::bc1_rgba_unorm: 		return MTL::PixelFormat::PixelFormatBC1_RGBA;
			case Format::bc1_rgba_unorm_srgb: 	return MTL::PixelFormat::PixelFormatBC1_RGBA_sRGB;
			case Format::bc2_rgba_unorm: 		return MTL::PixelFormat::PixelFormatBC2_RGBA;
			case Format::bc2_rgba_unorm_srgb: 	return MTL::PixelFormat::PixelFormatBC2_RGBA_sRGB;
			case Format::bc3_rgba_unorm: 		return MTL::PixelFormat::PixelFormatBC3_RGBA;
			case Format::bc3_rgba_unorm_srgb: 	return MTL::PixelFormat::PixelFormatBC3_RGBA_sRGB;
			case Format::bc4_r_unorm: 			return MTL::PixelFormat::PixelFormatBC4_RUnorm;
			case Format::bc4_r_snorm: 			return MTL::PixelFormat::PixelFormatBC4_RSnorm;
			case Format::bc5_rg_unorm: 			return MTL::PixelFormat::PixelFormatBC5_RGUnorm;
			case Format::bc5_rg_snorm: 			return MTL::PixelFormat::PixelFormatBC5_RGSnorm;
			case Format::bc6h_rgb_sfloat: 		return MTL::PixelFormat::PixelFormatBC6H_RGBFloat;
			case Format::bc6h_rgb_ufloat: 		return MTL::PixelFormat::PixelFormatBC6H_RGBUfloat;
			case Format::bc7_rgba_unorm: 		return MTL::PixelFormat::PixelFormatBC7_RGBAUnorm;
			case Format::bc7_rgba_unorm_srgb: 	return MTL::PixelFormat::PixelFormatBC7_RGBAUnorm_sRGB;
			default:
				return MTL::PixelFormat::PixelFormatInvalid;
            }
        }
		inline Format decode_pixel_format(MTL::PixelFormat format)
		{
			switch(format)
			{
			case MTL::PixelFormat::PixelFormatInvalid: return Format::unknown;
			case MTL::PixelFormat::PixelFormatR8Unorm: return Format::r8_unorm;
			case MTL::PixelFormat::PixelFormatR8Snorm: return Format::r8_snorm;
			case MTL::PixelFormat::PixelFormatR8Uint: return Format::r8_uint;
			case MTL::PixelFormat::PixelFormatR8Sint: return Format::r8_sint;

			case MTL::PixelFormat::PixelFormatR16Unorm: return Format::r16_unorm; 	
			case MTL::PixelFormat::PixelFormatR16Snorm: return Format::r16_snorm; 	
			case MTL::PixelFormat::PixelFormatR16Uint: return Format::r16_uint; 	
			case MTL::PixelFormat::PixelFormatR16Sint: return Format::r16_sint; 	
			case MTL::PixelFormat::PixelFormatR16Float: return Format::r16_float; 	
			case MTL::PixelFormat::PixelFormatRG8Unorm: return Format::rg8_unorm; 	
			case MTL::PixelFormat::PixelFormatRG8Snorm: return Format::rg8_snorm; 	
			case MTL::PixelFormat::PixelFormatRG8Uint: return Format::rg8_uint; 	
			case MTL::PixelFormat::PixelFormatRG8Sint: return Format::rg8_sint; 	

			case MTL::PixelFormat::PixelFormatR32Uint: return Format::r32_uint;
			case MTL::PixelFormat::PixelFormatR32Sint: return Format::r32_sint;
			case MTL::PixelFormat::PixelFormatR32Float: return Format::r32_float;

			case MTL::PixelFormat::PixelFormatRG16Unorm: return Format::rg16_unorm;
			case MTL::PixelFormat::PixelFormatRG16Snorm: return Format::rg16_snorm;
			case MTL::PixelFormat::PixelFormatRG16Uint: return Format::rg16_uint;
			case MTL::PixelFormat::PixelFormatRG16Sint: return Format::rg16_sint;
			case MTL::PixelFormat::PixelFormatRG16Float: return Format::rg16_float;
			case MTL::PixelFormat::PixelFormatRGBA8Unorm: return Format::rgba8_unorm;
			case MTL::PixelFormat::PixelFormatRGBA8Unorm_sRGB: return Format::rgba8_unorm_srgb;
			case MTL::PixelFormat::PixelFormatRGBA8Snorm: return Format::rgba8_snorm;
			case MTL::PixelFormat::PixelFormatRGBA8Uint: return Format::rgba8_uint;
			case MTL::PixelFormat::PixelFormatRGBA8Sint: return Format::rgba8_sint;
			case MTL::PixelFormat::PixelFormatBGRA8Unorm: return Format::bgra8_unorm;
			case MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB: return Format::bgra8_unorm_srgb;
			case MTL::PixelFormat::PixelFormatRG32Uint: return Format::rg32_uint;
			case MTL::PixelFormat::PixelFormatRG32Sint: return Format::rg32_sint;
			case MTL::PixelFormat::PixelFormatRG32Float: return Format::rg32_float;
			case MTL::PixelFormat::PixelFormatRGBA16Unorm: return Format::rgba16_unorm;
			case MTL::PixelFormat::PixelFormatRGBA16Snorm: return Format::rgba16_snorm;
			case MTL::PixelFormat::PixelFormatRGBA16Uint: return Format::rgba16_uint;
			case MTL::PixelFormat::PixelFormatRGBA16Sint: return Format::rgba16_sint;
			case MTL::PixelFormat::PixelFormatRGBA16Float: return Format::rgba16_float;
			case MTL::PixelFormat::PixelFormatRGBA32Uint: return Format::rgba32_uint;
			case MTL::PixelFormat::PixelFormatRGBA32Sint: return Format::rgba32_sint;
			case MTL::PixelFormat::PixelFormatRGBA32Float: return Format::rgba32_float;

			case MTL::PixelFormat::PixelFormatB5G6R5Unorm: return Format::b5g6r5_unorm;
			case MTL::PixelFormat::PixelFormatBGR5A1Unorm: return Format::bgr5a1_unorm;

			case MTL::PixelFormat::PixelFormatRGB10A2Unorm: return Format::rgb10a2_unorm;
			case MTL::PixelFormat::PixelFormatRGB10A2Uint: return Format::rgb10a2_uint;
			case MTL::PixelFormat::PixelFormatRG11B10Float: return Format::rg11b10_float;
			case MTL::PixelFormat::PixelFormatRGB9E5Float: return Format::rgb9e5_float;

			case MTL::PixelFormat::PixelFormatDepth16Unorm: return Format::d16_unorm;
			case MTL::PixelFormat::PixelFormatDepth32Float: return Format::d32_float;
			case MTL::PixelFormat::PixelFormatDepth24Unorm_Stencil8: return Format::d24_unorm_s8_uint;
			case MTL::PixelFormat::PixelFormatDepth32Float_Stencil8: return Format::d32_float_s8_uint_x24;

			case MTL::PixelFormat::PixelFormatBC1_RGBA: return Format::bc1_rgba_unorm;
			case MTL::PixelFormat::PixelFormatBC1_RGBA_sRGB: return Format::bc1_rgba_unorm_srgb;
			case MTL::PixelFormat::PixelFormatBC2_RGBA: return Format::bc2_rgba_unorm;
			case MTL::PixelFormat::PixelFormatBC2_RGBA_sRGB: return Format::bc2_rgba_unorm_srgb;
			case MTL::PixelFormat::PixelFormatBC3_RGBA: return Format::bc3_rgba_unorm;
			case MTL::PixelFormat::PixelFormatBC3_RGBA_sRGB: return Format::bc3_rgba_unorm_srgb;
			case MTL::PixelFormat::PixelFormatBC4_RUnorm: return Format::bc4_r_unorm;
			case MTL::PixelFormat::PixelFormatBC4_RSnorm: return Format::bc4_r_snorm;
			case MTL::PixelFormat::PixelFormatBC5_RGUnorm: return Format::bc5_rg_unorm;
			case MTL::PixelFormat::PixelFormatBC5_RGSnorm: return Format::bc5_rg_snorm;
			case MTL::PixelFormat::PixelFormatBC6H_RGBFloat: return Format::bc6h_rgb_sfloat;
			case MTL::PixelFormat::PixelFormatBC6H_RGBUfloat: return Format::bc6h_rgb_ufloat;
			case MTL::PixelFormat::PixelFormatBC7_RGBAUnorm: return Format::bc7_rgba_unorm;
			case MTL::PixelFormat::PixelFormatBC7_RGBAUnorm_sRGB: return Format::bc7_rgba_unorm_srgb;
			default: return Format::unknown;
			}
		}
		inline MTL::TextureType encode_texture_view_type(TextureViewType type)
		{
			switch(type)
			{
				case TextureViewType::tex1d: return MTL::TextureType1D;
				case TextureViewType::tex2d: return MTL::TextureType2D;
				case TextureViewType::tex2dms: return MTL::TextureType2DMultisample;
				case TextureViewType::tex3d: return MTL::TextureType3D;
				case TextureViewType::texcube: return MTL::TextureTypeCube;
				case TextureViewType::tex1darray: return MTL::TextureType1DArray;
				case TextureViewType::tex2darray: return MTL::TextureType2DArray;
				case TextureViewType::tex2dmsarray: return MTL::TextureType2DMultisampleArray;
				case TextureViewType::texcubearray: return MTL::TextureTypeCubeArray;
				default: lupanic(); return MTL::TextureType2D;
			}
		}
		inline MTL::VertexFormat encode_vertex_format(Format f)
        {
            switch(f)
            {

			case Format::unknown: return MTL::VertexFormat::VertexFormatInvalid;
			case Format::r8_unorm: return MTL::VertexFormat::VertexFormatUCharNormalized;
			case Format::r8_snorm: return MTL::VertexFormat::VertexFormatCharNormalized;
			case Format::r8_uint: return MTL::VertexFormat::VertexFormatUChar;
			case Format::r8_sint: return MTL::VertexFormat::VertexFormatChar;

			case Format::r16_unorm: return MTL::VertexFormat::VertexFormatUShortNormalized;
			case Format::r16_snorm: return MTL::VertexFormat::VertexFormatShortNormalized;
			case Format::r16_uint: return MTL::VertexFormat::VertexFormatUShort;
			case Format::r16_sint: return MTL::VertexFormat::VertexFormatShort;
			case Format::r16_float: return MTL::VertexFormat::VertexFormatHalf;
			case Format::rg8_unorm: return MTL::VertexFormat::VertexFormatUChar2Normalized;
			case Format::rg8_snorm: return MTL::VertexFormat::VertexFormatChar2Normalized;
			case Format::rg8_uint: return MTL::VertexFormat::VertexFormatUChar2;
			case Format::rg8_sint: return MTL::VertexFormat::VertexFormatChar2;

			case Format::r32_uint: return MTL::VertexFormat::VertexFormatUInt;
			case Format::r32_sint: return MTL::VertexFormat::VertexFormatInt;
			case Format::r32_float: return MTL::VertexFormat::VertexFormatFloat;

			case Format::rg16_unorm: return MTL::VertexFormat::VertexFormatUShort2Normalized;
			case Format::rg16_snorm: return MTL::VertexFormat::VertexFormatShort2Normalized;
			case Format::rg16_uint: return MTL::VertexFormat::VertexFormatUShort2;
			case Format::rg16_sint: return MTL::VertexFormat::VertexFormatShort2;
			case Format::rg16_float: return MTL::VertexFormat::VertexFormatHalf2;
			case Format::rgba8_unorm: return MTL::VertexFormat::VertexFormatUChar4Normalized;
			case Format::rgba8_snorm: return MTL::VertexFormat::VertexFormatChar4Normalized;
			case Format::rgba8_uint: return MTL::VertexFormat::VertexFormatUChar4;
			case Format::rgba8_sint: return MTL::VertexFormat::VertexFormatChar4;
			case Format::rg32_uint: return MTL::VertexFormat::VertexFormatUInt2;
			case Format::rg32_sint: return MTL::VertexFormat::VertexFormatInt2;
			case Format::rg32_float: return MTL::VertexFormat::VertexFormatFloat2;
			case Format::rgba16_unorm: return MTL::VertexFormat::VertexFormatUShort4Normalized;
			case Format::rgba16_snorm: return MTL::VertexFormat::VertexFormatShort4Normalized;
			case Format::rgba16_uint: return MTL::VertexFormat::VertexFormatUShort4;
			case Format::rgba16_sint: return MTL::VertexFormat::VertexFormatShort4;
			case Format::rgba16_float: return MTL::VertexFormat::VertexFormatHalf4;
			case Format::rgb32_uint: return MTL::VertexFormat::VertexFormatUInt3;
			case Format::rgb32_sint: return MTL::VertexFormat::VertexFormatInt3;
			case Format::rgb32_float: return MTL::VertexFormat::VertexFormatFloat3;
			case Format::rgba32_uint: return MTL::VertexFormat::VertexFormatUInt4;
			case Format::rgba32_sint: return MTL::VertexFormat::VertexFormatInt4;
			case Format::rgba32_float: return MTL::VertexFormat::VertexFormatFloat4;
            
            default:
                return MTL::VertexFormat::VertexFormatInvalid;
            }
        }

		inline MTL::StorageMode encode_storage_mode(MemoryType type)
		{
			switch(type)
			{
				case MemoryType::local: return MTL::StorageModePrivate;
				case MemoryType::upload: return MTL::StorageModeShared;
				case MemoryType::readback: return MTL::StorageModeShared;
			}
		}
		inline MTL::CPUCacheMode encode_cpu_cache_mode(MemoryType type)
		{
			switch(type)
			{
				case MemoryType::local: return MTL::CPUCacheModeDefaultCache;
				case MemoryType::upload: return MTL::CPUCacheModeWriteCombined;
				case MemoryType::readback: return MTL::CPUCacheModeDefaultCache;
			}
		}

		struct MTLBufferDesc
		{
			NS::Integer length;
			MTL::ResourceOptions options;
		};

		inline MTL::ResourceOptions encode_resource_options(MemoryType memory_type)
		{
			MTL::ResourceOptions options = 0;
			if(memory_type == MemoryType::local)
			{
				options |= MTL::ResourceStorageModePrivate;
			}
			else if(memory_type == MemoryType::upload)
			{
				options |= MTL::ResourceCPUCacheModeWriteCombined;
			}
			options |= MTL::ResourceHazardTrackingModeTracked;
			return options;
		}

		inline MTL::TextureUsage encode_texture_usage(TextureUsageFlag usages)
		{
			MTL::TextureUsage ret = 0;
			if(test_flags(usages, TextureUsageFlag::read_texture))
			{
				ret |= MTL::TextureUsageShaderRead;
			}
			if(test_flags(usages, TextureUsageFlag::read_write_texture))
			{
				ret |= MTL::TextureUsageShaderRead | MTL::TextureUsageShaderWrite;
			}
			if(test_flags(usages, TextureUsageFlag::color_attachment) ||
				test_flags(usages, TextureUsageFlag::depth_stencil_attachment))
			{
				ret |= MTL::TextureUsageRenderTarget;
			}
			return ret;
		}

		inline TextureUsageFlag decode_texture_usage(MTL::TextureUsage usages, bool is_depth_stencil_format)
		{
			TextureUsageFlag ret = TextureUsageFlag::none;
			if(usages & MTL::TextureUsageShaderRead)
			{
				ret |= TextureUsageFlag::read_texture;
			}
			if(usages & MTL::TextureUsageShaderWrite)
			{
				ret |= TextureUsageFlag::read_write_texture;
			}
			if(usages & MTL::TextureUsageRenderTarget)
			{
				if(is_depth_stencil_format)
				{
					ret |= TextureUsageFlag::depth_stencil_attachment;
				}
				else
				{
					ret |= TextureUsageFlag::color_attachment;
				}
			}
			return ret;
		}

		inline MTLBufferDesc encode_buffer_desc(MemoryType memory_type, const BufferDesc& desc)
		{
			MTLBufferDesc ret;
			ret.length = desc.size;
			ret.options = encode_resource_options(memory_type);
			return ret;
		}

		inline NSPtr<MTL::TextureDescriptor> encode_texture_desc(MemoryType memory_type, const TextureDesc& desc)
		{
			NSPtr<MTL::TextureDescriptor> ret = box(MTL::TextureDescriptor::alloc()->init());
			if(desc.type == TextureType::tex1d)
			{
				ret->setTextureType(desc.array_size == 1 ? MTL::TextureType1D : MTL::TextureType1DArray);
			}
			else if(desc.type == TextureType::tex2d)
			{
				if(desc.sample_count == 1)
				{
					if(test_flags(desc.usages, TextureUsageFlag::cube))
					{
						ret->setTextureType(desc.array_size == 6 ? MTL::TextureTypeCube : MTL::TextureTypeCubeArray);
					}
					else
					{
						ret->setTextureType(desc.array_size == 1 ? MTL::TextureType2D : MTL::TextureType2DArray);
					}
				}
				else
				{
					ret->setTextureType(desc.array_size == 1 ? MTL::TextureType2DMultisample : MTL::TextureType2DMultisampleArray);
				}
			}
			else if(desc.type == TextureType::tex3d)
			{
				ret->setTextureType(MTL::TextureType3D);
			}
			ret->setPixelFormat(encode_pixel_format(desc.format));
			ret->setWidth(desc.width);
			ret->setHeight(desc.height);
			ret->setDepth(desc.depth);
			ret->setMipmapLevelCount(desc.mip_levels);
			ret->setSampleCount(desc.sample_count);
			if(test_flags(desc.usages, TextureUsageFlag::cube))
			{
				ret->setArrayLength(desc.array_size / 6);
			}
			else
			{
				ret->setArrayLength(desc.array_size);
			}
			ret->setResourceOptions(encode_resource_options(memory_type));
			ret->setCpuCacheMode(encode_cpu_cache_mode(memory_type));
			ret->setStorageMode(encode_storage_mode(memory_type));
			ret->setUsage(encode_texture_usage(desc.usages));
			ret->setHazardTrackingMode(MTL::HazardTrackingModeTracked);
			return ret;
		}
		inline MTL::CompareFunction encode_compare_function(CompareFunction func)
		{
			switch(func)
			{
				case CompareFunction::never: return MTL::CompareFunctionNever;
				case CompareFunction::less: return MTL::CompareFunctionLess;
				case CompareFunction::equal: return MTL::CompareFunctionEqual;
				case CompareFunction::less_equal: return MTL::CompareFunctionLessEqual;
				case CompareFunction::greater: return MTL::CompareFunctionGreater;
				case CompareFunction::not_equal: return MTL::CompareFunctionNotEqual;
				case CompareFunction::greater_equal: return MTL::CompareFunctionGreaterEqual;
				case CompareFunction::always: return MTL::CompareFunctionAlways;
			}
			lupanic();
			return MTL::CompareFunctionNever;
		}
		inline MTL::SamplerMinMagFilter encode_min_mag_filter(Filter filter)
		{
			switch(filter)
			{
				case Filter::nearest: return MTL::SamplerMinMagFilterNearest;
				case Filter::linear: return MTL::SamplerMinMagFilterLinear;
			}
			lupanic();
			return MTL::SamplerMinMagFilterNearest;
		}
		inline MTL::SamplerMipFilter encode_mip_filter(Filter filter)
		{
			switch(filter)
			{
				case Filter::nearest: return MTL::SamplerMipFilterNearest;
				case Filter::linear: return MTL::SamplerMipFilterLinear;
			}
			lupanic();
			return MTL::SamplerMipFilterNearest;
		}
		inline MTL::SamplerAddressMode encode_address_mode(TextureAddressMode mode)
		{
			switch(mode)
			{
				case TextureAddressMode::repeat: return MTL::SamplerAddressModeRepeat;
				case TextureAddressMode::mirror: return MTL::SamplerAddressModeMirrorRepeat;
				case TextureAddressMode::clamp: return MTL::SamplerAddressModeClampToEdge;
				case TextureAddressMode::border: return MTL::SamplerAddressModeClampToBorderColor;
			}
			lupanic();
			return MTL::SamplerAddressModeRepeat;
		}
		inline MTL::BlendOperation encode_blend_op(BlendOp op)
		{
			switch(op)
			{
				case BlendOp::add: return MTL::BlendOperationAdd;
				case BlendOp::subtract: return MTL::BlendOperationSubtract;
				case BlendOp::rev_subtract: return MTL::BlendOperationReverseSubtract;
				case BlendOp::min: return MTL::BlendOperationMin;
				case BlendOp::max: return MTL::BlendOperationMax;
			}
			lupanic();
			return MTL::BlendOperationAdd;
		}
		inline MTL::BlendFactor encode_blend_factor(BlendFactor factor, bool is_rgb)
		{
			switch(factor)
			{
				case BlendFactor::zero: return MTL::BlendFactorZero;
				case BlendFactor::one: return MTL::BlendFactorOne;
				case BlendFactor::src_color: return MTL::BlendFactorSourceColor;
				case BlendFactor::one_minus_src_color: return MTL::BlendFactorOneMinusSourceColor;
				case BlendFactor::src_alpha: return MTL::BlendFactorSourceAlpha;
				case BlendFactor::one_minus_src_alpha: return MTL::BlendFactorOneMinusSourceAlpha;
				case BlendFactor::dst_color: return MTL::BlendFactorDestinationColor;
				case BlendFactor::one_minus_dst_color: return MTL::BlendFactorOneMinusDestinationColor;
				case BlendFactor::dst_alpha: return MTL::BlendFactorDestinationAlpha;
				case BlendFactor::one_minus_dst_alpha: return MTL::BlendFactorOneMinusDestinationAlpha;
				case BlendFactor::src_alpha_saturated: return MTL::BlendFactorSourceAlphaSaturated;
				case BlendFactor::blend_factor: return is_rgb ? MTL::BlendFactorBlendColor : MTL::BlendFactorBlendAlpha;
				case BlendFactor::one_minus_blend_factor: return is_rgb ? MTL::BlendFactorOneMinusBlendColor : MTL::BlendFactorOneMinusBlendAlpha;
				case BlendFactor::src1_color: return MTL::BlendFactorSource1Color;
				case BlendFactor::one_minus_src1_color: return MTL::BlendFactorOneMinusSource1Color;
				case BlendFactor::src1_alpha: return MTL::BlendFactorSource1Alpha;
				case BlendFactor::one_minus_src1_alpha: return MTL::BlendFactorOneMinusSource1Alpha;
			}
			lupanic();
			return MTL::BlendFactorZero;
		}
		inline MTL::PrimitiveTopologyClass encode_primitive_topology(PrimitiveTopology topology)
		{
			switch(topology)
			{
				case PrimitiveTopology::point_list: return MTL::PrimitiveTopologyClassPoint;
				case PrimitiveTopology::line_list:
				case PrimitiveTopology::line_strip: return MTL::PrimitiveTopologyClassLine;
				case PrimitiveTopology::triangle_list:
				case PrimitiveTopology::triangle_strip: return MTL::PrimitiveTopologyClassTriangle;
			}
			lupanic();
			return MTL::PrimitiveTopologyClassUnspecified;
		}
		inline MTL::LoadAction encode_load_action(LoadOp op)
		{
			switch(op)
			{
				case LoadOp::dont_care: return MTL::LoadActionDontCare;
				case LoadOp::load: return MTL::LoadActionLoad;
				case LoadOp::clear: return MTL::LoadActionClear;
			}
			lupanic();
			return MTL::LoadActionDontCare;
		}
		inline MTL::StoreAction encode_store_action(StoreOp op, bool resolve)
		{
			switch(op)
			{
				case StoreOp::dont_care: return resolve ? MTL::StoreActionMultisampleResolve : MTL::StoreActionDontCare;
				case StoreOp::store: return resolve ? MTL::StoreActionStoreAndMultisampleResolve : MTL::StoreActionStore;
			}
			lupanic();
			return MTL::StoreActionDontCare;
		}
		inline MTL::StencilOperation encode_stencil_operation(StencilOp op)
		{
			switch(op)
			{
				case StencilOp::keep: return MTL::StencilOperationKeep;
				case StencilOp::zero: return MTL::StencilOperationZero;
				case StencilOp::replace: return MTL::StencilOperationReplace;
				case StencilOp::increment_saturated: return MTL::StencilOperationIncrementClamp;
				case StencilOp::decrement_saturated: return MTL::StencilOperationDecrementClamp;
				case StencilOp::invert: return MTL::StencilOperationInvert;
				case StencilOp::increment: return MTL::StencilOperationIncrementWrap;
				case StencilOp::decrement: return MTL::StencilOperationDecrementWrap;
			}
			lupanic();
			return MTL::StencilOperationKeep;
		}
		inline MTL::PrimitiveType encode_primitive_type(PrimitiveTopology pt)
		{
			switch(pt)
			{
				case PrimitiveTopology::point_list: return MTL::PrimitiveType::PrimitiveTypePoint;
				case PrimitiveTopology::line_list: return MTL::PrimitiveTypeLine;
				case PrimitiveTopology::line_strip: return MTL::PrimitiveTypeLineStrip;
				case PrimitiveTopology::triangle_list: return MTL::PrimitiveTypeTriangle;
				case PrimitiveTopology::triangle_strip: return MTL::PrimitiveTypeTriangleStrip;
			}
			lupanic();
			return MTL::PrimitiveType::PrimitiveTypePoint;
		}
		inline MTL::IndexType encode_index_type(Format format)
		{
			switch(format)
			{
				case Format::r16_uint:
				case Format::r16_sint: return MTL::IndexTypeUInt16;
				case Format::r32_uint:
				case Format::r32_sint: return MTL::IndexTypeUInt32;
                default: break;
			}
			lupanic();
			return MTL::IndexTypeUInt32;
		}
		inline MTL::RenderStages encode_render_stage(BufferStateFlag flags)
		{
			MTL::RenderStages r = 0;
			if(test_flags(flags, BufferStateFlag::shader_read_vs) || test_flags(flags, BufferStateFlag::uniform_buffer_vs))
			{
				r |= MTL::RenderStageVertex;
			}
			if(test_flags(flags, BufferStateFlag::shader_read_ps) || test_flags(flags, BufferStateFlag::uniform_buffer_ps))
			{
				r |= MTL::RenderStageFragment;
			}
			return r;
		}
		inline MTL::RenderStages encode_render_stage(TextureStateFlag flags)
		{
			MTL::RenderStages r = 0;
			if(test_flags(flags, TextureStateFlag::shader_read_vs))
			{
				r |= MTL::RenderStageVertex;
			}
			if(test_flags(flags, TextureStateFlag::shader_read_ps))
			{
				r |= MTL::RenderStageFragment;
			}
			return r;
		}
        inline bool is_depth_format(Format format)
        {
            switch(format)
            {
                case Format::d16_unorm:
                case Format::d32_float:
                case Format::d24_unorm_s8_uint:
                case Format::d32_float_s8_uint_x24:
                    return true;
                default:
                    return false;
            }
        }
        inline bool is_stencil_format(Format format)
        {
            switch(format)
            {
                case Format::d24_unorm_s8_uint:
                case Format::d32_float_s8_uint_x24:
                    return true;
                default:
                    return false;
            }
        }
    }
}
