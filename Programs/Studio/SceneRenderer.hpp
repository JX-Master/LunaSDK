/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SceneRenderer.hpp
* @author JXMaster
* @date 2022/12/17
*/
#pragma once
#include <Runtime/TypeInfo.hpp>
#include <RHI/RHI.hpp>
#include <Asset/Asset.hpp>
namespace Luna
{
	struct Entity;
	struct SceneRenderer
	{
		lustruct("SceneRenderer", "{CE0188A0-C1A6-421E-A60C-8D4F260972A3}");

		Ref<RHI::IResource> depth_buffer;	// D32_FLOAT
		Ref<RHI::IDepthStencilView> depth_buffer_dsv;

		Ref<RHI::IResource> screen_buffer;	// RGBA8_UNORM
		Ref<RHI::IRenderTargetView> screen_buffer_rtv;

		Ref<RHI::IResource> lighting_buffer; // RGB32_FLOAT
		Ref<RHI::IRenderTargetView> lighting_buffer_rtv;

		Ref<RHI::IResource> lighting_accms[11];		// R32_FLOAT from 1024.v

		Name camera_entity;

		Asset::asset_t skybox;

		Float3 environment_color = Float3(0.0f, 0.0f, 0.0f);

		f32 skybox_rotation = 0.0f;
		f32 exposure = 1.0f;

		RV resize_screen_buffer(UInt2U size)
		{
			lutry
			{
				using namespace RHI;

				auto device = get_main_device();

				auto desc = screen_buffer->get_desc();
				desc.width_or_buffer_size = size.x;
				desc.height = size.y;
				f32 clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
				luset(screen_buffer, device->new_resource(desc, &ClearValue::as_color(Format::rgba8_unorm, clear_color)));
				luset(screen_buffer_rtv, device->new_render_target_view(screen_buffer));

				desc = depth_buffer->get_desc();
				desc.width_or_buffer_size = size.x;
				desc.height = size.y;
				luset(depth_buffer, device->new_resource(desc, &ClearValue::as_depth_stencil(Format::d32_float, 1.0f, 0xFF)));
				luset(depth_buffer_dsv, device->new_depth_stencil_view(depth_buffer));

				desc = lighting_buffer->get_desc();
				desc.width_or_buffer_size = size.x;
				desc.height = size.y;
				luset(lighting_buffer, device->new_resource(desc, &ClearValue::as_color(Format::rgba32_float, clear_color)));
				luset(lighting_buffer_rtv, device->new_render_target_view(lighting_buffer));
			}
			lucatchret;
			return ok;
		}
		RV init(const Float2U& framebuffer_size)
		{
			lutry
			{
				using namespace RHI;
				auto device = get_main_device();
				f32 clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
				luset(screen_buffer, device->new_resource(ResourceDesc::tex2d(
					ResourceHeapType::local,
					Format::rgba8_unorm,
					ResourceUsageFlag::render_target | ResourceUsageFlag::unordered_access, framebuffer_size.x, framebuffer_size.y, 1, 1),
					&ClearValue::as_color(Format::rgba8_unorm, clear_color)));
				luset(screen_buffer_rtv, device->new_render_target_view(screen_buffer));

				luset(depth_buffer, device->new_resource(ResourceDesc::tex2d(
					ResourceHeapType::local,
					Format::d32_float,
					ResourceUsageFlag::depth_stencil,
					framebuffer_size.x, framebuffer_size.y, 1, 1), &ClearValue::as_depth_stencil(Format::d32_float, 1.0f, 0xFF)));
				luset(depth_buffer_dsv, device->new_depth_stencil_view(depth_buffer));

				luset(lighting_buffer, device->new_resource(ResourceDesc::tex2d(
					ResourceHeapType::local,
					Format::rgba32_float,
					ResourceUsageFlag::render_target | ResourceUsageFlag::shader_resource | ResourceUsageFlag::unordered_access,
					framebuffer_size.x, framebuffer_size.y, 1, 1), &ClearValue::as_color(Format::rgba32_float, clear_color)));
				luset(lighting_buffer_rtv, device->new_render_target_view(lighting_buffer));

				u32 sz = 1024;
				for (u32 i = 0; i < 11; ++i)
				{
					luset(lighting_accms[i], device->new_resource(ResourceDesc::tex2d(
						ResourceHeapType::local,
						Format::r32_float,
						ResourceUsageFlag::unordered_access | ResourceUsageFlag::shader_resource,
						sz, sz, 1, 1)));

					sz >>= 1;
				}
			}
			lucatchret;
			return ok;
		}
	};
}