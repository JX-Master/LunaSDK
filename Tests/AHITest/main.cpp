/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.cpp
* @author JXMaster
* @date 2023/10/17
*/
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Window/Window.hpp>
#include <Luna/RHI/SwapChain.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/EditorGUI/EditorGUI.hpp>
#include <Luna/GUIWindow/GUIWindow.hpp>
#include <Luna/Font/Font.hpp>
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/VG/ShapeRenderer.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/AHI/Device.hpp>
#include <Luna/AHI/Adapter.hpp>
#include <Luna/AHI/AHI.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Window/Event.hpp>

#include <Luna/Window/AppMain.hpp>
using namespace Luna;

inline f32 gen_sine_wave(f32 time, f32 freq, f32 amp)
{
    return sinf(time * freq * 2 * PI) * amp;
}
inline void* write_u8(void* dst_buffer, f32 sample, u32 num_channels)
{
    u8* dst = (u8*)dst_buffer;
    u8 value = (u8)((sample + 1.0f) * 255.0f);
    for(u32 i = 0; i < num_channels; ++i)
    {
        dst[i] = value;
    }
    return dst + num_channels;
}
inline void* write_s16(void* dst_buffer, f32 sample, u32 num_channels)
{
    i16* dst = (i16*)dst_buffer;
    i16 value = (i16)(sample * 32767.0f);
    for(u32 i = 0; i < num_channels; ++i)
    {
        dst[i] = value;
    }
    return dst + num_channels;
}
inline void* write_s24(void* dst_buffer, f32 sample, u32 num_channels)
{
    u8* dst = (u8*)dst_buffer;
    i32 value = (i32)(sample * 8388607.0f);
    for(u32 i = 0; i < num_channels; ++i)
    {
#ifdef LUNA_PLATFORM_LITTLE_ENDIAN
        dst[i * 3] = (u8)value;
        dst[i * 3 + 1] = (u8)(value >> 8);
        dst[i * 3 + 2] = (u8)((value >> 16) & 0x7F) + (u8)(value < 0 ? 0x80 : 0);
#else
        dst[i * 3] = (u8)((value >> 16) & 0x7F) + (u8)(value < 0 ? 0x80 : 0);
        dst[i * 3 + 1] = (u8)(value >> 8);
        dst[i * 3 + 2] = (u8)value;
#endif
    }
    return dst + num_channels * 3;
}
inline void* write_s32(void* dst_buffer, f32 sample, u32 num_channels)
{
    i32* dst = (i32*)dst_buffer;
    i32 value = (i32)(((f64)sample) * 2147483647.0);
    for(u32 i = 0; i < num_channels; ++i)
    {
        dst[i] = value;
    }
    return dst + num_channels;
}
inline void* write_f32(void* dst_buffer, f32 sample, u32 num_channels)
{
    f32* dst = (f32*)dst_buffer;
    for(u32 i = 0; i < num_channels; ++i)
    {
        dst[i] = sample;
    }
    return dst + num_channels;
}

struct AudioSourceCallback
{
    f32 time;
    f32 freq;
    f32 amp;
    u32 operator()(void* dst_buffer, const AHI::WaveFormat& format, u32 num_frames)
    {
        for(u32 i = 0; i < num_frames; ++i)
        {
            f32 sample = clamp(gen_sine_wave(time + (f32)i / (f32)format.sample_rate, freq, amp), -1.0f, 1.0f);
            switch(format.bit_depth)
            {
                case AHI::BitDepth::u8:
                dst_buffer = write_u8(dst_buffer, sample, format.num_channels);
                break;
                case AHI::BitDepth::s16:
                dst_buffer = write_s16(dst_buffer, sample, format.num_channels);
                break;
                case AHI::BitDepth::s24:
                dst_buffer = write_s24(dst_buffer, sample, format.num_channels);
                break;
                case AHI::BitDepth::s32:
                dst_buffer = write_s32(dst_buffer, sample, format.num_channels);
                break;
                case AHI::BitDepth::f32:
                dst_buffer = write_f32(dst_buffer, sample, format.num_channels);
                break;
                default: break;
            }
        }
        time += (f32)num_frames / (f32)format.sample_rate;
        return num_frames;
    }
};

struct AudioSource
{
    usize audio_source = USIZE_MAX;
    f32 frequency = 261.626;
    f32 volume = 0.1f;
};

f32 input_audio_level = 0.0f;

