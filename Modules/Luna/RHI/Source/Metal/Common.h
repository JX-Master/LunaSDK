/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Common.h
* @author JXMaster
* @date 2023/7/12
*/
#pragma once
#include "../../RHI.hpp"
#include "../RHI.hpp"

#import <Metal/Metal.h>

namespace Luna
{
    namespace RHI
    {
        template <typename _Ty>
        struct NSWrapper
        {
            NSWrapper() :
                m_obj(nil) {}
            NSWrapper(const NSWrapper&) = default;
            NSWrapper(NSWrapper&&) = default;
            NSWrapper& operator=(const NSWrapper&) = default;
            NSWrapper& operator=(NSWrapper&&) = default;
            NSWrapper(_Ty obj) :
                m_obj(obj) {}
            ~NSWrapper() {}

            operator _Ty()
            {
                return m_obj;
            }
        private:
            _Ty m_obj;
        };
        inline MTLPixelFormat encode_pixel_format(Format f)
        {
            switch(f)
            {
            case Format::unknown:      return MTLPixelFormatInvalid;
            case Format::r8_unorm:     return MTLPixelFormatR8Unorm;
            case Format::r8_snorm:     return MTLPixelFormatR8Snorm;
            case Format::r8_uint:      return MTLPixelFormatR8Uint;
            case Format::r8_sint:      return MTLPixelFormatR8Sint;

            case Format::r16_unorm:    return MTLPixelFormatR16Unorm;
            case Format::r16_snorm:    return MTLPixelFormatR16Snorm;
            case Format::r16_uint:     return MTLPixelFormatR16Uint;
            case Format::r16_sint:     return MTLPixelFormatR16Sint;
            case Format::r16_float:    return MTLPixelFormatR16Float;
            case Format::rg8_unorm:    return MTLPixelFormatRG8Unorm;
            case Format::rg8_snorm:    return MTLPixelFormatRG8Snorm;
            case Format::rg8_uint:     return MTLPixelFormatRG8Uint;
            case Format::rg8_sint:     return MTLPixelFormatRG8Sint;

            case Format::r32_uint:     return MTLPixelFormatR32Uint;
            case Format::r32_sint:     return MTLPixelFormatR32Sint;
            case Format::r32_float:    return MTLPixelFormatR32Float;

            case Format::rg16_unorm:         return MTLPixelFormatRG16Unorm;
            case Format::rg16_snorm:         return MTLPixelFormatRG16Snorm;
            case Format::rg16_uint:          return MTLPixelFormatRG16Uint;
            case Format::rg16_sint:          return MTLPixelFormatRG16Sint;
            case Format::rg16_float:         return MTLPixelFormatRG16Float;
            case Format::rgba8_unorm:        return MTLPixelFormatRGBA8Unorm;
            case Format::rgba8_unorm_srgb:   return MTLPixelFormatRGBA8Unorm_sRGB;
            case Format::rgba8_snorm:        return MTLPixelFormatRGBA8Snorm;
            case Format::rgba8_uint:         return MTLPixelFormatRGBA8Uint;
            case Format::rgba8_sint:         return MTLPixelFormatRGBA8Sint;
            case Format::bgra8_unorm:        return MTLPixelFormatBGRA8Unorm;
            case Format::bgra8_unorm_srgb:   return MTLPixelFormatBGRA8Unorm_sRGB;
            case Format::rg32_uint:          return MTLPixelFormatRG32Uint;
            case Format::rg32_sint:          return MTLPixelFormatRG32Sint;
            case Format::rg32_float:         return MTLPixelFormatRG32Float;
            case Format::rgba16_unorm:       return MTLPixelFormatRGBA16Unorm;
            case Format::rgba16_snorm:       return MTLPixelFormatRGBA16Snorm;
            case Format::rgba16_uint:        return MTLPixelFormatRGBA16Uint;
            case Format::rgba16_sint:        return MTLPixelFormatRGBA16Sint;
            case Format::rgba16_float:       return MTLPixelFormatRGBA16Float;
            case Format::rgba32_uint:        return MTLPixelFormatRGBA32Uint;
            case Format::rgba32_sint:        return MTLPixelFormatRGBA32Sint;
            case Format::rgba32_float:       return MTLPixelFormatRGBA32Float;

            case Format::b5g6r5_unorm:       return MTLPixelFormatB5G6R5Unorm;
            case Format::bgr5a1_unorm:       return MTLPixelFormatBGR5A1Unorm;

            case Format::rgb10a2_unorm:      return MTLPixelFormatRGB10A2Unorm;
            case Format::rgb10a2_uint:       return MTLPixelFormatRGB10A2Uint;
            case Format::rg11b10_float:      return MTLPixelFormatRG11B10Float;
            case Format::rgb9e5_float:       return MTLPixelFormatRGB9E5Float;

            case Format::d16_unorm:             return MTLPixelFormatDepth16Unorm;
            case Format::d32_float:             return MTLPixelFormatDepth32Float;
            case Format::d24_unorm_s8_uint:     return MTLPixelFormatDepth24Unorm_Stencil8;
            case Format::d32_float_s8_uint_x24: return MTLPixelFormatDepth32Float_Stencil8;

            case Format::bc1_rgba_unorm:         return MTLPixelFormatBC1_RGBA;
            case Format::bc1_rgba_unorm_srgb:    return MTLPixelFormatBC1_RGBA_sRGB;
            case Format::bc2_rgba_unorm:         return MTLPixelFormatBC2_RGBA;
            case Format::bc2_rgba_unorm_srgb:    return MTLPixelFormatBC2_RGBA_sRGB;
            case Format::bc3_rgba_unorm:         return MTLPixelFormatBC3_RGBA;
            case Format::bc3_rgba_unorm_srgb:    return MTLPixelFormatBC3_RGBA_sRGB;
            case Format::bc4_r_unorm:            return MTLPixelFormatBC4_RUnorm;
            case Format::bc4_r_snorm:            return MTLPixelFormatBC4_RSnorm;
            case Format::bc5_rg_unorm:           return MTLPixelFormatBC5_RGUnorm;
            case Format::bc5_rg_snorm:           return MTLPixelFormatBC5_RGSnorm;
            case Format::bc6h_rgb_sfloat:        return MTLPixelFormatBC6H_RGBFloat;
            case Format::bc6h_rgb_ufloat:        return MTLPixelFormatBC6H_RGBUfloat;
            case Format::bc7_rgba_unorm:         return MTLPixelFormatBC7_RGBAUnorm;
            case Format::bc7_rgba_unorm_srgb:    return MTLPixelFormatBC7_RGBAUnorm_sRGB;
            default:
                return MTLPixelFormatInvalid;
            }
        }
        inline Format decode_pixel_format(MTLPixelFormat format)
        {
            switch(format)
            {
            case MTLPixelFormatInvalid: return Format::unknown;
            case MTLPixelFormatR8Unorm: return Format::r8_unorm;
            case MTLPixelFormatR8Snorm: return Format::r8_snorm;
            case MTLPixelFormatR8Uint: return Format::r8_uint;
            case MTLPixelFormatR8Sint: return Format::r8_sint;

            case MTLPixelFormatR16Unorm: return Format::r16_unorm;     
            case MTLPixelFormatR16Snorm: return Format::r16_snorm;     
            case MTLPixelFormatR16Uint: return Format::r16_uint;     
            case MTLPixelFormatR16Sint: return Format::r16_sint;     
            case MTLPixelFormatR16Float: return Format::r16_float;     
            case MTLPixelFormatRG8Unorm: return Format::rg8_unorm;     
            case MTLPixelFormatRG8Snorm: return Format::rg8_snorm;     
            case MTLPixelFormatRG8Uint: return Format::rg8_uint;     
            case MTLPixelFormatRG8Sint: return Format::rg8_sint;     

            case MTLPixelFormatR32Uint: return Format::r32_uint;
            case MTLPixelFormatR32Sint: return Format::r32_sint;
            case MTLPixelFormatR32Float: return Format::r32_float;

            case MTLPixelFormatRG16Unorm: return Format::rg16_unorm;
            case MTLPixelFormatRG16Snorm: return Format::rg16_snorm;
            case MTLPixelFormatRG16Uint: return Format::rg16_uint;
            case MTLPixelFormatRG16Sint: return Format::rg16_sint;
            case MTLPixelFormatRG16Float: return Format::rg16_float;
            case MTLPixelFormatRGBA8Unorm: return Format::rgba8_unorm;
            case MTLPixelFormatRGBA8Unorm_sRGB: return Format::rgba8_unorm_srgb;
            case MTLPixelFormatRGBA8Snorm: return Format::rgba8_snorm;
            case MTLPixelFormatRGBA8Uint: return Format::rgba8_uint;
            case MTLPixelFormatRGBA8Sint: return Format::rgba8_sint;
            case MTLPixelFormatBGRA8Unorm: return Format::bgra8_unorm;
            case MTLPixelFormatBGRA8Unorm_sRGB: return Format::bgra8_unorm_srgb;
            case MTLPixelFormatRG32Uint: return Format::rg32_uint;
            case MTLPixelFormatRG32Sint: return Format::rg32_sint;
            case MTLPixelFormatRG32Float: return Format::rg32_float;
            case MTLPixelFormatRGBA16Unorm: return Format::rgba16_unorm;
            case MTLPixelFormatRGBA16Snorm: return Format::rgba16_snorm;
            case MTLPixelFormatRGBA16Uint: return Format::rgba16_uint;
            case MTLPixelFormatRGBA16Sint: return Format::rgba16_sint;
            case MTLPixelFormatRGBA16Float: return Format::rgba16_float;
            case MTLPixelFormatRGBA32Uint: return Format::rgba32_uint;
            case MTLPixelFormatRGBA32Sint: return Format::rgba32_sint;
            case MTLPixelFormatRGBA32Float: return Format::rgba32_float;

            case MTLPixelFormatB5G6R5Unorm: return Format::b5g6r5_unorm;
            case MTLPixelFormatBGR5A1Unorm: return Format::bgr5a1_unorm;

            case MTLPixelFormatRGB10A2Unorm: return Format::rgb10a2_unorm;
            case MTLPixelFormatRGB10A2Uint: return Format::rgb10a2_uint;
            case MTLPixelFormatRG11B10Float: return Format::rg11b10_float;
            case MTLPixelFormatRGB9E5Float: return Format::rgb9e5_float;

            case MTLPixelFormatDepth16Unorm: return Format::d16_unorm;
            case MTLPixelFormatDepth32Float: return Format::d32_float;
            case MTLPixelFormatDepth24Unorm_Stencil8: return Format::d24_unorm_s8_uint;
            case MTLPixelFormatDepth32Float_Stencil8: return Format::d32_float_s8_uint_x24;

            case MTLPixelFormatBC1_RGBA: return Format::bc1_rgba_unorm;
            case MTLPixelFormatBC1_RGBA_sRGB: return Format::bc1_rgba_unorm_srgb;
            case MTLPixelFormatBC2_RGBA: return Format::bc2_rgba_unorm;
            case MTLPixelFormatBC2_RGBA_sRGB: return Format::bc2_rgba_unorm_srgb;
            case MTLPixelFormatBC3_RGBA: return Format::bc3_rgba_unorm;
            case MTLPixelFormatBC3_RGBA_sRGB: return Format::bc3_rgba_unorm_srgb;
            case MTLPixelFormatBC4_RUnorm: return Format::bc4_r_unorm;
            case MTLPixelFormatBC4_RSnorm: return Format::bc4_r_snorm;
            case MTLPixelFormatBC5_RGUnorm: return Format::bc5_rg_unorm;
            case MTLPixelFormatBC5_RGSnorm: return Format::bc5_rg_snorm;
            case MTLPixelFormatBC6H_RGBFloat: return Format::bc6h_rgb_sfloat;
            case MTLPixelFormatBC6H_RGBUfloat: return Format::bc6h_rgb_ufloat;
            case MTLPixelFormatBC7_RGBAUnorm: return Format::bc7_rgba_unorm;
            case MTLPixelFormatBC7_RGBAUnorm_sRGB: return Format::bc7_rgba_unorm_srgb;
            default: return Format::unknown;
            }
        }
        inline MTLTextureType encode_texture_view_type(TextureViewType type)
        {
            switch(type)
            {
                case TextureViewType::tex1d: return MTLTextureType1D;
                case TextureViewType::tex2d: return MTLTextureType2D;
                case TextureViewType::tex2dms: return MTLTextureType2DMultisample;
                case TextureViewType::tex3d: return MTLTextureType3D;
                case TextureViewType::texcube: return MTLTextureTypeCube;
                case TextureViewType::tex1darray: return MTLTextureType1DArray;
                case TextureViewType::tex2darray: return MTLTextureType2DArray;
                case TextureViewType::tex2dmsarray: return MTLTextureType2DMultisampleArray;
                case TextureViewType::texcubearray: return MTLTextureTypeCubeArray;
                default: lupanic(); return MTLTextureType2D;
            }
        }
        inline MTLVertexFormat encode_vertex_format(Format f)
        {
            switch(f)
            {

            case Format::unknown: return MTLVertexFormatInvalid;
            case Format::r8_unorm: return MTLVertexFormatUCharNormalized;
            case Format::r8_snorm: return MTLVertexFormatCharNormalized;
            case Format::r8_uint: return MTLVertexFormatUChar;
            case Format::r8_sint: return MTLVertexFormatChar;

            case Format::r16_unorm: return MTLVertexFormatUShortNormalized;
            case Format::r16_snorm: return MTLVertexFormatShortNormalized;
            case Format::r16_uint: return MTLVertexFormatUShort;
            case Format::r16_sint: return MTLVertexFormatShort;
            case Format::r16_float: return MTLVertexFormatHalf;
            case Format::rg8_unorm: return MTLVertexFormatUChar2Normalized;
            case Format::rg8_snorm: return MTLVertexFormatChar2Normalized;
            case Format::rg8_uint: return MTLVertexFormatUChar2;
            case Format::rg8_sint: return MTLVertexFormatChar2;

            case Format::r32_uint: return MTLVertexFormatUInt;
            case Format::r32_sint: return MTLVertexFormatInt;
            case Format::r32_float: return MTLVertexFormatFloat;

            case Format::rg16_unorm: return MTLVertexFormatUShort2Normalized;
            case Format::rg16_snorm: return MTLVertexFormatShort2Normalized;
            case Format::rg16_uint: return MTLVertexFormatUShort2;
            case Format::rg16_sint: return MTLVertexFormatShort2;
            case Format::rg16_float: return MTLVertexFormatHalf2;
            case Format::rgba8_unorm: return MTLVertexFormatUChar4Normalized;
            case Format::rgba8_snorm: return MTLVertexFormatChar4Normalized;
            case Format::rgba8_uint: return MTLVertexFormatUChar4;
            case Format::rgba8_sint: return MTLVertexFormatChar4;
            case Format::rg32_uint: return MTLVertexFormatUInt2;
            case Format::rg32_sint: return MTLVertexFormatInt2;
            case Format::rg32_float: return MTLVertexFormatFloat2;
            case Format::rgba16_unorm: return MTLVertexFormatUShort4Normalized;
            case Format::rgba16_snorm: return MTLVertexFormatShort4Normalized;
            case Format::rgba16_uint: return MTLVertexFormatUShort4;
            case Format::rgba16_sint: return MTLVertexFormatShort4;
            case Format::rgba16_float: return MTLVertexFormatHalf4;
            case Format::rgb32_uint: return MTLVertexFormatUInt3;
            case Format::rgb32_sint: return MTLVertexFormatInt3;
            case Format::rgb32_float: return MTLVertexFormatFloat3;
            case Format::rgba32_uint: return MTLVertexFormatUInt4;
            case Format::rgba32_sint: return MTLVertexFormatInt4;
            case Format::rgba32_float: return MTLVertexFormatFloat4;
            
            default:
                return MTLVertexFormatInvalid;
            }
        }

