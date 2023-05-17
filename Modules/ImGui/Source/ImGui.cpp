/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ImGui.cpp
* @author JXMaster
* @date 2022/6/14
*/
#pragma once
#include <Runtime/PlatformDefines.hpp>
#define LUNA_IMGUI_API LUNA_EXPORT
#include "../ImGui.hpp"
#include <Runtime/Result.hpp>
#include <Runtime/Module.hpp>
#include <Runtime/Time.hpp>
#include <HID/HID.hpp>
#include <HID/Keyboard.hpp>
#include <HID/Mouse.hpp>
#include <Runtime/Math/Matrix.hpp>
#include <Font/Font.hpp>
#include <RHI/ShaderCompileHelper.hpp>
namespace Luna
{
    template<>
    struct hash<RHI::Format>
    {
        usize operator()(RHI::Format val) const
        {
            return static_cast<usize>(val);
        }
    };

    namespace ImGuiUtils
    {
        using namespace ImGui;

        Ref<Window::IWindow> g_active_window;
        u64 g_time;

        Ref<RHI::IBuffer> g_vb;
        Ref<RHI::IBuffer> g_ib;
        usize g_vb_size;
        usize g_ib_size;

        Blob g_vs_blob;
        Blob g_ps_blob;

        Ref<RHI::IDescriptorSetLayout> g_desc_layout;
        Ref<RHI::IShaderInputLayout> g_slayout;
        HashMap<RHI::Format, Ref<RHI::IPipelineState>> g_pso;

        //! Expand when not enough.
        Vector<Ref<RHI::IDescriptorSet>> g_desc_sets;

        Ref<RHI::IBuffer> g_cb;

        Ref<RHI::ITexture> g_font_tex;

        const c8 IMGUI_VS_SOURCE[] = R"(
cbuffer vertexBuffer : register(b0) 
{
    float4x4 ProjectionMatrix; 
};
struct VS_INPUT
{
    [[vk::location(0)]]
    float2 pos : POSITION;
    [[vk::location(1)]]
    float2 uv  : TEXCOORD0;
    [[vk::location(2)]]
    float4 col : COLOR0;
};
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 pos : SV_POSITION;
    [[vk::location(1)]]
    float2 uv  : TEXCOORD0;
    [[vk::location(2)]]
    float4 col : COLOR0;
};
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
	output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));
	output.col = input.col;
	output.uv  = input.uv;
	return output;
})";
        const c8 IMGUI_PS_SOURCE[] = R"(
