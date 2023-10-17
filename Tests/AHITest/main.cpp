/*!
* This file is a portion of Luna SDK.
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
            }
        }
        time += (f32)num_frames / (f32)format.sample_rate;
        return num_frames;
    }
};

struct AudioSource
{
    Ref<AHI::IAudioSource> audio_source;
    f32 frequency = 261.626;
    f32 volume = 0.1f;
};

RV run()
{
    lutry
    {
        luexp(init_modules());
        lulet(window, Window::new_window("Luna Studio - Open Project", Window::WindowDisplaySettings::as_windowed(Window::DEFAULT_POS, Window::DEFAULT_POS, 1000, 500)));
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
        lulet(swap_chain, dev->new_swap_chain(graphics_queue, window, RHI::SwapChainDesc({0, 0, 2, RHI::Format::bgra8_unorm, true})));
		lulet(cmdbuf, dev->new_command_buffer(graphics_queue));
        window->get_close_event().add_handler([](Window::IWindow* window) { window->close(); });

        Vector<Ref<AHI::IAdapter>> audio_adapters;
        Ref<AHI::IDevice> device;
        luexp(AHI::get_adapters(&audio_adapters, nullptr));

        Vector<AudioSource> audio_sources;

        // Create back buffer.
		u32 w = 0, h = 0;
		// Create ImGui context.
		ImGuiUtils::set_active_window(window);
        while(true)
        {
            Window::poll_events();
			if (window->is_closed())
			{
				break;
			}
            if (window->is_minimized())
			{
				sleep(100);
				continue;
			}
            // Recreate the back buffer if needed.
			auto fb_sz = window->get_framebuffer_size();
			if (fb_sz.x && fb_sz.y && (fb_sz.x != w || fb_sz.y != h))
			{
				luexp(swap_chain->reset({fb_sz.x, fb_sz.y, 2, RHI::Format::unknown, true}));
				f32 clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
				w = fb_sz.x;
				h = fb_sz.y;
			}
            auto sz = window->get_size();
			ImGuiUtils::update_io();
			ImGui::NewFrame();
            {
                using namespace ImGui;
				SetNextWindowPos({ 0.0f, 0.0f });
				SetNextWindowSize({ (f32)sz.x, (f32)sz.y });
				Begin("AHITest", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

                if(CollapsingHeader("Adapters and formats"))
                {
                    Vector<const c8*> adapter_names;
                    adapter_names.reserve(audio_adapters.size());
                    for(auto& adapter : audio_adapters)
                    {
                        adapter_names.push_back(adapter->get_name());
                    }
                    static int current_adapter;
                    Combo("Adapters", &current_adapter, adapter_names.data(), (int)adapter_names.size());
                    if(current_adapter < audio_adapters.size())
                    {
                        usize num_formats;
                        luexp(audio_adapters[current_adapter]->get_native_wave_formats(nullptr, &num_formats));
                        Vector<AHI::WaveFormat> formats;
                        static int current_format;
                        if(num_formats)
                        {
                            Vector<Name> format_names;
                            Vector<const c8*> format_strs;
                            formats.resize(num_formats);
                            format_names.reserve(num_formats);
                            format_strs.reserve(num_formats);
                            luexp(audio_adapters[current_adapter]->get_native_wave_formats(formats.data(), &num_formats));
                            for(auto& format : formats)
                            {
                                const c8* bit_depth;
                                switch(format.bit_depth)
                                {
                                    case AHI::BitDepth::u8: bit_depth = "8bit"; break;
                                    case AHI::BitDepth::s16: bit_depth = "16bit"; break;
                                    case AHI::BitDepth::s24: bit_depth = "24bit"; break;
                                    case AHI::BitDepth::s32: bit_depth = "32bit"; break;
                                    case AHI::BitDepth::f32: bit_depth = "32bit(float)"; break;
                                }
                                c8 buf[128];
                                snprintf(buf, 128, "%s, %uHz, %u channels", bit_depth, format.sample_rate, format.num_channels);
                                format_names.push_back(buf);
                                format_strs.push_back(format_names.back().c_str());
                            }
                            Combo("Formats", &current_format, format_strs.data(), (int)format_strs.size());
                        }
                        else
                        {
                            Text("No native audio formats.");
                        }
                        if(!device && Button("Create Device"))
                        {
                            AHI::DeviceDesc desc;
                            desc.flags = AHI::DeviceFlag::playback;
                            desc.playback.adapter = audio_adapters[current_adapter];
                            if(num_formats)
                            {
                                desc.sample_rate = formats[current_format].sample_rate;
                                desc.playback.bit_depth = formats[current_format].bit_depth;
                                desc.playback.num_channels = 2;
                            }
                            else
                            {
                                desc.sample_rate = 0;
                                desc.playback.bit_depth = AHI::BitDepth::unspecified;
                                desc.playback.num_channels = 2;
                            }
                            luset(device, AHI::new_device(desc));
                        }
                    }
                    if(device && CollapsingHeader("Device"))
                    {
                        {
                            auto bd = device->get_playback_bit_depth();
                            const c8* bit_depth;
                            switch(bd)
                            {
                                case AHI::BitDepth::u8: bit_depth = "8bit"; break;
                                case AHI::BitDepth::s16: bit_depth = "16bit"; break;
                                case AHI::BitDepth::s24: bit_depth = "24bit"; break;
                                case AHI::BitDepth::s32: bit_depth = "32bit"; break;
                                case AHI::BitDepth::f32: bit_depth = "32bit(float)"; break;
                            }
                            Text("%s, %uHz, %u channels", bit_depth, device->get_sample_rate(), device->get_playback_num_channels());
                        }
                        if(Button("Add Audio Source"))
                        {
                            Ref<AHI::IAudioSource> audio_source = AHI::new_audio_source();
                            device->add_audio_source(audio_source);
                            AudioSource source;
                            source.audio_source = audio_source;
                            audio_sources.push_back(source);
                        }
                        for (auto& source : audio_sources)
                        {
                            PushID(source.audio_source.get());
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
                                source.audio_source->set_data_callback(callback);
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
			lulet(back_buffer, swap_chain->get_current_back_buffer());
			render_pass.color_attachments[0] = RHI::ColorAttachment(back_buffer, RHI::LoadOp::clear, RHI::StoreOp::store, clear_color);
			cmdbuf->begin_render_pass(render_pass);
			cmdbuf->end_render_pass();
			luexp(ImGuiUtils::render_draw_data(ImGui::GetDrawData(), cmdbuf, back_buffer));
			cmdbuf->resource_barrier({}, {
				{back_buffer, RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES, RHI::TextureStateFlag::automatic, RHI::TextureStateFlag::present, RHI::ResourceBarrierFlag::none}
				});
			luexp(cmdbuf->submit({}, {}, true));
			cmdbuf->wait();
			luexp(cmdbuf->reset());
			luexp(swap_chain->present());
        }
    }
    lucatchret;
    return ok;
}

int main()
{
    Luna::init();
    set_log_to_platform_enabled(true);
    auto r = run();
    if(failed(r))
    {
        log_error("AHITest", "%s", explain(r.errcode()));
        Luna::close();
        return -1;
    }
    Luna::close();
    return 0;
}