        inline MTLStorageMode encode_storage_mode(MemoryType type)
        {
            switch(type)
            {
                case MemoryType::local: return MTLStorageModePrivate;
                case MemoryType::upload: return MTLStorageModeShared;
                case MemoryType::readback: return MTLStorageModeShared;
            }
        }
        inline MTLCPUCacheMode encode_cpu_cache_mode(MemoryType type)
        {
            switch(type)
            {
                case MemoryType::local: return MTLCPUCacheModeDefaultCache;
                case MemoryType::upload: return MTLCPUCacheModeWriteCombined;
                case MemoryType::readback: return MTLCPUCacheModeDefaultCache;
            }
        }

        struct MTLBufferDesc
        {
            NSInteger length;
            MTLResourceOptions options;
        };

        inline MTLResourceOptions encode_resource_options(MemoryType memory_type)
        {
            MTLResourceOptions options = 0;
            if(memory_type == MemoryType::local)
            {
                options |= MTLResourceStorageModePrivate;
            }
            else if(memory_type == MemoryType::upload)
            {
                options |= MTLResourceCPUCacheModeWriteCombined;
            }
            options |= MTLResourceHazardTrackingModeTracked;
            return options;
        }

        inline MTLTextureUsage encode_texture_usage(TextureUsageFlag usages)
        {
            MTLTextureUsage ret = 0;
            if(test_flags(usages, TextureUsageFlag::read_texture))
            {
                ret |= MTLTextureUsageShaderRead;
            }
            if(test_flags(usages, TextureUsageFlag::read_write_texture))
            {
                ret |= MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
            }
            if(test_flags(usages, TextureUsageFlag::color_attachment) ||
                test_flags(usages, TextureUsageFlag::depth_stencil_attachment))
            {
                ret |= MTLTextureUsageRenderTarget;
            }
            return ret;
        }

