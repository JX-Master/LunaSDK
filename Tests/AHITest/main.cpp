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
#include <Luna/ImGui/ImGui.hpp>
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
        Vector<Ref<AHI::IAdapter>> playback_adapters;
        Vector<Ref<AHI::IAdapter>> capture_adapters;
        Vector<AudioSource> audio_sources;
        Ref<AHI::IDevice> device;
        u32 width = 0;
        u32 height = 0;
    };

    RV run_app()
    {
        lutry
        {
            luexp(add_modules({module_ahi(), module_rhi(), module_window(), module_imgui()}));
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
            luset(app.swap_chain, dev->new_swap_chain(graphics_queue, app.window, RHI::SwapChainDesc({0, 0, 2, RHI::Format::bgra8_unorm, true})));
            luset(app.cmdbuf, dev->new_command_buffer(graphics_queue));
            
            luexp(AHI::get_adapters(&app.playback_adapters, &app.capture_adapters));

            ImGuiUtils::set_active_window(app.window);

            while(true)
            {
                // Handle events.
                while(ObjRef event = Window::pop_event())
                {
                    if(!ImGuiUtils::handle_window_event(event))
                    {
                        Window::default_event_handler(event.get());
                    }
                }
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
                    f32 clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
                    app.width = fb_sz.x;
                    app.height = fb_sz.y;
                }
                auto sz = app.window->get_size();
                ImGuiUtils::update_io();
                ImGui::NewFrame();
                {
                    using namespace ImGui;
                    SetNextWindowPos({ 0.0f, 0.0f });
                    SetNextWindowSize({ (f32)sz.x, (f32)sz.y });
                    Begin("AHITest", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

                    if(CollapsingHeader("Adapters and formats"))
                    {
                        Vector<const c8*> playback_adapter_names;
                        Vector<const c8*> capture_adapter_names;
                        playback_adapter_names.reserve(app.playback_adapters.size());
                        for(auto& adapter : app.playback_adapters)
                        {
                            playback_adapter_names.push_back(adapter->get_name());
                        }
                        for(auto& adapter : app.capture_adapters)
                        {
                            capture_adapter_names.push_back(adapter->get_name());
                        }
                        static int current_playback_adapter;
                        static int current_capture_adapter;
                        Combo("Playback Adapters", &current_playback_adapter, playback_adapter_names.data(), (int)playback_adapter_names.size());
                        Combo("Capture Adapters", &current_capture_adapter, capture_adapter_names.data(), (int)capture_adapter_names.size());
                        if(current_playback_adapter < app.playback_adapters.size() && current_capture_adapter < app.capture_adapters.size())
                        {
                            if(!app.device && Button("Create Device"))
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
                        }
                        if(app.device && CollapsingHeader("Device"))
                        {
                            {
                                auto bd = app.device->get_playback_bit_depth();
                                const c8* bit_depth;
                                switch(bd)
                                {
                                    case AHI::BitDepth::u8: bit_depth = "8bit"; break;
                                    case AHI::BitDepth::s16: bit_depth = "16bit"; break;
                                    case AHI::BitDepth::s24: bit_depth = "24bit"; break;
                                    case AHI::BitDepth::s32: bit_depth = "32bit"; break;
                                    case AHI::BitDepth::f32: bit_depth = "32bit(float)"; break;
                                    default: break;
                                }
                                Text("Playback: %s, %uHz, %u channels", bit_depth, app.device->get_sample_rate(), app.device->get_playback_num_channels());
                                bd = app.device->get_capture_bit_depth();
                                switch(bd)
                                {
                                    case AHI::BitDepth::u8: bit_depth = "8bit"; break;
                                    case AHI::BitDepth::s16: bit_depth = "16bit"; break;
                                    case AHI::BitDepth::s24: bit_depth = "24bit"; break;
                                    case AHI::BitDepth::s32: bit_depth = "32bit"; break;
                                    case AHI::BitDepth::f32: bit_depth = "32bit(float)"; break;
                                    default: break;
                                }
                                Text("Capture: %s, %uHz, %u channels", bit_depth, app.device->get_sample_rate(), app.device->get_capture_num_channels());
                            }
                            SetNextItemWidth(200.0f);
                            SliderFloat("Input Audio Level", &input_audio_level, 0.0f, 1.0f);
                            if(Button("Add Audio Source"))
                            {
                                AudioSource source;
                                app.audio_sources.push_back(source);
                            }
                            for (auto& source : app.audio_sources)
                            {
                                PushID(&source);
                                Text("Audio Source");
                                SameLine();
                                SetNextItemWidth(100.0f);
                                DragFloat("Frequency", &source.frequency, 1.0f, 8.176f, 15804.266f);
                                SameLine();
                                SetNextItemWidth(100.0f);
                                SliderFloat("Volume", &source.volume, 0.0f, 1.0f);
                                SameLine();
                                if (Button("Apply"))
                                {
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
                                PopID();
                            }
                        }
                    }
                    End();
                }
                ImGui::Render();
                Float4U clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };

                RHI::RenderPassDesc render_pass;
                lulet(back_buffer, app.swap_chain->get_current_back_buffer());
                render_pass.color_attachments[0] = RHI::ColorAttachment(back_buffer, RHI::LoadOp::clear, RHI::StoreOp::store, clear_color);
                app.cmdbuf->begin_render_pass(render_pass);
                app.cmdbuf->end_render_pass();
                luexp(ImGuiUtils::render_draw_data(ImGui::GetDrawData(), app.cmdbuf, back_buffer));
                app.cmdbuf->resource_barrier({}, {
                    {back_buffer, RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES, RHI::TextureStateFlag::automatic, RHI::TextureStateFlag::present, RHI::ResourceBarrierFlag::none}
                    });
                luexp(app.cmdbuf->submit({}, {}, true));
                app.cmdbuf->wait();
                luexp(app.cmdbuf->reset());
                luexp(app.swap_chain->present());
            }
        }
        lucatchret;
        return ok;
    }

    int luna_main(int argc, const char** argv)
    {
        if(!Luna::init())
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
}