struct PS_INPUT
{
    [[vk::location(0)]]
    float4 pos : SV_POSITION;
    [[vk::location(1)]]
    float2 uv  : TEXCOORD0;
    [[vk::location(2)]]
    float4 col : COLOR0;
};
SamplerState sampler0 : register(s2);
Texture2D texture0 : register(t1);
[[vk::location(0)]]
float4 main(PS_INPUT input) : SV_Target
{
    float4 out_col = input.col * texture0.Sample(sampler0, input.uv); 
    return out_col; 
}
)";

        static RV init()
        {
            using namespace RHI;
            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
            //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
            //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
            io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;     // Disable mouse cursor chgange. TODO: add support for this later.
            //io.ConfigViewportsNoAutoMerge = true;
            //io.ConfigViewportsNoTaskBarIcon = true;
            //io.ConfigViewportsNoDefaultParent = true;
            //io.ConfigDockingAlwaysTabBar = true;
            //io.ConfigDockingTransparentPayload = true;
            //io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;     // FIXME-DPI: Experimental. THIS CURRENTLY DOESN'T WORK AS EXPECTED. DON'T USE IN USER APP!
            //io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI: Experimental.

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            //ImGui::StyleColorsClassic();

            // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
            ImGuiStyle& style = ImGui::GetStyle();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                style.WindowRounding = 0.0f;
                style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            }

            g_time = get_ticks();

            g_vb_size = 0;
            g_ib_size = 0;

            io.BackendRendererName = "imgui_impl_luna_rhi";
            io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
            io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)

            // Create render resources.
            lutry
            {
                auto dev = get_main_device();

                auto compiler = ShaderCompiler::new_compiler();

                compiler->set_source({ IMGUI_VS_SOURCE, sizeof(IMGUI_VS_SOURCE) });
                compiler->set_source_name("ImGuiVS");
                compiler->set_entry_point("main");
                compiler->set_target_format(get_current_platform_shader_target_format());
                compiler->set_shader_type(ShaderCompiler::ShaderType::vertex);
                compiler->set_shader_model(6, 0);
                compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);
                luexp(compiler->compile());
                auto vs_data = compiler->get_output();
                g_vs_blob = Blob(vs_data.data(), vs_data.size());
                compiler->reset();
                compiler->set_source({ IMGUI_PS_SOURCE, sizeof(IMGUI_PS_SOURCE) });
                compiler->set_source_name("ImGuiPS");
                compiler->set_entry_point("main");
                compiler->set_target_format(get_current_platform_shader_target_format());
                compiler->set_shader_type(ShaderCompiler::ShaderType::pixel);
                compiler->set_shader_model(6, 0);
                compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);
                luexp(compiler->compile());
                auto ps_data = compiler->get_output();
                g_ps_blob = Blob(ps_data.data(), ps_data.size());
                luset(g_desc_layout, dev->new_descriptor_set_layout(DescriptorSetLayoutDesc(
                    {
                        DescriptorSetLayoutBinding(DescriptorType::uniform_buffer_view, 0, 1, ShaderVisibilityFlag::vertex),
                        DescriptorSetLayoutBinding(DescriptorType::sampled_texture_view, 1, 1, ShaderVisibilityFlag::pixel),
                        DescriptorSetLayoutBinding(DescriptorType::sampler, 2, 1, ShaderVisibilityFlag::pixel),
                    }
                )));
                IDescriptorSetLayout* dl = g_desc_layout;
                luset(g_slayout, dev->new_shader_input_layout(ShaderInputLayoutDesc({
                    &dl, 1
                    },
                    ShaderInputLayoutFlag::allow_input_assembler_input_layout)));

                // Create constant buffer.
                usize buffer_size_align = dev->get_uniform_buffer_data_alignment();
                luset(g_cb, dev->new_buffer(BufferDesc(ResourceHeapType::upload, BufferUsageFlag::uniform_buffer, align_upper(sizeof(Float4x4), buffer_size_align))));
            }
            lucatchret;

            return ok;
        }

        static RV rebuild_font(f32 scale)
        {
            using namespace RHI;
            lutry
            {
                ImGuiIO& io = ::ImGui::GetIO();
                unsigned char* pixels;
                int width, height;

                io.Fonts->Clear();

                auto default_font = Font::get_default_font();
                usize font_size = default_font->data().size();
                void* font_data = ImGui::MemAlloc(font_size);
                memcpy(font_data, default_font->data().data(), font_size);
                io.Fonts->AddFontFromMemoryTTF(const_cast<void*>(font_data), (int)font_size, 18.0f * scale);
                io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
                auto dev = get_main_device();
                luset(g_font_tex, dev->new_texture(TextureDesc::tex2d(ResourceHeapType::local, Format::rgba8_unorm, 
                    TextureUsageFlag::sampled_texture | TextureUsageFlag::copy_dest, width, height, 1, 1)));
                u32 src_row_pitch = (u32)width * 4;
                {
                    u64 size, row_pitch, slice_pitch;
                    dev->get_texture_data_placement_info(width, height, 1, Format::rgba8_unorm, &size, nullptr, &row_pitch, &slice_pitch);
                    lulet(tex_staging, dev->new_buffer(BufferDesc(ResourceHeapType::upload, BufferUsageFlag::copy_source, size)));
                    lulet(tex_staging_data, tex_staging->map(0, 0));
                    memcpy_bitmap(tex_staging_data, pixels, src_row_pitch, height, row_pitch, src_row_pitch);
                    tex_staging->unmap(0, src_row_pitch * height);

                    u32 copy_queue_index = U32_MAX;
                    {
                        // Prefer a dedicated copy queue if present.
                        u32 num_queues = dev->get_num_command_queues();
                        for (u32 i = 0; i < num_queues; ++i)
                        {
                            auto desc = dev->get_command_queue_desc(i);
                            if (desc.type == CommandQueueType::graphics && copy_queue_index == U32_MAX)
                            {
                                copy_queue_index = i;
                            }
                            else if (desc.type == CommandQueueType::copy)
                            {
                                copy_queue_index = i;
                                break;
                            }
                        }
                    }
                    lulet(upload_cmdbuf, dev->new_command_buffer(copy_queue_index));
                    upload_cmdbuf->resource_barrier({
                        { tex_staging, BufferStateFlag::automatic, BufferStateFlag::copy_source, ResourceBarrierFlag::none} },
                        { { g_font_tex, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::copy_dest, ResourceBarrierFlag::discard_content } });
                    upload_cmdbuf->copy_buffer_to_texture(g_font_tex, SubresourceIndex(0, 0), 0, 0, 0, tex_staging, 0,
                        src_row_pitch, src_row_pitch * height, width, height, 1);
                    luexp(upload_cmdbuf->submit({}, {}, true));
                    upload_cmdbuf->wait();
                }
                io.Fonts->TexID = (ITexture*)(g_font_tex);
            }
            lucatchret;
            return ok;
        }

        static void close()
        {
            ImGui::DestroyContext();
            g_vb = nullptr;
            g_ib = nullptr;
            g_vs_blob.clear();
            g_ps_blob.clear();
            g_active_window = nullptr;
            g_slayout = nullptr;
            g_pso.clear();
            g_pso.shrink_to_fit();
            g_cb = nullptr;
            g_font_tex = nullptr;
            g_desc_layout = nullptr;
            g_desc_sets.clear();
            g_desc_sets.shrink_to_fit();
        }

        inline ImGuiKey hid_key_to_imgui_key(HID::KeyCode key)
        {
            using namespace HID;
            switch (key)
            {
            case KeyCode::tab: return ImGuiKey_Tab;
            case KeyCode::left: return ImGuiKey_LeftArrow;
            case KeyCode::right: return ImGuiKey_RightArrow;
            case KeyCode::up: return ImGuiKey_UpArrow;
            case KeyCode::down: return ImGuiKey_DownArrow;
            case KeyCode::page_up: return ImGuiKey_PageUp;
            case KeyCode::page_down: return ImGuiKey_PageDown;
            case KeyCode::home: return ImGuiKey_Home;
            case KeyCode::end: return ImGuiKey_End;
            case KeyCode::insert: return ImGuiKey_Insert;
            case KeyCode::del: return ImGuiKey_Delete;
            case KeyCode::backspace: return ImGuiKey_Backspace;
            case KeyCode::spacebar: return ImGuiKey_Space;
            case KeyCode::enter: return ImGuiKey_Enter;
            case KeyCode::esc: return ImGuiKey_Escape;
            case KeyCode::quote: return ImGuiKey_Apostrophe;
            case KeyCode::comma: return ImGuiKey_Comma;
            case KeyCode::minus: return ImGuiKey_Minus;
            case KeyCode::period: return ImGuiKey_Period;
            case KeyCode::slash: return ImGuiKey_Slash;
            case KeyCode::semicolon: return ImGuiKey_Semicolon;
            case KeyCode::equal: return ImGuiKey_Equal;
            case KeyCode::l_branket: return ImGuiKey_LeftBracket;
            case KeyCode::backslash: return ImGuiKey_Backslash;
            case KeyCode::r_branket: return ImGuiKey_RightBracket;
            case KeyCode::grave: return ImGuiKey_GraveAccent;
            case KeyCode::caps_lock: return ImGuiKey_CapsLock;
            case KeyCode::scroll_lock: return ImGuiKey_ScrollLock;
            case KeyCode::num_lock: return ImGuiKey_NumLock;
            case KeyCode::print_screen: return ImGuiKey_PrintScreen;
            case KeyCode::pause: return ImGuiKey_Pause;
            case KeyCode::numpad0: return ImGuiKey_Keypad0;
            case KeyCode::numpad1: return ImGuiKey_Keypad1;
            case KeyCode::numpad2: return ImGuiKey_Keypad2;
            case KeyCode::numpad3: return ImGuiKey_Keypad3;
            case KeyCode::numpad4: return ImGuiKey_Keypad4;
            case KeyCode::numpad5: return ImGuiKey_Keypad5;
            case KeyCode::numpad6: return ImGuiKey_Keypad6;
            case KeyCode::numpad7: return ImGuiKey_Keypad7;
            case KeyCode::numpad8: return ImGuiKey_Keypad8;
            case KeyCode::numpad9: return ImGuiKey_Keypad9;
            case KeyCode::numpad_decimal: return ImGuiKey_KeypadDecimal;
            case KeyCode::numpad_divide: return ImGuiKey_KeypadDivide;
            case KeyCode::numpad_multiply: return ImGuiKey_KeypadMultiply;
            case KeyCode::numpad_subtract: return ImGuiKey_KeypadSubtract;
            case KeyCode::numpad_add: return ImGuiKey_KeypadAdd;
            case KeyCode::l_shift: return ImGuiKey_LeftShift;
            case KeyCode::l_ctrl: return ImGuiKey_LeftCtrl;
            case KeyCode::l_menu: return ImGuiKey_LeftAlt;
            case KeyCode::l_system: return ImGuiKey_LeftSuper;
            case KeyCode::r_shift: return ImGuiKey_RightShift;
            case KeyCode::r_ctrl: return ImGuiKey_RightCtrl;
            case KeyCode::r_menu: return ImGuiKey_RightAlt;
            case KeyCode::r_system: return ImGuiKey_RightSuper;
            case KeyCode::apps: return ImGuiKey_Menu;
            case KeyCode::num0: return ImGuiKey_0;
            case KeyCode::num1: return ImGuiKey_1;
            case KeyCode::num2: return ImGuiKey_2;
            case KeyCode::num3: return ImGuiKey_3;
            case KeyCode::num4: return ImGuiKey_4;
            case KeyCode::num5: return ImGuiKey_5;
            case KeyCode::num6: return ImGuiKey_6;
            case KeyCode::num7: return ImGuiKey_7;
            case KeyCode::num8: return ImGuiKey_8;
            case KeyCode::num9: return ImGuiKey_9;
            case KeyCode::a: return ImGuiKey_A;
            case KeyCode::b: return ImGuiKey_B;
            case KeyCode::c: return ImGuiKey_C;
            case KeyCode::d: return ImGuiKey_D;
            case KeyCode::e: return ImGuiKey_E;
            case KeyCode::f: return ImGuiKey_F;
            case KeyCode::g: return ImGuiKey_G;
            case KeyCode::h: return ImGuiKey_H;
            case KeyCode::i: return ImGuiKey_I;
            case KeyCode::j: return ImGuiKey_J;
            case KeyCode::k: return ImGuiKey_K;
            case KeyCode::l: return ImGuiKey_L;
            case KeyCode::m: return ImGuiKey_M;
            case KeyCode::n: return ImGuiKey_N;
            case KeyCode::o: return ImGuiKey_O;
            case KeyCode::p: return ImGuiKey_P;
            case KeyCode::q: return ImGuiKey_Q;
            case KeyCode::r: return ImGuiKey_R;
            case KeyCode::s: return ImGuiKey_S;
            case KeyCode::t: return ImGuiKey_T;
            case KeyCode::u: return ImGuiKey_U;
            case KeyCode::v: return ImGuiKey_V;
            case KeyCode::w: return ImGuiKey_W;
            case KeyCode::x: return ImGuiKey_X;
            case KeyCode::y: return ImGuiKey_Y;
            case KeyCode::z: return ImGuiKey_Z;
            case KeyCode::f1: return ImGuiKey_F1;
            case KeyCode::f2: return ImGuiKey_F2;
            case KeyCode::f3: return ImGuiKey_F3;
            case KeyCode::f4: return ImGuiKey_F4;
            case KeyCode::f5: return ImGuiKey_F5;
            case KeyCode::f6: return ImGuiKey_F6;
            case KeyCode::f7: return ImGuiKey_F7;
            case KeyCode::f8: return ImGuiKey_F8;
            case KeyCode::f9: return ImGuiKey_F9;
            case KeyCode::f10: return ImGuiKey_F10;
            case KeyCode::f11: return ImGuiKey_F11;
            case KeyCode::f12: return ImGuiKey_F12;
            default: return ImGuiKey_None;
            }
        }

        static void handle_mouse_move(Window::IWindow* window, i32 x, i32 y)
        {
            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                auto pos = g_active_window->client_to_screen(Int2U(x, y));
                x = pos.x;
                y = pos.y;
            }
            io.AddMousePosEvent((f32)x, (f32)y);
        }

        static void handle_mouse_down(Window::IWindow* window, Window::ModifierKeyFlag modifier_flags, HID::MouseButton button)
        {
            ImGuiIO& io = ImGui::GetIO();
            int button_id = 0;
            if (button == HID::MouseButton::left) button_id = 0;
            else if (button == HID::MouseButton::right) button_id = 1;
            else if (button == HID::MouseButton::middle) button_id = 2;
            else if (button == HID::MouseButton::function1) button_id = 3;
            else if (button == HID::MouseButton::function2) button_id = 4;
            // TODO: Add capture API.
            io.AddMouseButtonEvent(button_id, true);
        }

        static void handle_mouse_up(Window::IWindow* window, Window::ModifierKeyFlag modifier_flags, HID::MouseButton button)
        {
            ImGuiIO& io = ImGui::GetIO();
            int button_id = 0;
            if (button == HID::MouseButton::left) button_id = 0;
            else if (button == HID::MouseButton::right) button_id = 1;
            else if (button == HID::MouseButton::middle) button_id = 2;
            else if (button == HID::MouseButton::function1) button_id = 3;
            else if (button == HID::MouseButton::function2) button_id = 4;

            io.AddMouseButtonEvent(button_id, false);
        }

        static void handle_mouse_wheel(Window::IWindow* window, f32 x_wheel_delta, f32 y_wheel_delta)
        {
            ImGuiIO& io = ImGui::GetIO();
            io.AddMouseWheelEvent(x_wheel_delta, y_wheel_delta);
        }

        static void handle_key_state_change(HID::KeyCode key, bool is_key_down)
        {
            ImGuiIO& io = ImGui::GetIO();
            auto keyboard = HID::get_device<HID::IKeyboard>().get();
            // Submit modifiers
            io.AddKeyEvent(ImGuiKey_ModCtrl, keyboard->get_key_state(HID::KeyCode::ctrl));
            io.AddKeyEvent(ImGuiKey_ModShift, keyboard->get_key_state(HID::KeyCode::shift));
            io.AddKeyEvent(ImGuiKey_ModAlt, keyboard->get_key_state(HID::KeyCode::menu));
            io.AddKeyEvent(ImGuiKey_ModSuper, keyboard->get_key_state(HID::KeyCode::apps));
            auto key_id = hid_key_to_imgui_key(key);
            if (key_id != ImGuiKey_None)
            {
                io.AddKeyEvent(key_id, is_key_down);
            }
            // Submit individual left/right modifier events
            if (key == HID::KeyCode::shift)
            {
                if (keyboard->get_key_state(HID::KeyCode::l_shift) == is_key_down) io.AddKeyEvent(ImGuiKey_LeftShift, is_key_down);
                if (keyboard->get_key_state(HID::KeyCode::r_shift) == is_key_down) io.AddKeyEvent(ImGuiKey_RightShift, is_key_down);
            }
            else if (key == HID::KeyCode::ctrl)
            {
                if (keyboard->get_key_state(HID::KeyCode::l_ctrl) == is_key_down) io.AddKeyEvent(ImGuiKey_LeftCtrl, is_key_down);
                if (keyboard->get_key_state(HID::KeyCode::r_ctrl) == is_key_down) io.AddKeyEvent(ImGuiKey_RightCtrl, is_key_down);
            }
            else if (key == HID::KeyCode::menu)
            {
                if (keyboard->get_key_state(HID::KeyCode::l_menu) == is_key_down) io.AddKeyEvent(ImGuiKey_LeftAlt, is_key_down);
                if (keyboard->get_key_state(HID::KeyCode::r_menu) == is_key_down) io.AddKeyEvent(ImGuiKey_RightAlt, is_key_down);
            }
        }

        static void handle_key_down(Window::IWindow* window, HID::KeyCode key)
        {
            handle_key_state_change(key, true);
        }

        static void handle_key_up(Window::IWindow* window, HID::KeyCode key)
        {
            handle_key_state_change(key, false);
        }

        static void handle_focus(Window::IWindow* window)
        {
            ImGuiIO& io = ImGui::GetIO();
            io.AddFocusEvent(true);
        }

        static void handle_lose_focus(Window::IWindow* window)
        {
            ImGuiIO& io = ImGui::GetIO();
            io.AddFocusEvent(false);
        }

        static void handle_input_character(Window::IWindow* window, c32 character)
        {
            ImGuiIO& io = ImGui::GetIO();
            io.AddInputCharacterUTF16((c16)character);
        }

        static void handle_dpi_changed(Window::IWindow* window, f32 dpi_scale)
        {
            auto _ = rebuild_font(dpi_scale);
        }

        LUNA_IMGUI_API void set_active_window(Window::IWindow* window)
        {
            if (g_active_window)
            {
                // Unregister old callbacks.
                g_active_window->get_mouse_move_event() -= handle_mouse_move;
                g_active_window->get_mouse_down_event() -= handle_mouse_down;
                g_active_window->get_mouse_up_event() -= handle_mouse_up;
                g_active_window->get_mouse_wheel_event() -= handle_mouse_wheel;
                g_active_window->get_key_down_event() -= handle_key_down;
                g_active_window->get_key_up_event() -= handle_key_up;
                g_active_window->get_focus_event() -= handle_focus;
                g_active_window->get_lose_focus_event() -= handle_lose_focus;
                g_active_window->get_input_character_event() -= handle_input_character;
                g_active_window->get_dpi_changed_event() -= handle_dpi_changed;
            }
            g_active_window = window;
            if (g_active_window)
            {
                // Register new callbacks.
                g_active_window->get_mouse_move_event() += handle_mouse_move;
                g_active_window->get_mouse_down_event() += handle_mouse_down;
                g_active_window->get_mouse_up_event() += handle_mouse_up;
                g_active_window->get_mouse_wheel_event() += handle_mouse_wheel;
                g_active_window->get_key_down_event() += handle_key_down;
                g_active_window->get_key_up_event() += handle_key_up;
                g_active_window->get_focus_event() += handle_focus;
                g_active_window->get_lose_focus_event() += handle_lose_focus;
                g_active_window->get_input_character_event() += handle_input_character;
                g_active_window->get_dpi_changed_event() += handle_dpi_changed;
            }
        }

        static void update_hid_mouse()
        {
            ImGuiIO& io = ImGui::GetIO();

            auto mouse = HID::get_device<HID::IMouse>().get();
            auto mouse_pos = mouse->get_cursor_pos();

            bool app_focused = true;
            //if(g_active_window) app_focused = g_active_window->is_foreground();
            if (app_focused)
            {
                // (Optional) Set OS mouse position from Dear ImGui if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
                // When multi-viewports are enabled, all Dear ImGui positions are same as OS positions.
                if (io.WantSetMousePos)
                {
                    Int2U pos = { (int)io.MousePos.x, (int)io.MousePos.y };
                    if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) == 0)
                        pos = g_active_window->client_to_screen(pos);
                    auto _ = mouse->set_cursor_pos(pos.x, pos.y);
                }
            }
        }

        LUNA_IMGUI_API void update_io()
        {
            ImGuiIO& io = ImGui::GetIO();

            // Setup time step
            u64 current_time = get_ticks();
            io.DeltaTime = (f32)((f64)(current_time - g_time) / get_ticks_per_second());
            g_time = current_time;

            // Setup display size (every frame to accommodate for window resizing)
            if (g_active_window)
            {
                auto sz = g_active_window->get_size();
                io.DisplaySize = ImVec2((f32)sz.x, (f32)sz.y);
            }
            
            // Update OS mouse position
            update_hid_mouse();

            if (!g_font_tex)
            {
                if (g_active_window)
                {
                    auto _ = rebuild_font(g_active_window->get_dpi_scale_factor());
                }
                else
                {
                    auto _ = rebuild_font(1.0f);
                }
            }
        }

        static R<RHI::IPipelineState*> get_pso(RHI::Format rt_format)
        {
            using namespace RHI;
            auto iter = g_pso.find(rt_format);
            if (iter != g_pso.end()) return iter->second.get();
            lutry
            {
                GraphicsPipelineStateDesc ps_desc;
                ps_desc.primitive_topology = PrimitiveTopology::triangle_list;
                ps_desc.sample_mask = U32_MAX;
                ps_desc.sample_quality = 0;
                ps_desc.blend_state = BlendDesc({ AttachmentBlendDesc(true, BlendFactor::src_alpha,
                    BlendFactor::inv_src_alpha, BlendOp::add, BlendFactor::inv_src_alpha, BlendFactor::zero, BlendOp::add, ColorWriteMask::all) });
                ps_desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::none, 0, 0.0f, 0.0f, 1, false, true, false, false, false);
                ps_desc.depth_stencil_state = DepthStencilDesc(false, false, ComparisonFunc::always, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
                ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
                InputBindingDesc input_bindings[] = {
                    InputBindingDesc(0, sizeof(ImDrawVert), InputRate::per_vertex)
                };
                InputAttributeDesc input_attributes[] = {
                    InputAttributeDesc("POSITION", 0, 0, 0, 0, Format::rg32_float),
                    InputAttributeDesc("TEXCOORD", 0, 1, 0, 8, Format::rg32_float),
                    InputAttributeDesc("COLOR", 0, 2, 0, 16, Format::rgba8_unorm)
                };
                ps_desc.input_layout.bindings = { input_bindings, 1 };
                ps_desc.input_layout.attributes = { input_attributes , 3 };
                ps_desc.vs = { g_vs_blob.data(), g_vs_blob.size() };
                ps_desc.ps = { g_ps_blob.data(), g_ps_blob.size() };
                ps_desc.shader_input_layout = g_slayout;
                ps_desc.num_render_targets = 1;
                ps_desc.rtv_formats[0] = rt_format;
                lulet(pso, get_main_device()->new_graphics_pipeline_state(ps_desc));
                iter = g_pso.insert(make_pair(rt_format, pso)).first;
            }
            lucatchret;
            return iter->second.get();
        }

        LUNA_IMGUI_API RV render_draw_data(ImDrawData* data, RHI::ICommandBuffer* cmd_buffer, RHI::IRenderTargetView* render_target)
        {
            using namespace RHI;
            lutry
            {
                ImDrawData* draw_data = ::ImGui::GetDrawData();
                // Avoid rendering when minimized
                if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
                    return ok;
                // Create and grow vertex/index buffers if needed
                auto dev = cmd_buffer->get_device();
                if (!g_vb || g_vb_size < (u32)draw_data->TotalVtxCount)
                {
                    g_vb_size = draw_data->TotalVtxCount + 5000;
                    luset(g_vb, dev->new_buffer(BufferDesc(ResourceHeapType::upload, BufferUsageFlag::vertex_buffer, g_vb_size * sizeof(ImDrawVert))));
                }
                if (!g_ib || g_ib_size < (u32)draw_data->TotalIdxCount)
                {
                    g_ib_size = draw_data->TotalIdxCount + 10000;
                    luset(g_ib, dev->new_buffer(BufferDesc(ResourceHeapType::upload, BufferUsageFlag::index_buffer, g_ib_size * sizeof(ImDrawIdx))));
                }

                // Upload vertex/index data into a single contiguous GPU buffer
                lulet(vtx_resource, g_vb->map(0, 0));
                lulet(idx_resource, g_ib->map(0, 0));
                ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource;
                ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource;
                for (i32 n = 0; n < draw_data->CmdListsCount; ++n)
                {
                    const ImDrawList* cmd_list = draw_data->CmdLists[n];
                    memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                    memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                    vtx_dst += cmd_list->VtxBuffer.Size;
                    idx_dst += cmd_list->IdxBuffer.Size;
                }
                g_vb->unmap(0, (usize)vtx_dst - (usize)vtx_resource);
                g_ib->unmap(0, (usize)idx_dst - (usize)idx_resource);

                auto res = render_target->get_texture();
                auto rt_desc = res->get_desc();

                // Setup orthographic projection matrix into our constant buffer
                // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right).
                Float4x4 mvp;
                {
                    float L = draw_data->DisplayPos.x;
                    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
                    float T = draw_data->DisplayPos.y;
                    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
                    mvp =
                    {
                        { 2.0f / (R - L),		0.0f,				0.0f,       0.0f },
                        { 0.0f,					2.0f / (T - B),     0.0f,       0.0f },
                        { 0.0f,					0.0f,				0.5f,       0.0f },
                        { (R + L) / (L - R),	(T + B) / (B - T),  0.5f,       1.0f },
                    };
                    lulet(cb_resource, g_cb->map(0, 0));
                    memcpy(cb_resource, &mvp, sizeof(Float4x4));
                    g_cb->unmap(0, sizeof(Float4x4));
                }

                Vector<TextureBarrier> barriers;
                barriers.push_back({ res, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::color_attachment_write, ResourceBarrierFlag::none });
                for (i32 n = 0; n < draw_data->CmdListsCount; ++n)
                {
                    const ImDrawList* cmd_list = draw_data->CmdLists[n];
                    for (i32 cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i)
                    {
                        const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                        barriers.push_back({ (ITexture*)pcmd->TextureId, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::shader_read_ps, ResourceBarrierFlag::none });
                    }
                }

                cmd_buffer->resource_barrier({},
                    { barriers.data(), barriers.size() });

                RenderPassDesc desc;
                desc.color_attachments[0] = render_target;
                desc.color_load_ops[0] = LoadOp::load;
                desc.color_store_ops[0] = StoreOp::store;
                cmd_buffer->begin_render_pass(desc);

                cmd_buffer->set_viewport(Viewport(0.0f, 0.0f, draw_data->DisplaySize.x, draw_data->DisplaySize.y, 0.0f, 1.0f));

                cmd_buffer->set_vertex_buffers(0, { &VertexBufferView(g_vb, 0, (u32)(g_vb_size * sizeof(ImDrawVert)), sizeof(ImDrawVert)), 1 });
                cmd_buffer->set_index_buffer({g_ib, 0, (u32)(g_ib_size * sizeof(ImDrawIdx)), sizeof(ImDrawIdx) == 2 ? Format::r16_uint : Format::r32_uint});
                lulet(pso, get_pso(rt_desc.pixel_format));
                cmd_buffer->set_pipeline_state(pso);
                cmd_buffer->set_graphics_shader_input_layout(g_slayout);
                const f32 blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
                cmd_buffer->set_blend_factor(blend_factor);

                // Render command lists.
                i32 vtx_offset = 0;
                i32 idx_offset = 0;
                Float2 clip_off = { draw_data->DisplayPos.x, draw_data->DisplayPos.y };
                Float2 clip_scale = { draw_data->FramebufferScale.x, draw_data->FramebufferScale.y };

                u32 num_draw_calls = 0;

                for (i32 n = 0; n < draw_data->CmdListsCount; ++n)
                {
                    const ImDrawList* cmd_list = draw_data->CmdLists[n];
                    for (i32 cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i)
                    {
                        const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                        if (pcmd->UserCallback)
                        {
                            pcmd->UserCallback(cmd_list, pcmd);
                        }
                        else
                        {
                            // Project scissor/clipping rectangles into framebuffer space
                            Float2 clip_min = Float2((pcmd->ClipRect.x - clip_off.x), (pcmd->ClipRect.y - clip_off.y)) * clip_scale;
                            Float2 clip_max = Float2((pcmd->ClipRect.z - clip_off.x), (pcmd->ClipRect.w - clip_off.y)) * clip_scale;
                            // Apply Scissor, Bind texture, Draw
                            const RectI r = {
                                (i32)(clip_min.x),
                                (i32)(clip_min.y),
                                (i32)(clip_max.x - clip_min.x),
                                (i32)(clip_max.y - clip_min.y) };
                            while (g_desc_sets.size() <= num_draw_calls)
                            {
                                lulet(new_vs, dev->new_descriptor_set(DescriptorSetDesc(g_desc_layout)));
                                g_desc_sets.push_back(new_vs);
                            }
                            IDescriptorSet* vs = g_desc_sets[num_draw_calls];
                            usize cb_align = dev->get_uniform_buffer_data_alignment();
                            luexp(vs->update_descriptors({
                                DescriptorSetWrite::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(g_cb)),
                                DescriptorSetWrite::sampled_texture_view(1, TextureViewDesc::tex2d((ITexture*)pcmd->TextureId)),
                                DescriptorSetWrite::sampler(2, SamplerDesc(Filter::min_mag_mip_linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp))
                                }));
                            cmd_buffer->set_graphics_descriptor_sets(0, { &vs, 1 });
                            cmd_buffer->set_scissor_rect(r);
                            cmd_buffer->draw_indexed(pcmd->ElemCount, pcmd->IdxOffset + idx_offset, pcmd->VtxOffset + vtx_offset);
                            ++num_draw_calls;
                        }
                    }
                    idx_offset += cmd_list->IdxBuffer.Size;
                    vtx_offset += cmd_list->VtxBuffer.Size;
                }

                cmd_buffer->end_render_pass();
            }
            lucatchret;
            return ok;
        }

        StaticRegisterModule imgui_module("ImGui", "RHI;HID;Font;ShaderCompiler", init, close);
    }
}

