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
	struct TextureImporter : public IAssetEditor
	{
		lustruct("TextureImporter", "{29488656-e1e3-4e7d-b772-2cf93308ba8b}");
		luiimpl();

		Path m_create_dir;

		Path m_source_file_path;

		Image::ImageDesc m_desc;

		//int m_import_format;
		String m_asset_name;

		Blob m_file_data;

		bool m_open;

		//bool m_allow_render_target;


		TextureImporter() :
			m_open(true)
			//m_allow_render_target(false),
			//m_import_format(7)
		{
			m_asset_name = String();
		}

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

	void TextureImporter::on_render()
	{
		char title[32];
		sprintf_s(title, "Texture Importer###%d", (u32)(usize)this);

		ImGui::Begin(title, &m_open, ImGuiWindowFlags_NoCollapse);

		if (ImGui::Button("Select Source File"))
		{
			lutry
			{
				lulet(img_path, Window::open_file_dialog("Image File\0*.jpg;*.jpeg;*.png;*.png;*.tga;*.bmp;*.psd;*.gif;*.hdr;*.pic\0\0",
					"Select Source File"));
				// Open file.
				lulet(img_file, open_file(img_path[0].encode(PathSeparator::system_preferred).c_str(),
					FileOpenFlag::read | FileOpenFlag::user_buffering, FileCreationMode::open_existing));
				luset(m_file_data, load_file_data(img_file));
				luset(m_desc, Image::read_image_file_desc(m_file_data.data(), m_file_data.size()));
				m_source_file_path = img_path[0];
			}
			lucatch
			{
				if (lures != BasicError::interrupted())
				{
					auto _ = Window::message_box(explain(lures), "Failed to import texture",
						Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
				}
				m_source_file_path = nullptr;
			}
		}

		if (m_source_file_path.empty())
		{
			ImGui::Text("No image file selected.");
		}
		else
		{
			ImGui::Text(m_source_file_path.encode().c_str());
			ImGui::Text("Texture Information:");
			ImGui::Text("Width: %u", m_desc.width);
			ImGui::Text("Height: %u", m_desc.height);
			const char* fmt = nullptr;
			switch (m_desc.format)
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
			ImGui::InputText("Asset Name", m_asset_name);
			if (!m_asset_name.empty())
			{
				ImGui::Text("The texture will be imported as: %s%s", m_create_dir.encode().c_str(), m_asset_name.c_str());
				if (ImGui::Button("Import"))
				{
					lutry2
					{
						Path file_path = m_create_dir;
						file_path.push_back(m_asset_name);
						lulet2(asset, Asset::new_asset(file_path, get_static_texture_asset_type()));
						file_path.append_extension("tex");
						lulet2(file, VFS::open_file(file_path, FileOpenFlag::write, FileCreationMode::create_always));
						luexp2(file->write({(byte_t*)m_file_data.data(), m_file_data.size()}));
						file.reset();
						Asset::load_asset(asset);
					}
					lucatch2
					{
						auto _ = Window::message_box(explain(lures2), "Failed to create texture asset",
							Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
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