void on_capture_data(const void* src_buffer, const AHI::WaveFormat& format, u32 num_frames)
{
    f32 sample = 0;
    for(u32 i = 0; i < num_frames; ++i)
    {
        switch(format.bit_depth)
        {
        case AHI::BitDepth::u8:
        {
            const u8* src = (const u8*)src_buffer;
            for(u32 c = 0; c < format.num_channels; ++c)
            {
                sample = max(sample, (f32)src[c] / 255.0f);
            }
            src += format.num_channels;
            src_buffer = src;
        }
        break;
        case AHI::BitDepth::s16:
        {
            const i16* src = (const i16*)src_buffer;
            for(u32 c = 0; c < format.num_channels; ++c)
            {
                sample = max(sample, (f32)src[c] / 32767.0f);
            }
            src += format.num_channels;
            src_buffer = src;
        }
        break;
        case AHI::BitDepth::s24:
        {
            const u8* src = (const u8*)src_buffer;
            for(u32 c = 0; c < format.num_channels; ++c)
            {
                i32 data;
#ifdef LUNA_PLATFORM_LITTLE_ENDIAN
                data = ((i32)(src[0])) + (((i32)src[1]) << 8) + (((i32)(src[2] & 0x7F)) << 16);
                data = (src[2] & 0x80) ? -data : data;
#else
                data = ((i32)(src[2])) + ((i32)src[1] << 8) + (((i32)(src[0] & 0x7F)) << 16);
                data = (src[0] & 0x80) ? -data : data;
#endif
                sample = max(sample, (f32)data / 8388607.0f);
                src += 3;
            }
            src_buffer = src;
        }
        break;
        case AHI::BitDepth::s32:
        {
            const i32* src = (const i32*)src_buffer;
            for(u32 c = 0; c < format.num_channels; ++c)
            {
                sample = max(sample, (f32)src[c] / 2147483647.0f);
            }
            src += format.num_channels;
            src_buffer = src;
        }
        break;
        case AHI::BitDepth::f32:
        {
            const f32* src = (const f32*)src_buffer;
            for(u32 c = 0; c < format.num_channels; ++c)
            {
                sample = max(sample, src[c]);
            }
            src += format.num_channels;
            src_buffer = src;
        }
        break;
        default: break;
        }
    }
    input_audio_level = sample;
}

namespace Luna
{
    struct App
    {
        Ref<Window::IWindow> window;
        Ref<RHI::ISwapChain> swap_chain;
        Ref<RHI::ICommandBuffer> cmdbuf;
        Ref<GUI::IContext> gui;
        Ref<GUI::IRenderer> gui_renderer;
        Vector<Ref<AHI::IAdapter>> playback_adapters;
        Vector<Ref<AHI::IAdapter>> capture_adapters;
        Vector<AudioSource> audio_sources;
        Ref<AHI::IDevice> device;
        u32 width = 0;
        u32 height = 0;
    };

    constexpr GUI::id_t DEFAULT_LAYER_ID = 1;
    constexpr GUI::id_t ROOT_ID = 2;
    constexpr GUI::id_t ADAPTERS_HEADER_ID = 10;
    constexpr GUI::id_t PLAYBACK_COMBO_ID = 11;
    constexpr GUI::id_t CAPTURE_COMBO_ID = 12;
    constexpr GUI::id_t CREATE_DEVICE_BUTTON_ID = 13;
    constexpr GUI::id_t DEVICE_HEADER_ID = 20;
    constexpr GUI::id_t INPUT_LEVEL_SLIDER_ID = 21;
    constexpr GUI::id_t ADD_SOURCE_BUTTON_ID = 22;
    constexpr GUI::id_t FIRST_TEXT_ID = 1000;
    constexpr GUI::id_t FIRST_SOURCE_ID = 2000;
    constexpr GUI::id_t SOURCE_ID_STRIDE = 16;

    inline GUI::LayoutConfig fixed_layout(f32 width, f32 height)
    {
        GUI::LayoutConfig layout;
        layout.width.kind = GUI::SizeKind::fixed;
        layout.width.value = width;
        layout.height.kind = GUI::SizeKind::fixed;
        layout.height.value = height;
        return layout;
    }

    inline void set_element_rect(GUI::IContext* context, const GUI::ElementHandle& element, const RectF& rect)
    {
        GUI::LayoutResult layout;
        layout.rect = rect;
        layout.clip_rect = rect;
        layout.content_size = Float2U(rect.width, rect.height);
        context->set_layout_result(element, layout);
    }

