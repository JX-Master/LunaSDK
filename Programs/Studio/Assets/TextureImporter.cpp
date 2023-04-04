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
	struct TextureFile
	{
		Path m_path;
		Image::ImageDesc m_desc;
		String m_asset_name;
		Blob m_file_data;
	};

	struct TextureImporter : public IAssetEditor
	{
		lustruct("TextureImporter", "{29488656-e1e3-4e7d-b772-2cf93308ba8b}");
		luiimpl();

		Path m_create_dir;

		Vector<TextureFile> m_files;

		bool m_open;

		TextureImporter() :
			m_open(true) {}

		virtual void on_render() override;
		virtual bool closed() override
		{
			return !m_open;
		}
	};

	static Ref<IAssetEditor> new_static_texture_importer(const Path& create_dir)
	{
		auto dialog = new_object<TextureImporter>();
		dialog->m_create_dir = create_dir;
		return dialog;
	}

	static void import_texture_asset(const Path& create_dir, const TextureFile& file)
	{
		lutry
		{
			Path file_path = create_dir;
			file_path.push_back(file.m_asset_name);
			lulet(asset, Asset::new_asset(file_path, get_static_texture_asset_type()));
			file_path.append_extension("tex");
			lulet(f, VFS::open_file(file_path, FileOpenFlag::write, FileCreationMode::create_always));
			luexp(f->write({(byte_t*)file.m_file_data.data(), file.m_file_data.size()}));
			f.reset();
			Asset::load_asset(asset);
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