        inline TextureUsageFlag decode_texture_usage(MTLTextureUsage usages, bool is_depth_stencil_format)
        {
            TextureUsageFlag ret = TextureUsageFlag::none;
            if(usages & MTLTextureUsageShaderRead)
            {
                ret |= TextureUsageFlag::read_texture;
            }
            if(usages & MTLTextureUsageShaderWrite)
            {
                ret |= TextureUsageFlag::read_write_texture;
            }
            if(usages & MTLTextureUsageRenderTarget)
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

        inline MTLTextureDescriptor* encode_texture_desc(MemoryType memory_type, const TextureDesc& desc)
        {
            MTLTextureDescriptor* ret = [[MTLTextureDescriptor alloc] init];
            if(desc.type == TextureType::tex1d)
            {
                ret.textureType = desc.array_size == 1 ? MTLTextureType1D : MTLTextureType1DArray;
            }
            else if(desc.type == TextureType::tex2d)
            {
                if(desc.sample_count == 1)
                {
                    if(test_flags(desc.usages, TextureUsageFlag::cube))
                    {
                        ret.textureType = desc.array_size == 6 ? MTLTextureTypeCube : MTLTextureTypeCubeArray;
                    }
                    else
                    {
                        ret.textureType = desc.array_size == 1 ? MTLTextureType2D : MTLTextureType2DArray;
                    }
                }
                else
                {
                    ret.textureType = desc.array_size == 1 ? MTLTextureType2DMultisample : MTLTextureType2DMultisampleArray;
                }
            }
            else if(desc.type == TextureType::tex3d)
            {
                ret.textureType = MTLTextureType3D;
            }
            ret.pixelFormat = encode_pixel_format(desc.format);
            ret.width = desc.width;
            ret.height = desc.height;
            ret.depth = desc.depth;
            ret.mipmapLevelCount = desc.mip_levels;
            ret.sampleCount = desc.sample_count;
            if(test_flags(desc.usages, TextureUsageFlag::cube))
            {
                ret.arrayLength = desc.array_size / 6;
            }
            else
            {
                ret.arrayLength = desc.array_size;
            }
            ret.resourceOptions = encode_resource_options(memory_type);
            ret.cpuCacheMode = encode_cpu_cache_mode(memory_type);
            ret.storageMode = encode_storage_mode(memory_type);
            ret.usage = encode_texture_usage(desc.usages);
            ret.hazardTrackingMode = MTLHazardTrackingModeTracked;
            return ret;
        }
        inline MTLCompareFunction encode_compare_function(CompareFunction func)
        {
            switch(func)
            {
                case CompareFunction::never: return MTLCompareFunctionNever;
                case CompareFunction::less: return MTLCompareFunctionLess;
                case CompareFunction::equal: return MTLCompareFunctionEqual;
                case CompareFunction::less_equal: return MTLCompareFunctionLessEqual;
                case CompareFunction::greater: return MTLCompareFunctionGreater;
                case CompareFunction::not_equal: return MTLCompareFunctionNotEqual;
                case CompareFunction::greater_equal: return MTLCompareFunctionGreaterEqual;
                case CompareFunction::always: return MTLCompareFunctionAlways;
            }
            lupanic();
            return MTLCompareFunctionNever;
        }
        inline MTLSamplerMinMagFilter encode_min_mag_filter(Filter filter)
        {
            switch(filter)
            {
                case Filter::nearest: return MTLSamplerMinMagFilterNearest;
                case Filter::linear: return MTLSamplerMinMagFilterLinear;
            }
            lupanic();
            return MTLSamplerMinMagFilterNearest;
        }
        inline MTLSamplerMipFilter encode_mip_filter(Filter filter)
        {
            switch(filter)
            {
                case Filter::nearest: return MTLSamplerMipFilterNearest;
                case Filter::linear: return MTLSamplerMipFilterLinear;
            }
            lupanic();
            return MTLSamplerMipFilterNearest;
        }
        inline MTLSamplerAddressMode encode_address_mode(TextureAddressMode mode)
        {
            switch(mode)
            {
                case TextureAddressMode::repeat: return MTLSamplerAddressModeRepeat;
                case TextureAddressMode::mirror: return MTLSamplerAddressModeMirrorRepeat;
                case TextureAddressMode::clamp: return MTLSamplerAddressModeClampToEdge;
                case TextureAddressMode::border: return MTLSamplerAddressModeClampToBorderColor;
            }
            lupanic();
            return MTLSamplerAddressModeRepeat;
        }
        inline MTLBlendOperation encode_blend_op(BlendOp op)
        {
            switch(op)
            {
                case BlendOp::add: return MTLBlendOperationAdd;
                case BlendOp::subtract: return MTLBlendOperationSubtract;
                case BlendOp::rev_subtract: return MTLBlendOperationReverseSubtract;
                case BlendOp::min: return MTLBlendOperationMin;
                case BlendOp::max: return MTLBlendOperationMax;
            }
            lupanic();
            return MTLBlendOperationAdd;
        }
        inline MTLBlendFactor encode_blend_factor(BlendFactor factor, bool is_rgb)
        {
            switch(factor)
            {
                case BlendFactor::zero: return MTLBlendFactorZero;
                case BlendFactor::one: return MTLBlendFactorOne;
                case BlendFactor::src_color: return MTLBlendFactorSourceColor;
                case BlendFactor::one_minus_src_color: return MTLBlendFactorOneMinusSourceColor;
                case BlendFactor::src_alpha: return MTLBlendFactorSourceAlpha;
                case BlendFactor::one_minus_src_alpha: return MTLBlendFactorOneMinusSourceAlpha;
                case BlendFactor::dst_color: return MTLBlendFactorDestinationColor;
                case BlendFactor::one_minus_dst_color: return MTLBlendFactorOneMinusDestinationColor;
                case BlendFactor::dst_alpha: return MTLBlendFactorDestinationAlpha;
                case BlendFactor::one_minus_dst_alpha: return MTLBlendFactorOneMinusDestinationAlpha;
                case BlendFactor::src_alpha_saturated: return MTLBlendFactorSourceAlphaSaturated;
                case BlendFactor::blend_factor: return is_rgb ? MTLBlendFactorBlendColor : MTLBlendFactorBlendAlpha;
                case BlendFactor::one_minus_blend_factor: return is_rgb ? MTLBlendFactorOneMinusBlendColor : MTLBlendFactorOneMinusBlendAlpha;
                case BlendFactor::src1_color: return MTLBlendFactorSource1Color;
                case BlendFactor::one_minus_src1_color: return MTLBlendFactorOneMinusSource1Color;
                case BlendFactor::src1_alpha: return MTLBlendFactorSource1Alpha;
                case BlendFactor::one_minus_src1_alpha: return MTLBlendFactorOneMinusSource1Alpha;
            }
            lupanic();
            return MTLBlendFactorZero;
        }
        inline MTLPrimitiveTopologyClass encode_primitive_topology(PrimitiveTopology topology)
        {
            switch(topology)
            {
                case PrimitiveTopology::point_list: return MTLPrimitiveTopologyClassPoint;
                case PrimitiveTopology::line_list:
                case PrimitiveTopology::line_strip: return MTLPrimitiveTopologyClassLine;
                case PrimitiveTopology::triangle_list:
                case PrimitiveTopology::triangle_strip: return MTLPrimitiveTopologyClassTriangle;
            }
            lupanic();
            return MTLPrimitiveTopologyClassUnspecified;
        }
        inline MTLLoadAction encode_load_action(LoadOp op)
        {
            switch(op)
            {
                case LoadOp::dont_care: return MTLLoadActionDontCare;
                case LoadOp::load: return MTLLoadActionLoad;
                case LoadOp::clear: return MTLLoadActionClear;
            }
            lupanic();
            return MTLLoadActionDontCare;
        }
        inline MTLStoreAction encode_store_action(StoreOp op, bool resolve)
        {
            switch(op)
            {
                case StoreOp::dont_care: return resolve ? MTLStoreActionMultisampleResolve : MTLStoreActionDontCare;
                case StoreOp::store: return resolve ? MTLStoreActionStoreAndMultisampleResolve : MTLStoreActionStore;
            }
            lupanic();
            return MTLStoreActionDontCare;
        }
        inline MTLStencilOperation encode_stencil_operation(StencilOp op)
        {
            switch(op)
            {
                case StencilOp::keep: return MTLStencilOperationKeep;
                case StencilOp::zero: return MTLStencilOperationZero;
                case StencilOp::replace: return MTLStencilOperationReplace;
                case StencilOp::increment_saturated: return MTLStencilOperationIncrementClamp;
                case StencilOp::decrement_saturated: return MTLStencilOperationDecrementClamp;
                case StencilOp::invert: return MTLStencilOperationInvert;
                case StencilOp::increment: return MTLStencilOperationIncrementWrap;
                case StencilOp::decrement: return MTLStencilOperationDecrementWrap;
            }
            lupanic();
            return MTLStencilOperationKeep;
        }
        inline MTLPrimitiveType encode_primitive_type(PrimitiveTopology pt)
        {
            switch(pt)
            {
                case PrimitiveTopology::point_list: return MTLPrimitiveTypePoint;
                case PrimitiveTopology::line_list: return MTLPrimitiveTypeLine;
                case PrimitiveTopology::line_strip: return MTLPrimitiveTypeLineStrip;
                case PrimitiveTopology::triangle_list: return MTLPrimitiveTypeTriangle;
                case PrimitiveTopology::triangle_strip: return MTLPrimitiveTypeTriangleStrip;
            }
            lupanic();
            return MTLPrimitiveTypePoint;
        }
        inline MTLIndexType encode_index_type(Format format)
        {
            switch(format)
            {
                case Format::r16_uint:
                case Format::r16_sint: return MTLIndexTypeUInt16;
                case Format::r32_uint:
                case Format::r32_sint: return MTLIndexTypeUInt32;
                default: break;
            }
            lupanic();
            return MTLIndexTypeUInt32;
        }
        inline MTLRenderStages encode_render_stage(BufferStateFlag flags)
        {
            MTLRenderStages r = 0;
            if(test_flags(flags, BufferStateFlag::shader_read_vs) || test_flags(flags, BufferStateFlag::uniform_buffer_vs))
            {
                r |= MTLRenderStageVertex;
            }
            if(test_flags(flags, BufferStateFlag::shader_read_ps) || test_flags(flags, BufferStateFlag::uniform_buffer_ps))
            {
                r |= MTLRenderStageFragment;
            }
            return r;
        }
        inline MTLRenderStages encode_render_stage(TextureStateFlag flags)
        {
            MTLRenderStages r = 0;
            if(test_flags(flags, TextureStateFlag::shader_read_vs))
            {
                r |= MTLRenderStageVertex;
            }
            if(test_flags(flags, TextureStateFlag::shader_read_ps))
            {
                r |= MTLRenderStageFragment;
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
