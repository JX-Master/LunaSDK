/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ImGui.cpp
* @author JXMaster
* @date 2022/6/14
*/
#include "Luna/RHI/CommandBuffer.hpp"
#include "Luna/RHIUtility/MipmapGenerationContext.hpp"
#include "Luna/Runtime/Object.hpp"
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_IMGUI_API LUNA_EXPORT
#include "../ImGui.hpp"
#include "imgui.h"
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Time.hpp>
#include <Luna/HID/HID.hpp>
#include <Luna/HID/Keyboard.hpp>
#include <Luna/HID/Mouse.hpp>
#include <Luna/Runtime/Math/Matrix.hpp>
#include <Luna/Font/Font.hpp>
#include <Luna/RHI/ShaderCompileHelper.hpp>
#include <Luna/RHIUtility/RHIUtility.hpp>
#include <Luna/RHIUtility/ResourceWriteContext.hpp>
#include <Luna/Window/Event.hpp>
#include "Luna/Window/Clipboard.hpp"
#include <ImGuiVS.hpp>
#include <ImGuiPS.hpp>
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

        Ref<RHI::IDescriptorSetLayout> g_desc_layout;
        Ref<RHI::IPipelineLayout> g_playout;
        HashMap<RHI::Format, Ref<RHI::IPipelineState>> g_pso;

        //! Expand when not enough.
        Vector<Ref<RHI::IDescriptorSet>> g_desc_sets;

        Ref<RHI::IBuffer> g_cb;

        struct SampledImage : ISampledImage
        {
            lustruct("ImGuiUtils::SampledImage", "29378bf1-b58e-4c8a-a30f-d29239f9a713");
            luiimpl();

            Ref<RHI::ITexture> m_texture;
            RHI::SamplerDesc m_sampler;

            virtual RHI::ITexture* get_texture() override { return m_texture; }
            virtual void set_texture(RHI::ITexture* texture) override { m_texture = texture; }
            virtual RHI::SamplerDesc get_sampler() override { return m_sampler; }
            virtual void set_sampler(const RHI::SamplerDesc& desc) override { m_sampler = desc; }
        };

        LUNA_IMGUI_API Ref<ISampledImage> new_sampled_image(RHI::ITexture* texture, const RHI::SamplerDesc& sampler_desc)
        {
            Ref<SampledImage> image = new_object<SampledImage>();
            image->m_texture = texture;
            image->m_sampler = sampler_desc;
            return Ref<ISampledImage>(image);
        }

        struct ImGuiClipboardData
        {
            String text;
        };

        static RV init()
        {
            using namespace RHI;
            register_boxed_type<SampledImage>();
            impl_interface_for_type<SampledImage, ISampledImage>();

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
            io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
            
            auto& platform_io = ImGui::GetPlatformIO();

            platform_io.Platform_ClipboardUserData = memnew<ImGuiClipboardData>();
            platform_io.Platform_GetClipboardTextFn = [](ImGuiContext* ctx) -> const char*
            {
                ImGuiClipboardData* data = (ImGuiClipboardData*)ImGui::GetPlatformIO().Platform_ClipboardUserData;
                if(data)
                {
                    auto r = Window::get_clipboard_text(data->text);
                    return data->text.c_str();
                }
                return nullptr;
            };
            platform_io.Platform_SetClipboardTextFn = [](ImGuiContext* ctx, const char* text)
            {
                auto r = Window::set_clipboard_text(text);
            };
            platform_io.Platform_SetImeDataFn = [](ImGuiContext* ctx, ImGuiViewport* viewport, ImGuiPlatformImeData* data)
            {
                if(!g_active_window) return;
                if ((!(data->WantVisible || data->WantTextInput) && g_active_window->is_text_input_active()))
                {
                    auto _ = g_active_window->end_text_input();
                }
                if (data->WantVisible)
                {
                    RectI r;
                    r.offset_x = (int)(data->InputPos.x - viewport->Pos.x);
                    r.offset_y = (int)(data->InputPos.y - viewport->Pos.y + data->InputLineHeight);
                    r.width = 1;
                    r.height = (int)data->InputLineHeight;
                    auto _ = g_active_window->set_text_input_area(r, 0);
                }
                if (!g_active_window->is_text_input_active() && (data->WantVisible || data->WantTextInput))
                {
                    auto _ = g_active_window->begin_text_input();
                }
            };

            add_default_font(18.0f);

            // Create render resources.
            lutry
            {
                auto dev = get_main_device();
                luset(g_desc_layout, dev->new_descriptor_set_layout(DescriptorSetLayoutDesc(
                    {
                        DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::vertex),
                        DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 1, 1, ShaderVisibilityFlag::pixel),
                        DescriptorSetLayoutBinding::sampler(2, 1, ShaderVisibilityFlag::pixel),
                    }
                )));
                IDescriptorSetLayout* dl = g_desc_layout;
                luset(g_playout, dev->new_pipeline_layout(PipelineLayoutDesc({
                    &dl, 1
                    },
                    PipelineLayoutFlag::allow_input_assembler_input_layout)));

                // Create constant buffer.
                usize buffer_size_align = dev->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment;
                luset(g_cb, dev->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::uniform_buffer, align_upper(sizeof(Float4x4), buffer_size_align))));
            }
            lucatchret;

            return ok;
        }

        LUNA_IMGUI_API void add_default_font(f32 font_size)
        {
            ImGuiIO& io = ::ImGui::GetIO();
            Font::IFontFile* font = Font::get_default_font();
            usize font_data_size = font->get_data().size();
            void* font_data = ImGui::MemAlloc(font_data_size);
            memcpy(font_data, font->get_data().data(), font_data_size);
            io.Fonts->AddFontFromMemoryTTF(const_cast<void*>(font_data), (int)font_data_size, font_size, NULL, NULL);
        }

        static f32 get_font_render_scale()
        {
            return g_active_window ? g_active_window->get_dpi_scale_factor() : 1.0f;
        }

        static f32 get_font_display_scale()
        {
            if(!g_active_window) return 1.0;
            auto sz = g_active_window->get_size();
            auto fb_sz = g_active_window->get_framebuffer_size();
            f32 display_scale = (f32)sz.x / (f32)fb_sz.x;
            return display_scale;
        }

        LUNA_IMGUI_API Vector<Pair<c16, c16>> get_glyph_ranges_default()
        {
            ImGuiIO& io = ::ImGui::GetIO();
            const ImWchar* range = io.Fonts->GetGlyphRangesDefault();
            Vector<Pair<c16, c16>> r;
            while(*range)
            {
                r.push_back(make_pair(range[0], range[1]));
                range += 2;
            }
            return r;
        }
        static void close()
        {
            auto& platform_io = ImGui::GetPlatformIO();
            ImGuiClipboardData* clipboard = (ImGuiClipboardData*)platform_io.Platform_ClipboardUserData;
            if(clipboard)
            {
                memdelete(clipboard);
                platform_io.Platform_ClipboardUserData = nullptr;
            }
            // Delete all imgui textures.
            for (ImTextureData* tex : ImGui::GetPlatformIO().Textures)
            {
                if (tex->RefCount == 1 && tex->BackendUserData)
                {
                    object_t tex_id = (object_t)tex->BackendUserData;
                    object_release(tex_id);
                    tex->BackendUserData = nullptr;
                    tex->SetTexID(ImTextureID_Invalid);
                    tex->SetStatus(ImTextureStatus_Destroyed);
                }
            }
            ImGui::DestroyContext();
            g_vb = nullptr;
            g_ib = nullptr;
            g_active_window = nullptr;
            g_playout = nullptr;
            g_pso.clear();
            g_pso.shrink_to_fit();
            g_cb = nullptr;
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

        static void handle_key_state_change(HID::KeyCode key, bool is_key_down)
        {
            ImGuiIO& io = ImGui::GetIO();
            // Submit modifiers
            io.AddKeyEvent(ImGuiMod_Ctrl, HID::get_key_state(HID::KeyCode::ctrl));
            io.AddKeyEvent(ImGuiMod_Shift, HID::get_key_state(HID::KeyCode::shift));
            io.AddKeyEvent(ImGuiMod_Alt, HID::get_key_state(HID::KeyCode::menu));
            io.AddKeyEvent(ImGuiMod_Super, HID::get_key_state(HID::KeyCode::l_system) || HID::get_key_state(HID::KeyCode::r_system));
            auto key_id = hid_key_to_imgui_key(key);
            if (key_id != ImGuiKey_None)
            {
                io.AddKeyEvent(key_id, is_key_down);
            }
            // Submit individual left/right modifier events
            if (key == HID::KeyCode::shift)
            {
                if (HID::get_key_state(HID::KeyCode::l_shift) == is_key_down) io.AddKeyEvent(ImGuiKey_LeftShift, is_key_down);
                if (HID::get_key_state(HID::KeyCode::r_shift) == is_key_down) io.AddKeyEvent(ImGuiKey_RightShift, is_key_down);
            }
            else if (key == HID::KeyCode::ctrl)
            {
                if (HID::get_key_state(HID::KeyCode::l_ctrl) == is_key_down) io.AddKeyEvent(ImGuiKey_LeftCtrl, is_key_down);
                if (HID::get_key_state(HID::KeyCode::r_ctrl) == is_key_down) io.AddKeyEvent(ImGuiKey_RightCtrl, is_key_down);
            }
            else if (key == HID::KeyCode::menu)
            {
                if (HID::get_key_state(HID::KeyCode::l_menu) == is_key_down) io.AddKeyEvent(ImGuiKey_LeftAlt, is_key_down);
                if (HID::get_key_state(HID::KeyCode::r_menu) == is_key_down) io.AddKeyEvent(ImGuiKey_RightAlt, is_key_down);
            }
            else if (key == HID::KeyCode::l_system || key == HID::KeyCode::r_system)
            {
                if (HID::get_key_state(HID::KeyCode::l_system) == is_key_down) io.AddKeyEvent(ImGuiKey_LeftSuper, is_key_down);
                if (HID::get_key_state(HID::KeyCode::r_system) == is_key_down) io.AddKeyEvent(ImGuiKey_RightSuper, is_key_down);
            }
        }

        LUNA_IMGUI_API bool handle_window_event(object_t event)
        {
            using namespace Window;
            ImGuiIO& io = ImGui::GetIO();
            if(auto window_event = cast_object<WindowEvent>(event))
            {
                if(window_event->window != g_active_window) return false;
                if(auto e = cast_object<WindowMouseMoveEvent>(event))
                {
                    i32 x = e->x;
                    i32 y = e->y;
                    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                    {
                        auto pos = g_active_window->client_to_screen(Int2U(x, y));
                        x = pos.x;
                        y = pos.y;
                    }
                    io.AddMousePosEvent((f32)x, (f32)y);
                    return true;
                }
                else if(auto e = cast_object<WindowMouseDownEvent>(event))
                {
                    int button_id = 0;
                    if (e->button == HID::MouseButton::left) button_id = 0;
                    else if (e->button == HID::MouseButton::right) button_id = 1;
                    else if (e->button == HID::MouseButton::middle) button_id = 2;
                    else if (e->button == HID::MouseButton::function1) button_id = 3;
                    else if (e->button == HID::MouseButton::function2) button_id = 4;
                    // TODO: Add capture API.
                    io.AddMouseButtonEvent(button_id, true);
                    return true;
                }
                else if(auto e = cast_object<WindowMouseUpEvent>(event))
                {
                    int button_id = 0;
                    if (e->button == HID::MouseButton::left) button_id = 0;
                    else if (e->button == HID::MouseButton::right) button_id = 1;
                    else if (e->button == HID::MouseButton::middle) button_id = 2;
                    else if (e->button == HID::MouseButton::function1) button_id = 3;
                    else if (e->button == HID::MouseButton::function2) button_id = 4;

                    io.AddMouseButtonEvent(button_id, false);
                    return true;
                }
                else if(auto e = cast_object<WindowScrollEvent>(event))
                {
                    io.AddMouseWheelEvent(e->scroll_x, e->scroll_y);
                    return true;
                }
                else if(auto e = cast_object<WindowKeyDownEvent>(event))
                {
                    handle_key_state_change(e->key, true);
                    return true;
                }
                else if(auto e = cast_object<WindowKeyUpEvent>(event))
                {
                    handle_key_state_change(e->key, false);
                    return true;
                }
                else if(auto e = cast_object<WindowInputFocusEvent>(event))
                {
                    io.AddFocusEvent(true);
                    return true;
                }
                else if(auto e = cast_object<WindowLoseInputFocusEvent>(event))
                {
                    io.AddFocusEvent(false);
                    return true;
                }
                else if(auto e = cast_object<WindowInputTextEvent>(event))
                {
                    io.AddInputCharactersUTF8(e->text.c_str());
                    return true;
                }
                else if(auto e = cast_object<WindowDPIScaleChangedEvent>(event))
                {
                    ImGuiIO& io = ::ImGui::GetIO();
                    f32 render_scale = get_font_render_scale();
                    f32 display_scale = get_font_display_scale();
                    if(io.Fonts->Sources.Size == 0)
                    {
                        add_default_font(18.0f * render_scale);
                    }
                    // Modify font configs.
                    for(auto& config : io.Fonts->Sources)
                    {
                        config.SizePixels = 18.0f * render_scale;
                    }
                    auto& style = ImGui::GetStyle();
                    style.FontScaleMain = display_scale;
                    return true;
                }
            }
            return false;
        }

        LUNA_IMGUI_API void set_active_window(Window::IWindow* window)
        {
            g_active_window = window;
        }

        static void update_hid_mouse()
        {
            ImGuiIO& io = ImGui::GetIO();

            auto mouse_pos = HID::get_mouse_pos();

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
                    auto _ = HID::set_mouse_pos(pos.x, pos.y);
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
                auto framebuffer_sz = g_active_window->get_framebuffer_size();
                io.DisplaySize = ImVec2((f32)sz.x, (f32)sz.y);
                io.DisplayFramebufferScale = ImVec2((f32)framebuffer_sz.x / (f32)sz.x, (f32)framebuffer_sz.y / (f32)sz.y);
            }
            
            // Update OS mouse position
            update_hid_mouse();
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
                ps_desc.blend_state = BlendDesc({ AttachmentBlendDesc(true, BlendFactor::src_alpha,
                    BlendFactor::one_minus_src_alpha, BlendOp::add, BlendFactor::one_minus_src_alpha, BlendFactor::zero, BlendOp::add, ColorWriteMask::all) });
                ps_desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::none, 0, 0.0f, 0.0f, false, true);
                ps_desc.depth_stencil_state = DepthStencilDesc(false, false, CompareFunction::always, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
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
                ps_desc.vs = LUNA_GET_SHADER_DATA(ImGuiVS);
                ps_desc.ps = LUNA_GET_SHADER_DATA(ImGuiPS);
                ps_desc.pipeline_layout = g_playout;
                ps_desc.num_color_attachments = 1;
                ps_desc.color_formats[0] = rt_format;
                lulet(pso, get_main_device()->new_graphics_pipeline_state(ps_desc));
                iter = g_pso.insert(make_pair(rt_format, pso)).first;
            }
            lucatchret;
            return iter->second.get();
        }
        struct TextureUpdateContext
        {
            Ref<RHI::ICommandBuffer> m_cmd_buffer;
            Ref<RHIUtility::IResourceWriteContext> m_writer;

            RHIUtility::IResourceWriteContext* get_writer()
            {
                if(!m_writer)
                {
                    m_writer = RHIUtility::new_resource_write_context(m_cmd_buffer->get_device());
                }
                return m_writer;
            }
        };
        static RV update_texture(ImTextureData* texture, TextureUpdateContext& ctx)
        {
            lutry
            {
                RHI::IDevice* dev = ctx.m_cmd_buffer->get_device();
                if(texture->Status == ImTextureStatus_WantCreate)
                {
                    // Create and upload new texture to graphics system
                    //IMGUI_DEBUG_LOG("UpdateTexture #%03d: WantCreate %dx%d\n", tex->UniqueID, tex->Width, tex->Height);
                    IM_ASSERT(texture->TexID == 0 && texture->BackendUserData == nullptr);
                    IM_ASSERT(texture->Format == ImTextureFormat_RGBA32);

                    // Create texture
                    // (Bilinear sampling is required by default. Set 'io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to allow point/nearest sampling)
                    lulet(tex, dev->new_texture(RHI::MemoryType::local, RHI::TextureDesc::tex2d(
                        RHI::Format::rgba8_unorm, 
                        RHI::TextureUsageFlag::read_texture | RHI::TextureUsageFlag::copy_dest, 
                        texture->Width, texture->Height, 1, 1)));
                    
                    auto writer = ctx.get_writer();
                    u32 row_pitch, slice_pitch;
                    lulet(mapped, writer->write_texture(tex, RHI::SubresourceIndex(0, 0), 0, 0, 0, texture->Width, texture->Height, 1, row_pitch, slice_pitch));
                    memcpy_bitmap(mapped, texture->GetPixels(), 
                        texture->GetPitch(), texture->Height, 
                        row_pitch, texture->GetPitch());
                    
                    luexp(writer->commit(ctx.m_cmd_buffer, true));
                    writer->reset();
                    texture->SetTexID((ImTextureID)tex.object());
                    texture->BackendUserData = tex.object();
                    // For one reference from texture backend user data.
                    object_retain(tex.object());
                    texture->SetStatus(ImTextureStatus_OK);
                }
                else if(texture->Status == ImTextureStatus_WantUpdates)
                {
                    object_t tex_id = (object_t)texture->GetTexID();
                    RHI::ITexture* dst_tex = query_interface<RHI::ITexture>(tex_id);
                    luassert(dst_tex);
                    auto writer = ctx.get_writer();
                    u32 row_pitch, slice_pitch;
                    auto r = texture->UpdateRect;
                    lulet(mapped, writer->write_texture(
                        dst_tex, RHI::SubresourceIndex(0, 0), 
                        r.x, r.y, 0, 
                        r.w, r.h, 1, row_pitch, slice_pitch));
                    memcpy_bitmap(mapped, texture->GetPixelsAt(r.x, r.y), 
                        r.w * texture->BytesPerPixel, r.h, row_pitch, texture->GetPitch());
                    luexp(writer->commit(ctx.m_cmd_buffer, true));
                    writer->reset();
                    texture->SetStatus(ImTextureStatus_OK);
                }
                else if(texture->Status == ImTextureStatus_WantDestroy)
                {
                    object_t tex_id = (object_t)texture->BackendUserData;
                    if(!tex_id) return ok;
                    object_release(tex_id);
                    texture->BackendUserData = nullptr;
                    texture->SetTexID(ImTextureID_Invalid);
                    texture->SetStatus(ImTextureStatus_Destroyed);
                }
            }
            lucatchret;
            return ok;
        }

        LUNA_IMGUI_API RV render_draw_data(ImDrawData* draw_data, RHI::ICommandBuffer* cmd_buffer, RHI::ITexture* render_target)
        {
            using namespace RHI;
            lutry
            {
                // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
                ImGuiIO& io = ImGui::GetIO();
                int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
                int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
                if (fb_width == 0 || fb_height == 0)
                    return ok;

                if(draw_data->Textures != nullptr)
                {
                    TextureUpdateContext ctx;
                    ctx.m_cmd_buffer = cmd_buffer;
                    for(ImTextureData* texture : *draw_data->Textures)
                    {
                        if(texture->Status != ImTextureStatus_OK)
                        {
                            luexp(update_texture(texture, ctx));
                        }
                    }
                }

                draw_data->ScaleClipRects(io.DisplayFramebufferScale);
                
                // Create and grow vertex/index buffers if needed
                auto dev = cmd_buffer->get_device();
                if (!g_vb || g_vb_size < (u32)draw_data->TotalVtxCount)
                {
                    g_vb_size = draw_data->TotalVtxCount + 5000;
                    luset(g_vb, dev->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::vertex_buffer, g_vb_size * sizeof(ImDrawVert))));
                }
                if (!g_ib || g_ib_size < (u32)draw_data->TotalIdxCount)
                {
                    g_ib_size = draw_data->TotalIdxCount + 10000;
                    luset(g_ib, dev->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::index_buffer, g_ib_size * sizeof(ImDrawIdx))));
                }
                // Upload vertex/index data into a single contiguous GPU buffer
                ImDrawVert* vtx_resource = nullptr;
                ImDrawIdx* idx_resource = nullptr;
                luexp(g_vb->map(0, 0, (void**)&vtx_resource));
                luexp(g_ib->map(0, 0, (void**)&idx_resource));
                ImDrawVert* vtx_dst = vtx_resource;
                ImDrawIdx* idx_dst = idx_resource;
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
                auto rt_desc = render_target->get_desc();

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
                        { 2.0f / (R - L),        0.0f,                0.0f,       0.0f },
                        { 0.0f,                    2.0f / (T - B),     0.0f,       0.0f },
                        { 0.0f,                    0.0f,                0.5f,       0.0f },
                        { (R + L) / (L - R),    (T + B) / (B - T),  0.5f,       1.0f },
                    };
                    void* cb_resource = nullptr;
                    luexp(g_cb->map(0, 0, &cb_resource));
                    memcpy(cb_resource, &mvp, sizeof(Float4x4));
                    g_cb->unmap(0, sizeof(Float4x4));
                }

                Vector<TextureBarrier> barriers;
                barriers.push_back({ render_target, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::color_attachment_write, ResourceBarrierFlag::none });
                for (i32 n = 0; n < draw_data->CmdListsCount; ++n)
                {
                    const ImDrawList* cmd_list = draw_data->CmdLists[n];
                    for (i32 cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i)
                    {
                        const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                        object_t tid = (object_t)pcmd->TexRef.GetTexID();
                        ITexture* tex = query_interface<ITexture>(tid);
                        if(!tex)
                        {
                            ISampledImage* img = query_interface<ISampledImage>(tid);
                            tex = img->get_texture();
                        }
                        barriers.push_back({ tex, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::shader_read_ps, ResourceBarrierFlag::none });
                    }
                }
                cmd_buffer->begin_event("ImGui");
                cmd_buffer->resource_barrier({},
                    { barriers.data(), barriers.size() });

                RenderPassDesc desc;
                desc.color_attachments[0] = ColorAttachment(render_target, LoadOp::load, StoreOp::store);
                cmd_buffer->begin_render_pass(desc);

                cmd_buffer->set_viewport(Viewport(0.0f, 0.0f, fb_width, fb_height, 0.0f, 1.0f));
                VertexBufferView vbv = VertexBufferView(g_vb, 0, (u32)(g_vb_size * sizeof(ImDrawVert)), sizeof(ImDrawVert));
                cmd_buffer->set_vertex_buffers(0, { &vbv, 1 });
                cmd_buffer->set_index_buffer({g_ib, 0, (u32)(g_ib_size * sizeof(ImDrawIdx)), sizeof(ImDrawIdx) == 2 ? Format::r16_uint : Format::r32_uint});
                lulet(pso, get_pso(rt_desc.format));
                cmd_buffer->set_graphics_pipeline_state(pso);
                cmd_buffer->set_graphics_pipeline_layout(g_playout);
                cmd_buffer->set_blend_factor({ 0.f, 0.f, 0.f, 0.f });

                // Render command lists.
                i32 vtx_offset = 0;
                i32 idx_offset = 0;
                Float2 clip_off = { draw_data->DisplayPos.x, draw_data->DisplayPos.y };

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
                            Float2 clip_min = Float2((pcmd->ClipRect.x - clip_off.x), (pcmd->ClipRect.y - clip_off.y));
                            Float2 clip_max = Float2((pcmd->ClipRect.z - clip_off.x), (pcmd->ClipRect.w - clip_off.y));
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
                            usize cb_align = dev->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment;
                            object_t tex_object = (object_t)pcmd->TexRef.GetTexID();
                            ITexture* tex1 = query_interface<ITexture>(tex_object);
                            ISampledImage* sampled_tex = query_interface<ISampledImage>(tex_object);
                            if(tex1)
                            {
                                luexp(vs->update_descriptors({
                                    WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(g_cb)),
                                    WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(tex1)),
                                    WriteDescriptorSet::sampler(2, SamplerDesc(Filter::linear, Filter::linear, Filter::linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp))
                                    }));
                            }
                            else if(sampled_tex)
                            {
                                luexp(vs->update_descriptors({
                                    WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(g_cb)),
                                    WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(sampled_tex->get_texture())),
                                    WriteDescriptorSet::sampler(2, sampled_tex->get_sampler())
                                    }));
                            }
                            else
                            {
                                // should not get here.
                                lupanic();
                            }
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
                cmd_buffer->end_event();
            }
            lucatchret;
            return ok;
        }

        struct ImGuiModule : public Module
        {
            virtual const c8* get_name() override { return "ImGui"; }
            virtual RV on_register() override
            {
                return add_dependency_modules(this, {module_rhi(), module_rhi_utility(), module_hid(), module_font(), module_window()} );
            }
            virtual RV on_init() override
            {
                return init();
            }
            virtual void on_close() override
            {
                close();
            }
        };
    }

    LUNA_IMGUI_API Module* module_imgui()
    {
        static ImGuiUtils::ImGuiModule m;
        return &m;
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

    LUNA_IMGUI_API void Image(Luna::RHI::ITexture* texture, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1)
    {
        Image(ImTextureRef((ImTextureID)texture->get_object()), image_size, uv0, uv1);
    }
    LUNA_IMGUI_API void Image(Luna::ImGuiUtils::ISampledImage* texture, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1)
    {
        Image(ImTextureRef((ImTextureID)texture->get_object()), image_size, uv0, uv1);
    }
    LUNA_IMGUI_API bool ImageButton(const char* str_id, Luna::RHI::ITexture* texture, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& bg_col, const ImVec4& tint_col)
    {
        return ImageButton(str_id, ImTextureRef((ImTextureID)texture->get_object()), image_size, uv0, uv1, bg_col, tint_col);
    }
    LUNA_IMGUI_API bool ImageButton(const char* str_id, Luna::ImGuiUtils::ISampledImage* texture, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& bg_col, const ImVec4& tint_col)
    {
        return ImageButton(str_id, ImTextureRef((ImTextureID)texture->get_object()), image_size, uv0, uv1, bg_col, tint_col);
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