namespace ImGui
{
    struct InputTextCallback_UserData
    {
        Luna::String& Str;
        ImGuiInputTextCallback ChainCallback;
        void* ChainCallbackUserData;
        InputTextCallback_UserData(Luna::String& s) :
            Str(s) {}
    };

    static int InputTextCallback(ImGuiInputTextCallbackData* data)
    {
        InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
        {
            // Resize string callback
            // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
            Luna::String& str = user_data->Str;
            IM_ASSERT(data->Buf == str.c_str());
            str.resize(data->BufTextLen, '\0');
            data->Buf = (char*)str.c_str();
        }
        else if (user_data->ChainCallback)
        {
            // Forward to user callback, if any
            data->UserData = user_data->ChainCallbackUserData;
            return user_data->ChainCallback(data);
        }
        return 0;
    }

    LUNA_IMGUI_API bool InputText(const char* label, Luna::String& buf, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
    {
        IM_ASSERT(!(flags & ImGuiInputTextFlags_CallbackResize));
        flags |= ImGuiInputTextFlags_CallbackResize;

        InputTextCallback_UserData cb_user_data(buf);
        cb_user_data.ChainCallback = callback;
        cb_user_data.ChainCallbackUserData = user_data;
        return InputText(label, (char*)buf.c_str(), buf.capacity() + 1, flags, InputTextCallback, &cb_user_data);
    }

    LUNA_IMGUI_API bool InputTextMultiline(const char* label, Luna::String& buf, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
    {
        IM_ASSERT(!(flags & ImGuiInputTextFlags_CallbackResize));
        flags |= ImGuiInputTextFlags_CallbackResize;

        InputTextCallback_UserData cb_user_data(buf);
        cb_user_data.ChainCallback = callback;
        cb_user_data.ChainCallbackUserData = user_data;
        return InputTextMultiline(label, (char*)buf.c_str(), buf.capacity() + 1, size, flags, InputTextCallback, &cb_user_data);
    }

    LUNA_IMGUI_API bool InputTextWithHint(const char* label, const char* hint, Luna::String& buf, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
    {
        IM_ASSERT(!(flags & ImGuiInputTextFlags_CallbackResize));
        flags |= ImGuiInputTextFlags_CallbackResize;

        InputTextCallback_UserData cb_user_data(buf);
        cb_user_data.ChainCallback = callback;
        cb_user_data.ChainCallbackUserData = user_data;
        return InputTextWithHint(label, hint, (char*)buf.c_str(), buf.capacity() + 1, flags, InputTextCallback, &cb_user_data);
    }

    LUNA_IMGUI_API void Gizmo(Luna::Float4x4& world_matrix, const Luna::Float4x4& view, const Luna::Float4x4& projection, const Luna::RectF& viewport_rect,
        GizmoOperation operation, GizmoMode mode,
        Luna::f32 snap, bool enabled, bool orthographic, Luna::Float4x4* delta_matrix,
        bool* is_mouse_hover, bool* is_mouse_moving)
    {
        using namespace Luna;
        // Set States.
        ImGuizmo::SetDrawlist();
        ImGuizmo::Enable(enabled);
        ImGuizmo::SetRect(viewport_rect.offset_x, viewport_rect.offset_y, viewport_rect.width, viewport_rect.height);
        ImGuizmo::SetOrthographic(orthographic);

        f32* fdelta_matrix = nullptr;
        f32* fsnap = nullptr;
        if (delta_matrix)
        {
            fdelta_matrix = delta_matrix->r[0].m;
        }
        if (snap)
        {
            fsnap = &snap;
        }

        ImGuizmo::OPERATION op = ImGuizmo::UNIVERSAL;
        switch (operation)
        {
        case ImGui::GizmoOperation::translate:
            op = ImGuizmo::TRANSLATE;
            break;
        case ImGui::GizmoOperation::rotate:
            op = ImGuizmo::ROTATE;
            break;
        case ImGui::GizmoOperation::scale:
            op = ImGuizmo::SCALE;
            break;
        case ImGui::GizmoOperation::bounds:
            op = ImGuizmo::UNIVERSAL;
            break;
        default:
            break;
        }
        ImGuizmo::MODE md = ImGuizmo::LOCAL;
        switch (mode)
        {
        case ImGui::GizmoMode::local:
            md = ImGuizmo::LOCAL;
            break;
        case ImGui::GizmoMode::world:
            md = ImGuizmo::WORLD;
            break;
        default:
            break;
        }

        ImGuizmo::Manipulate(view.r[0].m, projection.r[0].m, op, md, world_matrix.r[0].m, fdelta_matrix, fsnap);

        if (is_mouse_hover)
        {
            *is_mouse_hover = ImGuizmo::IsOver();
        }
        if (is_mouse_moving)
        {
            *is_mouse_moving = ImGuizmo::IsUsing();
        }
    }
}