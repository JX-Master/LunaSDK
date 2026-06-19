/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "Texture.hpp"
#include <Luna/Image/DDSImage.hpp>
#include <Luna/Image/RHIHelper.hpp>
#include <Luna/RHIUtility/MipmapGenerationContext.hpp>
#include "TextureImporter.generated.hpp"

namespace Luna
{
    enum class TexturePrefilerType : u8
    {
        normal = 0,
        environment_map = 1,
    };

    enum class TextureFileType : u8
    {
        image = 0,
        dds = 1,
    };

    struct TextureFile
    {
        Path m_path;
        String m_asset_name;
        Blob m_file_data;
        TextureFileType m_type;
        // For image files.
        Image::ImageDesc m_desc;
        TexturePrefilerType m_prefiler_type;
        // For DDS files.
        Image::DDSImageDesc m_dds_desc;
    };

    struct [[luna::struct("{29488656-e1e3-4e7d-b772-2cf93308ba8b}")]] TextureImporter : public IAssetEditor
    {
        luiimpl();

        Path m_create_dir;

        Vector<TextureFile> m_files;

        Ref<RHIUtility::IMipmapGenerationContext> m_mipmap_generation_ctx;

        Ref<RHI::IDescriptorSetLayout> m_env_mipmapping_dlayout;
        Ref<RHI::IPipelineLayout> m_env_mipmapping_playout;
        Ref<RHI::IPipelineState> m_env_mipmapping_pso;

        static constexpr u32 ENV_MAP_MIPS = 5;

        RV init();
        void import_texture_asset(const Path& create_dir, const TextureFile& file);
        void import_dds_texture_asset(const Path& create_dir, const Image::DDSImage& file);
        R<Ref<RHI::ITexture>> generate_environment_mipmaps(RHI::ITexture* resource_with_most_detailed_mip, RHI::ICommandBuffer* compute_cmdbuf);

        bool m_open = true;

        TextureImporter() {}

        virtual void on_render(GUICore::IContext* context, const GUICore::LayoutInput& layout) override;
        virtual bool closed() override
        {
            return !m_open;
        }
    };
}
