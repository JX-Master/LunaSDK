/*!
* This file is a portion of Luna SDK.
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

		//! Updates ImGui IO using inputs and times. This should be called before ImGui::NewFrame().
		LUNA_IMGUI_API void update_io();

		LUNA_IMGUI_API RV render_draw_data(ImDrawData* data, RHI::ICommandBuffer* cmd_buffer, RHI::ITexture* render_target);

		LUNA_IMGUI_API RV set_font(Font::IFontFile* font = nullptr, f32 font_size = 18.0f, Span<Pair<c16, c16>> ranges = {});

		LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_default();
		LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_greek();
		LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_korean();
		LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_japanese();
		LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_chinese_full();
		LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_chinese_simplified_common();
		LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_cyrillic();
		LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_thai();
		LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_vietnamese();
	}

	struct Module;
	LUNA_IMGUI_API Module* module_imgui();
}

namespace ImGui
{
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