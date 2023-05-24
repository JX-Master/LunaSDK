/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SceneRenderer.hpp
* @author JXMaster
* @date 2023/3/27
*/
#pragma once
#include "Scene.hpp"
#include <RG/RenderGraph.hpp>
namespace Luna
{
    struct CameraCB
	{
		Float4x4U world_to_view;
		Float4x4U view_to_proj;
		Float4x4U world_to_proj;
		Float4x4U proj_to_world;
		Float4x4U view_to_world;
		u32 screen_width;
		u32 screen_height;
	};

	struct LightingParams
	{
		Float3U strength;
		f32 attenuation_power;
		Float3U direction;
		u32 type;
		Float3U position;
		f32 spot_attenuation_power;
	};

    enum class SceneRendererMode : u8
    {
        lit = 0,
        wireframe,
        base_color,
        normal,
        roughness,
        metallic,
        depth,
        emissive,
        diffuse_lighting,
        specular_lighting,
        ambient_diffuse_lighting,
        ambient_specular_lighting
    };

    luenum(SceneRendererMode, "SceneRendererMode", "e66271d7-cbe7-4f0b-8de3-de0cc7b06982");

    struct SceneRendererSettings
    {
        // The screen size.
        UInt2U screen_size;
        // Whether to collect profiling data.
        bool frame_profiling = false;
        // The rendering mode.
        SceneRendererMode mode = SceneRendererMode::lit;

        bool operator==(const SceneRendererSettings& rhs) const
        {
            return screen_size == rhs.screen_size && 
            frame_profiling == rhs.frame_profiling && 
            mode == rhs.mode;
        }
        bool operator!=(const SceneRendererSettings& rhs) const
        {
            return !(*this == rhs);
        }
    };

    struct SceneRenderer
    {
        // The scene to be rendered.
        Ref<Scene> scene = nullptr;

        // The command buffer used to render the scene.
        Ref<RHI::ICommandBuffer> command_buffer;

        // The result texture.
        Ref<RHI::ITexture> render_texture;

        // The name of enabled passes if frame_profiling is enabled.
        Vector<Name> enabled_passes;
        // The time intervals of each passes if frame_profiling is enabled.
		Vector<f64> pass_time_intervals;

        SceneRenderer(RHI::IDevice* device);
        const SceneRendererSettings& get_settings();
        RV reset(const SceneRendererSettings& settings);
        RV render();
        void collect_frame_profiling_data();
        
    private:
        // Resources.
		static constexpr usize LIGHTING_BUFFER = 0;
		static constexpr usize DEPTH_BUFFER = 1;
		static constexpr usize BACK_BUFFER = 2;
		static constexpr usize WIREFRAME_BACK_BUFFER = 3;
        static constexpr usize GBUFFER_VIS_BUFFER = 4;
		static constexpr usize BASE_COLOR_ROUGHNESS_BUFFER = 5;
		static constexpr usize NORMAL_METALLIC_BUFFER = 6;
		static constexpr usize EMISSIVE_BUFFER = 7;

		// Passes.
		static constexpr usize WIREFRAME_PASS = 0;
		static constexpr usize DEPTH_PASS = 1;
		static constexpr usize GEOMETRY_PASS = 2;
        static constexpr usize BUFFER_VIS_PASS = 3;
		static constexpr usize SKYBOX_PASS = 4;
		static constexpr usize DEFERRED_LIGHTING_PASS = 5;
		static constexpr usize TONE_MAPPING_PASS = 6;
        Ref<RHI::IDevice> m_device;
        SceneRendererSettings m_settings;
        Ref<RG::IRenderGraph> m_render_graph;
        Ref<RHI::IBuffer> m_camera_cb;
        usize m_num_model_matrices = 0;
        Ref<RHI::IBuffer> m_model_matrices;
        usize m_num_lights = 0;
        Ref<RHI::IBuffer> m_lighting_params;
    };
}