    inline bool clicked(GUI::IContext* context, GUI::id_t id)
    {
        return context->get_interaction_state(id).clicked;
    }

    inline Float4U style_color(GUI::IContext* context, const c8* entry, const Float4U& fallback)
    {
        return context->get_style_value(Name(EditorGUI::DEFAULT_STYLE_NAME), Name(entry),
            GUI::style_f32x4(fallback)).number;
    }

    void draw_label(GUI::IContext* context, GUI::id_t id, const RectF& rect, const c8* text)
    {
        EditorGUI::TextDesc desc;
        desc.font_size = 16.0f;
        GUI::ElementHandle element = EditorGUI::text(context, id, text ? text : "",
            fixed_layout(rect.width, rect.height), desc);
        set_element_rect(context, element, rect);
    }

    void draw_solid_rect(GUI::IContext* context, const RectF& rect, const Float4U& color)
    {
        GUI::DrawCommand command;
        command.type = GUI::DrawCommandType::rect;
        command.rect = rect;
        command.color = color;
        context->draw(command);
    }

    GUI::ElementHandle build_gui(App& app, const Float2U& surface_size,
        i32& current_playback_adapter, i32& current_capture_adapter)
    {
        GUI::IContext* context = app.gui;
        GUI::ElementHandle root = context->begin_element(ROOT_ID);
        set_element_rect(context, root, RectF(0.0f, 0.0f, surface_size.x, surface_size.y));
        draw_solid_rect(context, RectF(0.0f, 0.0f, surface_size.x, surface_size.y),
            style_color(context, "gui.canvas", Float4U(0.92f, 0.93f, 0.92f, 1.0f)));

        f32 y = 16.0f;
        draw_label(context, FIRST_TEXT_ID, RectF(18.0f, y, 260.0f, 28.0f), "AHI Test");
        y += 38.0f;

        GUI::ElementHandle adapters_header;
        bool show_adapters = EditorGUI::collapsing_header(context, ADAPTERS_HEADER_ID, "Adapters and formats",
            fixed_layout(max(surface_size.x - 36.0f, 300.0f), 30.0f), EditorGUI::DisclosureDesc(), &adapters_header);
        set_element_rect(context, adapters_header, RectF(18.0f, y, max(surface_size.x - 36.0f, 300.0f), 30.0f));
        y += 38.0f;
        if(show_adapters)
        {
            Vector<const c8*> playback_adapter_names;
            Vector<const c8*> capture_adapter_names;
            playback_adapter_names.reserve(app.playback_adapters.size());
            capture_adapter_names.reserve(app.capture_adapters.size());
            for(auto& adapter : app.playback_adapters)
            {
                playback_adapter_names.push_back(adapter->get_name());
            }
            for(auto& adapter : app.capture_adapters)
            {
                capture_adapter_names.push_back(adapter->get_name());
            }
            draw_label(context, FIRST_TEXT_ID + 1, RectF(32.0f, y, 160.0f, 28.0f), "Playback Adapter");
            GUI::ElementHandle playback_combo = EditorGUI::combo(context, PLAYBACK_COMBO_ID, "Playback Adapters",
                &current_playback_adapter, Span<const c8*>(playback_adapter_names.data(), playback_adapter_names.size()),
                fixed_layout(max(surface_size.x - 240.0f, 240.0f), 30.0f));
            set_element_rect(context, playback_combo, RectF(198.0f, y, max(surface_size.x - 240.0f, 240.0f), 30.0f));
            y += 38.0f;

            draw_label(context, FIRST_TEXT_ID + 2, RectF(32.0f, y, 160.0f, 28.0f), "Capture Adapter");
            GUI::ElementHandle capture_combo = EditorGUI::combo(context, CAPTURE_COMBO_ID, "Capture Adapters",
                &current_capture_adapter, Span<const c8*>(capture_adapter_names.data(), capture_adapter_names.size()),
                fixed_layout(max(surface_size.x - 240.0f, 240.0f), 30.0f));
            set_element_rect(context, capture_combo, RectF(198.0f, y, max(surface_size.x - 240.0f, 240.0f), 30.0f));
            y += 42.0f;

            if(!app.device && (usize)current_playback_adapter < app.playback_adapters.size() &&
                (usize)current_capture_adapter < app.capture_adapters.size())
            {
                GUI::ElementHandle create_device = EditorGUI::text_button(context, CREATE_DEVICE_BUTTON_ID, "Create Device",
                    fixed_layout(150.0f, 32.0f));
                set_element_rect(context, create_device, RectF(32.0f, y, 150.0f, 32.0f));
                y += 44.0f;
            }

            if(app.device)
            {
                GUI::ElementHandle device_header;
                bool show_device = EditorGUI::collapsing_header(context, DEVICE_HEADER_ID, "Device",
                    fixed_layout(max(surface_size.x - 64.0f, 300.0f), 30.0f), EditorGUI::DisclosureDesc(), &device_header);
                set_element_rect(context, device_header, RectF(32.0f, y, max(surface_size.x - 64.0f, 300.0f), 30.0f));
                y += 38.0f;
                if(show_device)
                {
                    auto bd = app.device->get_playback_bit_depth();
                    const c8* bit_depth = "unknown";
                    switch(bd)
                    {
                    case AHI::BitDepth::u8: bit_depth = "8bit"; break;
                    case AHI::BitDepth::s16: bit_depth = "16bit"; break;
                    case AHI::BitDepth::s24: bit_depth = "24bit"; break;
                    case AHI::BitDepth::s32: bit_depth = "32bit"; break;
                    case AHI::BitDepth::f32: bit_depth = "32bit(float)"; break;
                    default: break;
                    }
                    String text;
                    strprintf(text, "Playback: %s, %uHz, %u channels", bit_depth, app.device->get_sample_rate(), app.device->get_playback_num_channels());
                    draw_label(context, FIRST_TEXT_ID + 3, RectF(48.0f, y, 520.0f, 28.0f), text.c_str());
                    y += 30.0f;
                    bd = app.device->get_capture_bit_depth();
                    switch(bd)
                    {
                    case AHI::BitDepth::u8: bit_depth = "8bit"; break;
                    case AHI::BitDepth::s16: bit_depth = "16bit"; break;
                    case AHI::BitDepth::s24: bit_depth = "24bit"; break;
                    case AHI::BitDepth::s32: bit_depth = "32bit"; break;
                    case AHI::BitDepth::f32: bit_depth = "32bit(float)"; break;
                    default: bit_depth = "unknown"; break;
                    }
                    strprintf(text, "Capture: %s, %uHz, %u channels", bit_depth, app.device->get_sample_rate(), app.device->get_capture_num_channels());
                    draw_label(context, FIRST_TEXT_ID + 4, RectF(48.0f, y, 520.0f, 28.0f), text.c_str());
                    y += 38.0f;

                    draw_label(context, FIRST_TEXT_ID + 5, RectF(48.0f, y, 150.0f, 28.0f), "Input Audio Level");
                    GUI::ElementHandle level_slider = EditorGUI::slider_float(context, INPUT_LEVEL_SLIDER_ID,
                        &input_audio_level, 0.0f, 1.0f, fixed_layout(max(surface_size.x - 250.0f, 220.0f), 26.0f));
                    set_element_rect(context, level_slider, RectF(210.0f, y + 2.0f, max(surface_size.x - 250.0f, 220.0f), 26.0f));
                    y += 42.0f;

                    GUI::ElementHandle add_source = EditorGUI::text_button(context, ADD_SOURCE_BUTTON_ID, "Add Audio Source",
                        fixed_layout(170.0f, 32.0f));
                    set_element_rect(context, add_source, RectF(48.0f, y, 170.0f, 32.0f));
                    y += 46.0f;

                    if(!app.audio_sources.empty())
                    {
                        f32 table_x = 48.0f;
                        f32 table_w = max(surface_size.x - 96.0f, 520.0f);
                        f32 freq_w = max((table_w - 240.0f) * 0.5f, 180.0f);
                        f32 volume_w = max(table_w - 220.0f - freq_w, 180.0f);
                        draw_label(context, FIRST_TEXT_ID + 6, RectF(table_x, y, 140.0f, 24.0f), "Audio Source");
                        draw_label(context, FIRST_TEXT_ID + 7, RectF(table_x + 130.0f, y, 140.0f, 24.0f), "Frequency");
                        draw_label(context, FIRST_TEXT_ID + 8, RectF(table_x + 140.0f + freq_w, y, 140.0f, 24.0f), "Volume");
                        y += 30.0f;
                        for(usize i = 0; i < app.audio_sources.size(); ++i)
                        {
                            AudioSource& source = app.audio_sources[i];
                            f32 row_y = y + (f32)i * 38.0f;
                            Float4U row_color = (i % 2) ?
                                style_color(context, "gui.surface.1", Float4U(0.97f, 0.97f, 0.96f, 1.0f)) :
                                style_color(context, "gui.surface.0", Float4U(0.95f, 0.95f, 0.94f, 1.0f));
                            draw_solid_rect(context, RectF(table_x, row_y, table_w, 34.0f), row_color);
                            draw_label(context, FIRST_SOURCE_ID + (GUI::id_t)i * SOURCE_ID_STRIDE + 1,
                                RectF(table_x + 8.0f, row_y + 4.0f, 120.0f, 26.0f), "Audio Source");
                            EditorGUI::DragDesc drag_desc;
                            drag_desc.speed = 1.0f;
                            GUI::ElementHandle frequency = EditorGUI::drag_float(context,
                                FIRST_SOURCE_ID + (GUI::id_t)i * SOURCE_ID_STRIDE + 2,
                                &source.frequency, 8.176f, 15804.266f,
                                fixed_layout(freq_w - 12.0f, 26.0f), drag_desc);
                            set_element_rect(context, frequency, RectF(table_x + 130.0f, row_y + 4.0f, freq_w - 12.0f, 26.0f));
                            GUI::ElementHandle volume = EditorGUI::slider_float(context,
                                FIRST_SOURCE_ID + (GUI::id_t)i * SOURCE_ID_STRIDE + 3,
                                &source.volume, 0.0f, 1.0f, fixed_layout(volume_w - 92.0f, 26.0f));
                            set_element_rect(context, volume, RectF(table_x + 140.0f + freq_w, row_y + 4.0f, volume_w - 92.0f, 26.0f));
                            GUI::ElementHandle apply = EditorGUI::text_button(context,
                                FIRST_SOURCE_ID + (GUI::id_t)i * SOURCE_ID_STRIDE + 4,
                                "Apply", fixed_layout(72.0f, 28.0f));
                            set_element_rect(context, apply,
                                RectF(table_x + table_w - 80.0f, row_y + 3.0f, 72.0f, 28.0f));
                        }
                    }
                }
            }
        }
        context->end_element();
        return root;
    }

