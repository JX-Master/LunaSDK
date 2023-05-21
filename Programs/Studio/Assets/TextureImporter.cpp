/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TextureImporter.cpp
* @author JXMaster
* @date 2020/5/8
*/
#include "Texture.hpp"
#include <Window/FileDialog.hpp>
#include <Window/MessageBox.hpp>
#include <Runtime/File.hpp>
#include <VFS/VFS.hpp>
namespace Luna
{
	enum class TexturePrefilerType : u8
	{
		normal = 0,
		environment_map = 1,
	};

	struct TextureFile
	{
		Path m_path;
		Image::ImageDesc m_desc;
		String m_asset_name;
		Blob m_file_data;
		TexturePrefilerType m_prefiler_type;
	};

	struct TextureImporter : public IAssetEditor
	{
		lustruct("TextureImporter", "{29488656-e1e3-4e7d-b772-2cf93308ba8b}");
		luiimpl();

		Path m_create_dir;

		Vector<TextureFile> m_files;

		Ref<RHI::IDescriptorSetLayout> m_mipmapping_dlayout;
		Ref<RHI::IShaderInputLayout> m_mipmapping_slayout;
		Ref<RHI::IPipelineState> m_mipmapping_pso;

		Ref<RHI::IDescriptorSetLayout> m_env_mipmapping_dlayout;
		Ref<RHI::IShaderInputLayout> m_env_mipmapping_slayout;
		Ref<RHI::IPipelineState> m_env_mipmapping_pso;

		static constexpr u32 ENV_MAP_MIPS = 5;

		RV init();
		void import_texture_asset(const Path& create_dir, const TextureFile& file);
		RV generate_mipmaps(RHI::IResource* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf);
		R<Ref<RHI::IResource>> generate_environment_mipmaps(RHI::IResource* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf);

		bool m_open;

		TextureImporter() :
			m_open(true) {}

		virtual void on_render() override;
		virtual bool closed() override
		{
			return !m_open;
		}
	};

