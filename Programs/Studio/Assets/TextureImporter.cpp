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
#include <Luna/Window/FileDialog.hpp>
#include <Luna/Window/MessageBox.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/RHI/Utility.hpp>
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
		Ref<RHI::IPipelineLayout> m_mipmapping_playout;
		Ref<RHI::IPipelineState> m_mipmapping_pso;

		Ref<RHI::IDescriptorSetLayout> m_env_mipmapping_dlayout;
		Ref<RHI::IPipelineLayout> m_env_mipmapping_playout;
		Ref<RHI::IPipelineState> m_env_mipmapping_pso;

		static constexpr u32 ENV_MAP_MIPS = 5;

		RV init();
		void import_texture_asset(const Path& create_dir, const TextureFile& file);
		RV generate_mipmaps(RHI::ITexture* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf);
		R<Ref<RHI::ITexture>> generate_environment_mipmaps(RHI::ITexture* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf);

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
				luset(m_mipmapping_dlayout, RHI::get_main_device()->new_descriptor_set_layout(DescriptorSetLayoutDesc({
					DescriptorSetLayoutBinding(DescriptorType::uniform_buffer_view, 0, 1, ShaderVisibilityFlag::all),
					DescriptorSetLayoutBinding(DescriptorType::read_texture_view, 1, 1, ShaderVisibilityFlag::all),
					DescriptorSetLayoutBinding(DescriptorType::read_write_texture_view, 2, 1, ShaderVisibilityFlag::all),
					DescriptorSetLayoutBinding(DescriptorType::sampler, 3, 1, ShaderVisibilityFlag::all)
					})));
				auto dlayout = m_mipmapping_dlayout.get();
				luset(m_mipmapping_playout, RHI::get_main_device()->new_pipeline_layout(PipelineLayoutDesc(
					{ &dlayout, 1 },
					PipelineLayoutFlag::deny_vertex_shader_access |
					
					PipelineLayoutFlag::deny_pixel_shader_access)));

				lulet(cs_blob, compile_shader("Shaders/MipmapGenerationCS.hlsl", ShaderCompiler::ShaderType::compute));
				ComputePipelineStateDesc ps_desc;
				ps_desc.pipeline_layout = m_mipmapping_playout;
				ps_desc.cs = cs_blob.cspan();
				luset(m_mipmapping_pso, RHI::get_main_device()->new_compute_pipeline_state(ps_desc));
			}
			{
				luset(m_env_mipmapping_dlayout, RHI::get_main_device()->new_descriptor_set_layout(DescriptorSetLayoutDesc({
					DescriptorSetLayoutBinding(DescriptorType::uniform_buffer_view, 0, 1, ShaderVisibilityFlag::all),
					DescriptorSetLayoutBinding(DescriptorType::read_texture_view, 1, 1, ShaderVisibilityFlag::all),
					DescriptorSetLayoutBinding(DescriptorType::read_write_texture_view, 2, 1, ShaderVisibilityFlag::all),
					DescriptorSetLayoutBinding(DescriptorType::sampler, 3, 1, ShaderVisibilityFlag::all)
					})));
				auto dlayout = m_env_mipmapping_dlayout.get();
				luset(m_env_mipmapping_playout, RHI::get_main_device()->new_pipeline_layout(PipelineLayoutDesc(
					{ &dlayout, 1 },
					PipelineLayoutFlag::deny_vertex_shader_access |
					
					PipelineLayoutFlag::deny_pixel_shader_access)));

				lulet(cs_blob, compile_shader("Shaders/PrecomputeEnvironmentMapMips.hlsl", ShaderCompiler::ShaderType::compute));
				ComputePipelineStateDesc ps_desc;
				ps_desc.pipeline_layout = m_env_mipmapping_playout;
				ps_desc.cs = cs_blob.cspan();
				luset(m_env_mipmapping_pso, RHI::get_main_device()->new_compute_pipeline_state(ps_desc));
			}
		}
		lucatchret;
		return ok;
	}

	RV TextureImporter::generate_mipmaps(RHI::ITexture* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf)
	{
		using namespace RHI;
		lutry
		{
			auto desc = resource_with_most_detailed_mip->get_desc();
			lucheck(desc.mip_levels);
			lucheck(desc.type == TextureType::tex2d);
			lucheck(desc.depth == 1);

			if (desc.mip_levels == 1)
			{
				return ok;
			}

			auto device = g_env->device;

			if (!m_mipmapping_playout)
			{
				luexp(init());
			}

			compute_cmdbuf->set_compute_pipeline_layout(m_mipmapping_playout);
			compute_cmdbuf->set_compute_pipeline_state(m_mipmapping_pso);
			u32 cb_align = device->get_uniform_buffer_data_alignment();
			u32 cb_size = (u32)align_upper(sizeof(Float2), cb_align);
			lulet(cb, device->new_buffer(MemoryType::upload,
				BufferDesc(BufferUsageFlag::uniform_buffer, cb_size * (desc.mip_levels - 1))));

			void* mapped = nullptr;
			luexp(cb->map(0, 0, &mapped));
			for (u32 j = 0; j < (u32)(desc.mip_levels - 1); ++j)
			{
				u32 width = max<u32>((u32)desc.width >> (j + 1), 1);
				u32 height = max<u32>(desc.height >> (j + 1), 1);
				Float2U* dst = (Float2U*)((usize)mapped + cb_size * j);
				dst->x = 1.0f / (f32)width;
				dst->y = 1.0f / (f32)height;
			}
			cb->unmap(0, USIZE_MAX);

			u32 width = desc.width / 2;
			u32 height = desc.height / 2;

			for (u32 j = 0; j < (u32)(desc.mip_levels - 1); ++j)
			{
				TextureBarrier barriers[] = {
					{resource_with_most_detailed_mip, SubresourceIndex(j, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_cs, ResourceBarrierFlag::none},
					{resource_with_most_detailed_mip, SubresourceIndex(j + 1, 0),TextureStateFlag::automatic, TextureStateFlag::shader_write_cs, ResourceBarrierFlag::none}
				};
				compute_cmdbuf->resource_barrier({}, { barriers, 2 });
				lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_mipmapping_dlayout)));
				vs->update_descriptors({
					WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(cb, cb_size * j, cb_size)),
					WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(resource_with_most_detailed_mip, Format::unknown, j, 1)),
					WriteDescriptorSet::read_write_texture_view(2, TextureViewDesc::tex2d(resource_with_most_detailed_mip, Format::unknown, j + 1, 1)),
					WriteDescriptorSet::sampler(3, SamplerDesc(Filter::linear, Filter::linear, Filter::linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp))
					});
				compute_cmdbuf->set_compute_descriptor_set(0, vs);
				compute_cmdbuf->attach_device_object(vs);
				compute_cmdbuf->dispatch(align_upper(width, 8) / 8, align_upper(height, 8) / 8, 1);
				width = max<u32>(width / 2, 1);
				height = max<u32>(height / 2, 1);
			}

			luexp(compute_cmdbuf->submit({}, {}, true));
			compute_cmdbuf->wait();
			luexp(compute_cmdbuf->reset());
		}
		lucatchret;
		return ok;
	}

	R<Ref<RHI::ITexture>> TextureImporter::generate_environment_mipmaps(RHI::ITexture* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf)
	{
		using namespace RHI;
		Ref<RHI::ITexture> prefiltered;
		lutry
		{
			auto desc = resource_with_most_detailed_mip->get_desc();
			lucheck(desc.mip_levels);
			lucheck(desc.type == TextureType::tex2d);
			lucheck(desc.depth == 1);

			auto device = g_env->device;

			if (!m_mipmapping_playout)
			{
				luexp(init());
			}

			desc.mip_levels = ENV_MAP_MIPS;
			luset(prefiltered, device->new_texture(MemoryType::local, desc));
			desc = prefiltered->get_desc();

			compute_cmdbuf->set_compute_pipeline_layout(m_env_mipmapping_playout);
			compute_cmdbuf->set_compute_pipeline_state(m_env_mipmapping_pso);
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
			lulet(cb, device->new_buffer(MemoryType::upload,
				BufferDesc(BufferUsageFlag::uniform_buffer, cb_size * (desc.mip_levels - 1))));

			void* mapped = nullptr;
			luexp(cb->map(0, 0, &mapped));
			for (u32 j = 0; j < (u32)(desc.mip_levels - 1); ++j)
			{
				u32 width = max<u32>((u32)desc.width >> (j + 1), 1);
				u32 height = max<u32>(desc.height >> (j + 1), 1);
				CB* dst = (CB*)((usize)mapped + cb_size * j);
				dst->tex_width = width;
				dst->tex_height = height;
				dst->mip_0_width = (u32)desc.width;
				dst->mip_0_height = desc.height;
				dst->roughness = 1.0f / (desc.mip_levels - 1) * (j + 1);
			}
			cb->unmap(0, USIZE_MAX);

			compute_cmdbuf->resource_barrier({},
				{
					{resource_with_most_detailed_mip, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::copy_source, ResourceBarrierFlag::none},
					{prefiltered, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::copy_dest, ResourceBarrierFlag::discard_content}
				});
			compute_cmdbuf->copy_texture(prefiltered, SubresourceIndex(0, 0), 0, 0, 0, resource_with_most_detailed_mip, SubresourceIndex(0, 0), 0, 0, 0, desc.width, desc.height, 1);

			compute_cmdbuf->resource_barrier({},
				{
					{resource_with_most_detailed_mip, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::shader_read_cs, ResourceBarrierFlag::none},
					{prefiltered, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic,  TextureStateFlag::shader_read_cs | TextureStateFlag::shader_write_cs, ResourceBarrierFlag::none}
				});

			for (u32 j = 0; j < (u32)(desc.mip_levels - 1); ++j)
			{
				u32 dst_mip = j + 1;
				lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_env_mipmapping_dlayout)));
				vs->update_descriptors({
					WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(cb, cb_size * j, cb_size)),
					WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(resource_with_most_detailed_mip)),
					WriteDescriptorSet::read_write_texture_view(2, TextureViewDesc::tex2d(prefiltered, desc.format, dst_mip, 1)),
					WriteDescriptorSet::sampler(3, SamplerDesc(Filter::linear, Filter::linear, Filter::linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp))
					});
				compute_cmdbuf->set_compute_descriptor_set(0, vs);
				compute_cmdbuf->attach_device_object(vs);
				u32 width = max<u32>((u32)desc.width >> (j + 1), 1);
				u32 height = max<u32>(desc.height >> (j + 1), 1);
				compute_cmdbuf->dispatch(align_upper(width, 8) / 8, align_upper(height, 8) / 8, 1);
			}
			luexp(compute_cmdbuf->submit({}, {}, true));
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
			auto device = g_env->device;
			// Create resource.
			lulet(tex, device->new_texture(RHI::MemoryType::local, RHI::TextureDesc::tex2d(
				get_format_from_image_format(file.m_desc.format), 
				RHI::TextureUsageFlag::read_texture | RHI::TextureUsageFlag::read_write_texture | RHI::TextureUsageFlag::copy_source | RHI::TextureUsageFlag::copy_dest,
				file.m_desc.width, file.m_desc.height)));
			// Upload data.
			{
				Image::ImageDesc image_desc;
				lulet(img_data, Image::read_image_file(file.m_file_data.data(), file.m_file_data.size(), get_desired_format(file.m_desc.format), image_desc));
				lulet(upload_cmdbuf, device->new_command_buffer(g_env->async_copy_queue));
				luexp(copy_resource_data(upload_cmdbuf, {CopyResourceData::write_texture(tex, SubresourceIndex(0, 0), 0, 0, 0, img_data.data(), pixel_size(image_desc.format) * image_desc.width,
					pixel_size(image_desc.format) * image_desc.width * image_desc.height, image_desc.width, image_desc.height, 1)}));
			}
			// Generate mipmaps.
			{
				lulet(cmd, device->new_command_buffer(g_env->async_compute_queue));
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
			Vector<Pair<u64, u64>> offsets;
			for (u32 i = 0; i < desc.mip_levels; ++i)
			{
				u32 mip_width = max<u32>((u32)desc.width >> i, 1);
				u32 mip_height = max<u32>(desc.height >> i, 1);
				usize row_pitch = (usize)mip_width * bits_per_pixel(desc.format) / 8;
				usize data_sz = row_pitch * (usize)mip_height;
				Blob data(data_sz);
				lulet(readback_cmdbuf, device->new_command_buffer(g_env->async_copy_queue));
				luexp(copy_resource_data(readback_cmdbuf, {CopyResourceData::read_texture(data.data(), row_pitch, data_sz, tex, SubresourceIndex(i, 0), 0, 0, 0, mip_width, mip_height, 1)}));
				img_data.push_back(move(data));
			}
			u64 file_offset = 0;
			Path file_path = create_dir;
			file_path.push_back(file.m_asset_name);
			lulet(asset, Asset::new_asset(file_path, get_static_texture_asset_type()));
			file_path.append_extension("tex");
			lulet(f, VFS::open_file(file_path, FileOpenFlag::write | FileOpenFlag::user_buffering, FileCreationMode::create_always));
			luexp(f->write("LUNAMIPS", 8));
			u64 num_mips = desc.mip_levels;
			luexp(f->write(&num_mips, sizeof(u64)));
			luexp(f->seek(sizeof(u64) * (2 * img_data.size() + 1), SeekMode::current));
			// Convert data to file.
			for (u32 i = 0; i < desc.mip_levels; ++i)
			{
				u32 mip_width = max<u32>((u32)desc.width >> i, 1);
				u32 mip_height = max<u32>(desc.height >> i, 1);

				Image::ImageDesc img_desc;
				img_desc.width = mip_width;
				img_desc.height = mip_height;
				luset(img_desc.format, get_image_format_from_format(desc.format));

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
			luexp(f->write(offsets.data(), offsets.size() * sizeof(Pair<u64, u64>)));
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
				luset(img_desc.format, get_image_format_from_format(desc.format));
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
		snprintf(title, 32, "Texture Importer###%d", (u32)(usize)this);

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
					file.m_asset_name = img_path.back().c_str();
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