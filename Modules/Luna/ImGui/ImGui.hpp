/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ImGui.hpp
* @author JXMaster
* @date 2022/6/14
*/
#pragma once
#include <Luna/Runtime/Math/Matrix.hpp>
#include "imgui.h"
#include "ImGuizmo.h"
#include <Luna/RHI/RHI.hpp>
#include <Luna/Font/Font.hpp>

#ifndef LUNA_IMGUI_API
#define LUNA_IMGUI_API
#endif

namespace Luna
{
    namespace ImGuiUtils
    {
        //! Sets the current active window.
        LUNA_IMGUI_API void set_active_window(Window::IWindow* window);
        
        //! Dispatches the window event to ImGui.
        //! @param[in] event The window event to handle.
        //! @return Returns `true` if the event is consumed by ImGui, `false` otherwise.
        LUNA_IMGUI_API bool handle_window_event(object_t event);

        //! Updates ImGui IO using inputs and times. This should be called before ImGui::NewFrame().
        LUNA_IMGUI_API void update_io();

        LUNA_IMGUI_API RV render_draw_data(ImDrawData* data, RHI::ICommandBuffer* cmd_buffer, RHI::ITexture* render_target);
        LUNA_IMGUI_API RV refresh_font_texture();
        LUNA_IMGUI_API void add_default_font(f32 font_size);

        LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_default();
        LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_greek();
        LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_korean();
        LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_japanese();
        LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_chinese_full();
        LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_chinese_simplified_common();
        LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_cyrillic();
        LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_thai();
        LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_vietnamese();

        struct ISampledImage : virtual Interface
        {
            luiid("7c85e4ac-3cf1-4d18-9a56-1bd8043e3e3f");

            virtual RHI::ITexture* get_texture() = 0;
            virtual void set_texture(RHI::ITexture* texture) = 0;
            virtual RHI::SamplerDesc get_sampler() = 0;
            virtual void set_sampler(const RHI::SamplerDesc& desc) = 0;
        };

        LUNA_IMGUI_API Ref<ISampledImage> new_sampled_image(RHI::ITexture* texture, const RHI::SamplerDesc& sampler_desc);
    }

    struct Module;
    LUNA_IMGUI_API Module* module_imgui();
}

namespace ImGui
{
    LUNA_IMGUI_API void Image(Luna::RHI::ITexture* texture, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));
    LUNA_IMGUI_API void Image(Luna::ImGuiUtils::ISampledImage* texture, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));
    LUNA_IMGUI_API bool ImageButton(const char* str_id, Luna::RHI::ITexture* texture, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));
    LUNA_IMGUI_API bool ImageButton(const char* str_id, Luna::ImGuiUtils::ISampledImage* texture, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));
    LUNA_IMGUI_API bool InputText(const char* label, Luna::String& buf, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    LUNA_IMGUI_API bool InputTextMultiline(const char* label, Luna::String& buf, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    LUNA_IMGUI_API bool InputTextWithHint(const char* label, const char* hint, Luna::String& buf, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);

    enum class GizmoOperation : Luna::u32
    {
        translate = 0,
        rotate = 1,
        scale = 2,
        bounds = 3,
    };

    enum class GizmoMode : Luna::u32
    {
        local = 0,
        world = 1,
    };

    LUNA_IMGUI_API void Gizmo(Luna::Float4x4& world_matrix, const Luna::Float4x4& view, const Luna::Float4x4& projection, const Luna::RectF& viewport_rect,
        GizmoOperation operation, GizmoMode mode,
        Luna::f32 snap, bool enabled, bool orthographic, Luna::Float4x4* delta_matrix,
        bool* is_mouse_hover, bool* is_mouse_moving);
}