	RV TextureImporter::init()
	{
		using namespace RHI;
		lutry
		{
			{
				DescriptorSetLayoutDesc desc;
				desc.bindings = {
					DescriptorSetLayoutBinding(DescriptorType::uniform_buffer_view, 0, 1, ShaderVisibilityFlag::all),
					DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibilityFlag::all),
					DescriptorSetLayoutBinding(DescriptorType::uav, 2, 1, ShaderVisibilityFlag::all),
					DescriptorSetLayoutBinding(DescriptorType::sampler, 3, 1, ShaderVisibilityFlag::all)
				};
				luset(m_mipmapping_dlayout, RHI::get_main_device()->new_descriptor_set_layout(desc));

				luset(m_mipmapping_slayout, RHI::get_main_device()->new_shader_input_layout(ShaderInputLayoutDesc(
					{ m_mipmapping_dlayout },
					ShaderInputLayoutFlag::deny_vertex_shader_access |
					
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
			{
				DescriptorSetLayoutDesc desc;
				desc.bindings = {
					DescriptorSetLayoutBinding(DescriptorType::uniform_buffer_view, 0, 1, ShaderVisibilityFlag::all),
					DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibilityFlag::all),
					DescriptorSetLayoutBinding(DescriptorType::uav, 2, 1, ShaderVisibilityFlag::all),
					DescriptorSetLayoutBinding(DescriptorType::sampler, 3, 1, ShaderVisibilityFlag::all)
				};
				luset(m_env_mipmapping_dlayout, RHI::get_main_device()->new_descriptor_set_layout(desc));

				luset(m_env_mipmapping_slayout, RHI::get_main_device()->new_shader_input_layout(ShaderInputLayoutDesc(
					{ m_env_mipmapping_dlayout },
					ShaderInputLayoutFlag::deny_vertex_shader_access |
					
					ShaderInputLayoutFlag::deny_pixel_shader_access)));

				lulet(psf, open_file("PrecomputeEnvironmentMapMips.cso", FileOpenFlag::read, FileCreationMode::open_existing));
				auto file_size = psf->get_size();
				auto cs_blob = Blob((usize)file_size);
				luexp(psf->read(cs_blob.span()));
				psf = nullptr;
				ComputePipelineStateDesc ps_desc;
				ps_desc.shader_input_layout = m_env_mipmapping_slayout;
				ps_desc.cs = cs_blob.cspan();
				luset(m_env_mipmapping_pso, RHI::get_main_device()->new_compute_pipeline_state(ps_desc));
			}
		}
		lucatchret;
		return ok;
	}

	RV TextureImporter::generate_mipmaps(RHI::IResource* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf)
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

			if (!m_mipmapping_slayout)
			{
				luexp(init());
			}

			compute_cmdbuf->set_compute_shader_input_layout(m_mipmapping_slayout);
			compute_cmdbuf->set_pipeline_state(m_mipmapping_pso);
			u32 cb_align = device->get_uniform_buffer_data_alignment();
			u32 cb_size = (u32)align_upper(sizeof(Float2), cb_align);
			lulet(cb, device->new_resource(
				ResourceDesc::buffer(MemoryType::upload, BufferUsageFlag::uniform_buffer, cb_size * (desc.mip_levels - 1))));

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
				vs->set_srv(1, resource_with_most_detailed_mip, &ShaderResourceViewDesc::tex2d(desc.pixel_format, src_subresource, 1, 0.0f));
				vs->set_uav(2, resource_with_most_detailed_mip, nullptr, &UnorderedAccessViewDesc::tex2d(desc.pixel_format, dest_subresource));
				vs->set_sampler(3, SamplerDesc(Filter::min_mag_mip_linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp));
				compute_cmdbuf->set_compute_descriptor_set(0, vs);
				compute_cmdbuf->attach_device_object(vs);
				u32 width = max<u32>((u32)desc.width_or_buffer_size >> (j + 1), 1);
				u32 height = max<u32>(desc.height >> (j + 1), 1);
				compute_cmdbuf->dispatch(align_upper(width, 8) / 8, align_upper(height, 8) / 8, 1);
			}

			compute_cmdbuf->resource_barrier(ResourceBarrierDesc::as_transition(resource_with_most_detailed_mip, ResourceStateFlag::common));

			luexp(compute_cmdbuf->submit());
			compute_cmdbuf->wait();
			luexp(compute_cmdbuf->reset());
		}
		lucatchret;
		return ok;
	}

	R<Ref<RHI::IResource>> TextureImporter::generate_environment_mipmaps(RHI::IResource* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf)
	{
		using namespace RHI;
		Ref<RHI::IResource> prefiltered;
		lutry
		{
			auto desc = resource_with_most_detailed_mip->get_desc();
			lucheck(desc.mip_levels);
			lucheck(desc.type == ResourceType::texture_2d);
			lucheck(desc.depth_or_array_size == 1);

			auto device = RHI::get_main_device();

			if (!m_mipmapping_slayout)
			{
				luexp(init());
			}

			desc.mip_levels = ENV_MAP_MIPS;
			luset(prefiltered, device->new_resource(desc));
			desc = prefiltered->get_desc();

			compute_cmdbuf->set_compute_shader_input_layout(m_env_mipmapping_slayout);
			compute_cmdbuf->set_pipeline_state(m_env_mipmapping_pso);
			struct CB
			{
				u32 tex_width;
				u32 tex_height;
				u32 mip_0_width;
				u32 mip_0_height;
				f32 roughness;
			};
			u32 cb_align = device->get_uniform_buffer_data_alignment();
			u32 cb_size = (u32)align_upper(sizeof(CB), cb_align);
			lulet(cb, device->new_resource(
				ResourceDesc::buffer(MemoryType::upload, BufferUsageFlag::uniform_buffer, cb_size * (desc.mip_levels - 1))));

			void* mapped = nullptr;
			luexp(cb->map_subresource(0, 0, 0, &mapped));
			for (u32 j = 0; j < (u32)(desc.mip_levels - 1); ++j)
			{
				u32 width = max<u32>((u32)desc.width_or_buffer_size >> (j + 1), 1);
				u32 height = max<u32>(desc.height >> (j + 1), 1);
				CB* dest = (CB*)((usize)mapped + cb_size * j);
				dest->tex_width = width;
				dest->tex_height = height;
				dest->mip_0_width = (u32)desc.width_or_buffer_size;
				dest->mip_0_height = desc.height;
				dest->roughness = 1.0f / (desc.mip_levels - 1) * (j + 1);
			}
			cb->unmap_subresource(0, 0, USIZE_MAX);

			compute_cmdbuf->resource_barriers({
				ResourceBarrierDesc::as_transition(resource_with_most_detailed_mip, ResourceStateFlag::copy_source, 0),
				ResourceBarrierDesc::as_transition(prefiltered, ResourceStateFlag::copy_dest, 0) });
			compute_cmdbuf->copy_texture_region(TextureCopyLocation::as_subresource_index(prefiltered, 0), 0, 0, 0, 
				TextureCopyLocation::as_subresource_index(resource_with_most_detailed_mip, 0), &BoxU(0, 0, 0, (u32)desc.width_or_buffer_size, desc.height, 1));

			compute_cmdbuf->resource_barrier(ResourceBarrierDesc::as_transition(resource_with_most_detailed_mip, ResourceStateFlag::shader_resource_non_pixel));
			compute_cmdbuf->resource_barrier(ResourceBarrierDesc::as_transition(prefiltered, ResourceStateFlag::unordered_access));

			for (u32 j = 0; j < (u32)(desc.mip_levels - 1); ++j)
			{
				u32 dest_subresource = RHI::calc_subresource_index(j + 1, 0, desc.mip_levels);
				lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_env_mipmapping_dlayout)));
				vs->set_cbv(0, cb, ConstantBufferViewDesc(cb_size * j, cb_size));
				vs->set_srv(1, resource_with_most_detailed_mip);
				vs->set_uav(2, prefiltered, nullptr, &UnorderedAccessViewDesc::tex2d(desc.pixel_format, dest_subresource));
				vs->set_sampler(3, SamplerDesc(Filter::min_mag_mip_linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp));
				compute_cmdbuf->set_compute_descriptor_set(0, vs);
				compute_cmdbuf->attach_device_object(vs);
				u32 width = max<u32>((u32)desc.width_or_buffer_size >> (j + 1), 1);
				u32 height = max<u32>(desc.height >> (j + 1), 1);
				compute_cmdbuf->dispatch(align_upper(width, 8) / 8, align_upper(height, 8) / 8, 1);
			}
			compute_cmdbuf->resource_barrier(ResourceBarrierDesc::as_transition(prefiltered, ResourceStateFlag::common));
			luexp(compute_cmdbuf->submit());
			compute_cmdbuf->wait();
			luexp(compute_cmdbuf->reset());
		}
		lucatchret;
		return prefiltered;
	}

	static Ref<IAssetEditor> new_static_texture_importer(const Path& create_dir)
	{
		auto dialog = new_object<TextureImporter>();
		dialog->m_create_dir = create_dir;
		return dialog;
	}
	void TextureImporter::import_texture_asset(const Path& create_dir, const TextureFile& file)
	{
		lutry
		{
			using namespace RHI;
			auto device = get_main_device();
			// Create resource.
			lulet(tex, device->new_resource(RHI::ResourceDesc::tex2d(RHI::MemoryType::local,
				get_pixel_format_from_image_format(file.m_desc.format), 
				RHI::ResourceUsageFlag::shader_resource | RHI::ResourceUsageFlag::unordered_access, 
				file.m_desc.width, file.m_desc.height)));
			// Upload data.
			{
				Image::ImageDesc image_desc;
				lulet(img_data, Image::read_image_file(file.m_file_data.data(), file.m_file_data.size(), get_desired_format(file.m_desc.format), image_desc));
				luexp(device->copy_resource({
					RHI::ResourceCopyDesc::as_write_texture(tex, img_data.data(),
						pixel_size(image_desc.format) * image_desc.width,
						pixel_size(image_desc.format) * image_desc.width * image_desc.height,
						0, BoxU(0, 0, 0, image_desc.width, image_desc.height, 1))
					}));
			}
			// Generate mipmaps.
			{
				lulet(cmd, g_env->async_compute_queue->new_command_buffer());
				luexp(generate_mipmaps(tex, cmd));
				if (file.m_prefiler_type == TexturePrefilerType::environment_map)
				{
					luset(tex, generate_environment_mipmaps(tex, cmd));
				}
			}
			// Read data.
			auto desc = tex->get_desc();
			Vector<Image::ImageDesc> descs;
			Vector<Blob> img_data;
			Vector<ResourceCopyDesc> copies;
			Vector<Pair<u64, u64>> offsets;
			for (u32 i = 0; i < desc.mip_levels; ++i)
			{
				u32 mip_width = max<u32>((u32)desc.width_or_buffer_size >> i, 1);
				u32 mip_height = max<u32>(desc.height >> i, 1);
				usize row_pitch = (usize)mip_width * bits_per_pixel(desc.pixel_format) / 8;
				usize data_sz = row_pitch * (usize)mip_height;
				Blob data(data_sz);
				img_data.push_back(move(data));
				ResourceCopyDesc copy = ResourceCopyDesc::as_read_texture(tex, img_data.back().data(), (u32)row_pitch, (u32)data_sz, i, BoxU(0, 0, 0, mip_width, mip_height, 1));
				copies.push_back(copy);
			}
			luexp(device->copy_resource({ copies.data(), copies.size() }));
			u64 file_offset = 0;
			Path file_path = create_dir;
			file_path.push_back(file.m_asset_name);
			lulet(asset, Asset::new_asset(file_path, get_static_texture_asset_type()));
			file_path.append_extension("tex");
			lulet(f, VFS::open_file(file_path, FileOpenFlag::write | FileOpenFlag::user_buffering, FileCreationMode::create_always));
			luexp(f->write({ (const byte_t*)"LUNAMIPS", 8 }));
			u64 num_mips = desc.mip_levels;
			luexp(f->write({ (const byte_t*)&num_mips, sizeof(u64) }));
			luexp(f->seek(sizeof(u64) * (2 * img_data.size() + 1), SeekMode::current));
			// Convert data to file.
			for (u32 i = 0; i < desc.mip_levels; ++i)
			{
				u32 mip_width = max<u32>((u32)desc.width_or_buffer_size >> i, 1);
				u32 mip_height = max<u32>(desc.height >> i, 1);

				Image::ImageDesc img_desc;
				img_desc.width = mip_width;
				img_desc.height = mip_height;
				luset(img_desc.format, get_image_format_from_pixel_format(desc.pixel_format));

				Pair<u64, u64> offset;
				luset(offset.first, f->tell());

				if (img_desc.format == Image::ImagePixelFormat::r32_float ||
					img_desc.format == Image::ImagePixelFormat::rg32_float ||
					img_desc.format == Image::ImagePixelFormat::rgba32_float)
				{
					luexp(Image::write_hdr_file(f, img_desc, img_data[i]));
				}
				else
				{
					luexp(Image::write_png_file(f, img_desc, img_data[i]));
				}
				luset(offset.second, f->tell());
				offset.second -= offset.first;
				offsets.push_back(offset);
			}
			// Write header.
			luexp(f->seek(16, SeekMode::begin));
			luexp(f->write({ (const byte_t*)offsets.data(), offsets.size() * sizeof(Pair<u64, u64>) }));
			f.reset();
			Asset::load_asset(asset);

			/*file_path.pop_back();
			for (u32 i = 0; i < desc.mip_levels; ++i)
			{

				c8 buf[32];
				snprintf(buf, 32, "%d.hdr", i);
				file_path.push_back(buf);
				Image::ImageDesc img_desc;
				u32 mip_width = max<u32>((u32)desc.width_or_buffer_size >> i, 1);
				u32 mip_height = max<u32>(desc.height >> i, 1);
				img_desc.width = mip_width;
				img_desc.height = mip_height;
				luset(img_desc.format, get_image_format_from_pixel_format(desc.pixel_format));
				lulet(df, VFS::open_file(file_path, FileOpenFlag::write | FileOpenFlag::user_buffering, FileCreationMode::create_always));
				luexp(Image::write_hdr_file(df, img_desc, img_data[i]));
				file_path.pop_back();
			}*/
		}
		lucatch
		{
			auto _ = Window::message_box(explain(lures), "Failed to import texture asset",
								Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
		}
	}

	void TextureImporter::on_render()
	{
		char title[32];
		sprintf_s(title, "Texture Importer###%d", (u32)(usize)this);

		ImGui::Begin(title, &m_open, ImGuiWindowFlags_NoCollapse);

		if (ImGui::Button("Select Source File"))
		{
			lutry
			{
				m_files.clear();
				lulet(img_paths, Window::open_file_dialog("Image File\0*.jpg;*.jpeg;*.png;*.png;*.tga;*.bmp;*.psd;*.gif;*.hdr;*.pic\0\0",
					"Select Source File", Path(), Window::FileOpenDialogFlag::multi_select));
				for(auto& img_path : img_paths)
				{
					// Open file.
					TextureFile file;
					lulet(img_file, open_file(img_path.encode(PathSeparator::system_preferred).c_str(),
						FileOpenFlag::read | FileOpenFlag::user_buffering, FileCreationMode::open_existing));
					luset(file.m_file_data, load_file_data(img_file));
					luset(file.m_desc, Image::read_image_file_desc(file.m_file_data.data(), file.m_file_data.size()));
					file.m_path = img_path;
					img_path.remove_extension();
					file.m_asset_name = img_path.back();
					file.m_prefiler_type = TexturePrefilerType::normal;
					m_files.push_back(move(file));
				}
			}
			lucatch
			{
				if (lures != BasicError::interrupted())
				{
					auto _ = Window::message_box(explain(lures), "Failed to import texture",
						Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
				}
				m_files.clear();
			}
		}

		if(m_files.empty())
		{
			ImGui::Text("No image file selected.");
		}
		else
		{
			if(ImGui::Button("Import All"))
			{
				for(auto& file : m_files)
				{
					if(!file.m_asset_name.empty())
					{
						import_texture_asset(m_create_dir, file);
					}	
				}
			}
			for(auto& file : m_files)
			{
				ImGui::Text(file.m_path.encode().c_str());
				ImGui::Text("Texture Information:");
				ImGui::Text("Width: %u", file.m_desc.width);
				ImGui::Text("Height: %u", file.m_desc.height);
				const char* fmt = nullptr;
				switch (file.m_desc.format)
				{
				case Image::ImagePixelFormat::r8_unorm:
					fmt = "R8 UNORM"; break;
				case Image::ImagePixelFormat::r16_unorm:
					fmt = "R16 UNORM"; break;
				case Image::ImagePixelFormat::r32_float:
					fmt = "R32 FLOAT"; break;
				case Image::ImagePixelFormat::rg8_unorm:
					fmt = "RG8 UNORM"; break;
				case Image::ImagePixelFormat::rg16_unorm:
					fmt = "RG16 UNORM"; break;
				case Image::ImagePixelFormat::rg32_float:
					fmt = "RG32 FLOAT"; break;
				case Image::ImagePixelFormat::rgb8_unorm:
					fmt = "RGB8 UNORM"; break;
				case Image::ImagePixelFormat::rgb16_unorm:
					fmt = "RGB16 UNORM"; break;
				case Image::ImagePixelFormat::rgb32_float:
					fmt = "RGB32 FLOAT"; break;
				case Image::ImagePixelFormat::rgba8_unorm:
					fmt = "RGBA8 UNORM"; break;
				case Image::ImagePixelFormat::rgba16_unorm:
					fmt = "RGBA16 UNORM"; break;
				case Image::ImagePixelFormat::rgba32_float:
					fmt = "RGBA32 FLOAT"; break;
				default:
					lupanic();
					break;
				}
				ImGui::Text("Format: %s", fmt);

				ImGui::Text("Import Settings:");

				/*const char* items[] = { "R8 UNORM", "R16 UNORM", "R32 FLOAT", "RG8 UNORM", "RG16 UNORM", "RG32 FLOAT", "RGB32 FLOAT",
				"RGBA8 UNORM", "RGBA16 UNORM", "RGBA32 FLOAT" };
				ImGui::Combo("Import Format", &m_import_format, items, 10);
				ImGui::Checkbox("Used as Render Target", &m_allow_render_target);*/
				ImGui::InputText("Asset Name", file.m_asset_name);
				int import_type = (int)file.m_prefiler_type;
				ImGui::Combo("Import Type", &import_type, "Texture\0Environment Map\0\0");
				file.m_prefiler_type = (TexturePrefilerType)import_type;
				if (!file.m_asset_name.empty())
				{
					ImGui::Text("The texture will be imported as: %s%s", m_create_dir.encode().c_str(), file.m_asset_name.c_str());
					if (ImGui::Button("Import"))
					{
						import_texture_asset(m_create_dir, file);
					}
				}
			}
		}
		ImGui::End();
	}
	void register_texture_importer()
	{
		register_boxed_type<TextureImporter>();
		impl_interface_for_type<TextureImporter, IAssetEditor>();
		AssetImporterDesc desc;
		desc.new_importer = new_static_texture_importer;
		g_env->register_asset_importer_type(get_static_texture_asset_type(), desc);
	}
}