    RV run_app()
    {
        lutry
        {
            luexp(add_modules({
                module_ahi(),
                module_rhi(),
                module_window(),
                module_font(),
                module_vg(),
                GUI::module_gui(),
                EditorGUI::module_editor_gui(),
                GUIWindow::module_gui_window()}));
            luexp(init_modules());

            App app;

            luset(app.window, Window::new_window("Luna Studio - Open Project", Window::DEFAULT_POS, Window::DEFAULT_POS, 1000, 500));
            auto dev = RHI::get_main_device();
            u32 graphics_queue = U32_MAX;
            {
                u32 num_queues = dev->get_num_command_queues();
                for (u32 i = 0; i < num_queues; ++i)
                {
                    auto desc = dev->get_command_queue_desc(i);
                    if (desc.type == RHI::CommandQueueType::graphics)
                    {
                        graphics_queue = i;
                        break;
                    }
                }
            }
            luset(app.swap_chain, dev->new_swap_chain(graphics_queue, app.window,
                RHI::SwapChainDesc({0, 0, 2, RHI::Format::bgra8_unorm, true, RHI::ColorSpace::srgb})));
            luset(app.cmdbuf, dev->new_command_buffer(graphics_queue));
            luset(app.gui_renderer, GUI::new_renderer(dev));
            app.gui = GUI::new_context();
            EditorGUI::register_style_schemas(app.gui);
            luexp(app.gui->register_font(Name("default"), Font::get_default_font()));
            EditorGUI::DefaultStyleDesc style_desc;
            style_desc.input_mode = EditorGUI::InputMode::pointer;
            EditorGUI::set_default_style(app.gui, style_desc);
            GUIWindow::GUIWindowInputAdapter input_adapter;
            input_adapter.window = app.window;
            input_adapter.gui = app.gui;
            GUIWindow::install_window_event_handler(&input_adapter);
            
            luexp(AHI::get_adapters(&app.playback_adapters, &app.capture_adapters));

            while(true)
            {
                // Handle events.
                Window::poll_events();
                if (app.window->is_closed()) break;
                if (app.window->is_minimized())
                {
                    sleep(100);
                    continue;
                }
                // Recreate the back buffer if needed.
                auto fb_sz = app.window->get_framebuffer_size();
                if (fb_sz.x && fb_sz.y && (fb_sz.x != app.width || fb_sz.y != app.height))
                {
                    luexp(app.swap_chain->reset({fb_sz.x, fb_sz.y, 2, RHI::Format::unknown, true}));
                    app.width = fb_sz.x;
                    app.height = fb_sz.y;
                }
                auto sz = app.window->get_size();

                GUI::FrameDesc frame;
                frame.logical_size = Float2U((f32)sz.x, (f32)sz.y);
                frame.render_size = fb_sz;
                frame.delta_time = 1.0f / 60.0f;
                app.gui->begin_frame(frame);
                GUIWindow::update_input(&input_adapter);

                static i32 current_playback_adapter = 0;
                static i32 current_capture_adapter = 0;
                app.gui->push_layer(DEFAULT_LAYER_ID, Float2U(0.0f));
                GUI::ElementHandle root = build_gui(app, frame.logical_size,
                    current_playback_adapter, current_capture_adapter);
                app.gui->pop_layer();
                RectF screen_rect(0.0f, 0.0f, frame.logical_size.x, frame.logical_size.y);
                luexp(EditorGUI::layout_tree(app.gui, root, screen_rect));
                app.gui->route_input();
                EditorGUI::ResolveResult resolved = EditorGUI::resolve_interactions(app.gui);
                if(resolved.relayout_requested)
                {
                    luexp(EditorGUI::layout_tree(app.gui, root, screen_rect));
                }
                luexp(GUIWindow::update_text_input(&input_adapter));

                if(clicked(app.gui, CREATE_DEVICE_BUTTON_ID))
                {
                    AHI::DeviceDesc desc;
                    desc.flags = AHI::DeviceFlag::playback | AHI::DeviceFlag::capture;
                    desc.sample_rate = 0;
                    desc.playback.adapter = app.playback_adapters[current_playback_adapter];
                    desc.playback.bit_depth = AHI::BitDepth::unspecified;
                    desc.playback.num_channels = 2;
                    desc.capture.adapter = app.capture_adapters[current_capture_adapter];
                    desc.capture.bit_depth = AHI::BitDepth::unspecified;
                    desc.capture.num_channels = 1;
                    luset(app.device, AHI::new_device(desc));
                    app.device->add_capture_data_callback(on_capture_data);
                }
                if(clicked(app.gui, ADD_SOURCE_BUTTON_ID))
                {
                    AudioSource source;
                    app.audio_sources.push_back(source);
                }
                for(usize i = 0; i < app.audio_sources.size(); ++i)
                {
                    GUI::id_t apply_id = FIRST_SOURCE_ID + (GUI::id_t)i * SOURCE_ID_STRIDE + 4;
                    if(clicked(app.gui, apply_id))
                    {
                        AudioSource& source = app.audio_sources[i];
                        AudioSourceCallback callback;
                        callback.time = 0.0f;
                        callback.freq = source.frequency;
                        callback.amp = source.volume;
                        if(source.audio_source != USIZE_MAX)
                        {
                            app.device->remove_playback_data_callback(source.audio_source);
                        }
                        source.audio_source = app.device->add_playback_data_callback(callback);
                    }
                }

                Float4U clear_color = style_color(app.gui, "gui.canvas",
                    Float4U(0.92f, 0.93f, 0.92f, 1.0f));

                lulet(back_buffer, app.swap_chain->get_current_back_buffer());
                GUI::RenderTargetDesc target(back_buffer);
                target.color_load_op = RHI::LoadOp::clear;
                target.color_clear_value = clear_color;
                target.color_final_state = RHI::TextureStateFlag::present;
                luexp(app.gui_renderer->render(app.gui, app.cmdbuf, target));
                luexp(app.cmdbuf->submit({}, {}, true));
                app.cmdbuf->wait();
                luexp(app.cmdbuf->reset());
                luexp(app.swap_chain->present());
            }
            GUIWindow::uninstall_window_event_handler(&input_adapter);
        }
        lucatchret;
        return ok;
    }
}

using namespace Luna;

int luna_main(int argc, const char** argv)
{
    if(Luna::failed(Luna::init()))
    {
        return -1;
    }
    RV r = run_app();
    if(failed(r))
    {
        log_error("AHITest", "%s", explain(r.errcode()));
        Luna::close();
        return -1;
    }
    Luna::close();
    return 0;
}
