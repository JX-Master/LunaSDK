/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TextureAsset.cpp
* @author JXMaster
* @date 2020/5/9
*/
#pragma once
#include "Texture.hpp"
#include <RHI/RHI.hpp>
#include <Runtime/File.hpp>
#include <Image/Image.hpp>
#include <VFS/VFS.hpp>

namespace Luna
{
	struct TextureAssetUserdata
	{
		lustruct("TextureAssetUserdata", "{816CDA20-AB1C-4E24-A7CE-59E2EFE9BE1E}");

		Ref<RHI::IDescriptorSetLayout> m_mipmapping_dlayout;
		Ref<RHI::IShaderInputLayout> m_mipmapping_slayout;
		Ref<RHI::IPipelineState> m_mipmapping_pso;

		static constexpr u32 ENV_MAP_MIPS_COUNT = 5;

		RV init();

		RV generate_mipmaps(RHI::IResource* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf);
	};
	RV TextureAssetUserdata::init()
	{
		using namespace RHI;
		lutry
		{
			{
				DescriptorSetLayoutDesc desc;
				desc.bindings = {
					DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::all),
					DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibility::all),
					DescriptorSetLayoutBinding(DescriptorType::uav, 2, 1, ShaderVisibility::all),
					DescriptorSetLayoutBinding(DescriptorType::sampler, 3, 1, ShaderVisibility::all)
				};
				luset(m_mipmapping_dlayout, RHI::get_main_device()->new_descriptor_set_layout(desc));

				luset(m_mipmapping_slayout, RHI::get_main_device()->new_shader_input_layout(ShaderInputLayoutDesc(
					{ m_mipmapping_dlayout },
					ShaderInputLayoutFlag::deny_vertex_shader_access |
					ShaderInputLayoutFlag::deny_domain_shader_access |
					ShaderInputLayoutFlag::deny_geometry_shader_access |
					ShaderInputLayoutFlag::deny_hull_shader_access |
					ShaderInputLayoutFlag::deny_pixel_shader_access)));

				lulet(psf, open_file("MipmapGenerationCS.cso", FileOpenFlag::read, FileCreationMode::open_existing));
				auto file_size = psf->get_size();
				auto cs_blob = Blob((usize)file_size);
				luexp(psf->read(cs_blob.span()));
				psf = nullptr;
				ComputePipelineStateDesc ps_desc;
				ps_desc.shader_input_layout = m_mipmapping_slayout;
				ps_desc.cs = cs_blob.cspan();
				luset(m_mipmapping_pso, RHI::get_main_device()->new_compute_pipeline_state(ps_desc));
			}
		}
		lucatchret;
		return ok;
	}
	RV TextureAssetUserdata::generate_mipmaps(RHI::IResource* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf)
	{
		using namespace RHI;
		lutry
		{
			auto desc = resource_with_most_detailed_mip->get_desc();
			lucheck(desc.mip_levels);
			lucheck(desc.type == ResourceType::texture_2d);
			lucheck(desc.depth_or_array_size == 1);

			if (desc.mip_levels == 1)
			{
				return ok;
			}

			auto device = RHI::get_main_device();

			compute_cmdbuf->set_compute_shader_input_layout(m_mipmapping_slayout);
			compute_cmdbuf->set_pipeline_state(m_mipmapping_pso);
			u32 cb_align = device->get_constant_buffer_data_alignment();
			u32 cb_size = (u32)align_upper(sizeof(Float2), cb_align);
			lulet(cb, device->new_resource(
				ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::constant_buffer, cb_size * (desc.mip_levels - 1))));

			void* mapped = nullptr;
			luexp(cb->map_subresource(0, 0, 0, &mapped));
			for (u32 j = 0; j < (u32)(desc.mip_levels - 1); ++j)
			{
				u32 width = max<u32>((u32)desc.width_or_buffer_size >> (j + 1), 1);
				u32 height = max<u32>(desc.height >> (j + 1), 1);
				Float2U* dest = (Float2U*)((usize)mapped + cb_size * j);
				dest->x = 1.0f / (f32)width;
				dest->y = 1.0f / (f32)height;
			}
			cb->unmap_subresource(0, 0, USIZE_MAX);

			u32 width = (u32)desc.width_or_buffer_size / 2;
			u32 height = desc.height / 2;

			for (u32 j = 0; j < (u32)(desc.mip_levels - 1); ++j)
			{
				u32 src_subresource = RHI::calc_subresource_index(j, 0, desc.mip_levels);
				u32 dest_subresource = RHI::calc_subresource_index(j + 1, 0, desc.mip_levels);
				ResourceBarrierDesc barriers[] = {
					ResourceBarrierDesc::as_transition(resource_with_most_detailed_mip, ResourceStateFlag::shader_resource_non_pixel, src_subresource),
					ResourceBarrierDesc::as_transition(resource_with_most_detailed_mip, ResourceStateFlag::unordered_access, dest_subresource)
				};
				compute_cmdbuf->resource_barriers({ barriers, 2 });
				lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_mipmapping_dlayout)));
				vs->set_cbv(0, cb, ConstantBufferViewDesc(cb_size * j, cb_size));
				vs->set_srv(1, resource_with_most_detailed_mip, &ShaderResourceViewDesc::as_tex2d(desc.pixel_format, src_subresource, 1, 0.0f));
				vs->set_uav(2, resource_with_most_detailed_mip, nullptr, &UnorderedAccessViewDesc::as_tex2d(desc.pixel_format, dest_subresource));
				vs->set_sampler(3, SamplerDesc(FilterMode::min_mag_mip_linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp));
				compute_cmdbuf->set_compute_descriptor_set(0, vs);
				compute_cmdbuf->attach_device_object(vs);
				compute_cmdbuf->dispatch(align_upper(width, 8) / 8, align_upper(height, 8) / 8, 1);
				width = max<u32>(width / 2, 1);
				height = max<u32>(height / 2, 1);
			}

			compute_cmdbuf->resource_barrier(ResourceBarrierDesc::as_transition(resource_with_most_detailed_mip, ResourceStateFlag::common));

			luexp(compute_cmdbuf->submit());
			compute_cmdbuf->wait();
			luexp(compute_cmdbuf->reset());
		}
		lucatchret;
		return ok;
	}
	static R<ObjRef> load_texture_asset(object_t userdata, Asset::asset_t asset, const Path& path)
	{
		ObjRef ret;
		lutry
		{
			// Open image file.
			Path file_path = path;
			file_path.append_extension("tex");
			lulet(file, VFS::open_file(file_path, FileOpenFlag::read, FileCreationMode::open_existing));
			lulet(file_data, load_file_data(file));
			// Check whether the texture contains mips.
			if (file_data.size() >= 8 && !memcmp((const c8*)file_data.data(), "LUNAMIPS", 8))
			{
				u64* dp = (u64*)(file_data.data() + 8);
				u64 num_mips = *dp;
				++dp;
				Vector<Pair<u64, u64>> mip_descs;
				mip_descs.reserve(num_mips);
				for (u64 i = 0; i < num_mips; ++i)
				{
					Pair<u64, u64> p;
					p.first = dp[i * 2];
					p.second = dp[i * 2 + 1];
					mip_descs.push_back(p);
				}
				// Load texture from file.
				lulet(desc, Image::read_image_file_desc(file_data.data() + mip_descs[0].first, mip_descs[0].second));
				auto desired_format = get_desired_format(desc.format);
				// Create resource.
				lulet(tex, RHI::get_main_device()->new_resource(RHI::ResourceDesc::tex2d(RHI::ResourceHeapType::local,
					get_pixel_format_from_image_format(desc.format), RHI::ResourceUsageFlag::shader_resource | RHI::ResourceUsageFlag::unordered_access, desc.width, desc.height)));
				// Upload data
				Vector<RHI::ResourceCopyDesc> copies;
				Vector<Blob> data;
				copies.reserve(num_mips);
				data.reserve(num_mips);
				for (u64 i = 0; i < num_mips; ++i)
				{
					Image::ImageDesc desc;
					lulet(image_data, Image::read_image_file(file_data.data() + mip_descs[i].first, mip_descs[i].second, desired_format, desc));
					data.push_back(move(image_data));
					RHI::ResourceCopyDesc copy = RHI::ResourceCopyDesc::as_write_texture(tex, data[i].data(), pixel_size(desc.format) * desc.width, pixel_size(desc.format) * desc.width * desc.height, i,
						BoxU(0, 0, 0, desc.width, desc.height, 1));
					copies.push_back(copy);
				}
				luexp(RHI::get_main_device()->copy_resource({ copies.data(), copies.size() }));
				ret = tex;
			}
			else
			{
				// Load texture from file.
				lulet(desc, Image::read_image_file_desc(file_data.data(), file_data.size()));
				auto desired_format = get_desired_format(desc.format);
				lulet(image_data, Image::read_image_file(file_data.data(), file_data.size(), desired_format, desc));
				// Create resource.
				lulet(tex, RHI::get_main_device()->new_resource(RHI::ResourceDesc::tex2d(RHI::ResourceHeapType::local,
					get_pixel_format_from_image_format(desc.format), RHI::ResourceUsageFlag::shader_resource | RHI::ResourceUsageFlag::unordered_access, desc.width, desc.height)));
				// Upload data.
				luexp(RHI::get_main_device()->copy_resource({
					RHI::ResourceCopyDesc::as_write_texture(tex, image_data.data(),
						pixel_size(desc.format) * desc.width, pixel_size(desc.format) * desc.width * desc.height,
						0, BoxU(0, 0, 0, desc.width, desc.height, 1))
					}));
				// Generate mipmaps.
				Ref<TextureAssetUserdata> ctx = ObjRef(userdata);
				lulet(cmdbuf, g_env->async_compute_queue->new_command_buffer());
				luexp(ctx->generate_mipmaps(tex, cmdbuf));
				ret = tex;
			}
		}
		lucatchret;
		return ret;
	}
	RV register_static_texture_asset_type()
	{
		lutry
		{
			register_boxed_type<TextureAssetUserdata>();
			Ref<TextureAssetUserdata> userdata = new_object<TextureAssetUserdata>();
			luexp(userdata->init());
			Asset::AssetTypeDesc desc;
			desc.name = get_static_texture_asset_type();
			desc.on_load_asset = load_texture_asset;
			desc.on_save_asset = nullptr;
			desc.on_set_asset_data = nullptr;
			desc.userdata = userdata;
			Asset::register_asset_type(desc);
		}
		lucatchret;
		return ok;
	}
	Name get_static_texture_asset_type()
	{
		return "Static Texture";
	}
}