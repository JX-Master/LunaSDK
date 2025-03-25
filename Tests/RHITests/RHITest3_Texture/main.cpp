/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.cpp
* @author JXMaster
* @date 2022/8/2
*/
#include "../RHITestBed/RHITestBed.hpp"
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/RHI/ShaderCompileHelper.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/Image/Image.hpp>
#include <Luna/RHI/Utility.hpp>
#include <TestTextureVS.hpp>
#include <TestTexturePS.hpp>
#include <Luna/Runtime/Thread.hpp>

#include <Luna/Window/AppMain.hpp>

using namespace Luna;
using namespace Luna::RHI;
using namespace Luna::RHITestBed;

Ref<RHI::IDescriptorSetLayout> desc_set_layout;
Ref<RHI::IPipelineLayout> pipeline_layout;
Ref<RHI::IDescriptorSet> desc_set;
Ref<RHI::IPipelineState> pso;
Ref<RHI::ITexture> tex;

Ref<RHI::IBuffer> vb;
Ref<RHI::IBuffer> ib;

u32 tex_width;
u32 tex_height;

struct VertexData
{
    Float2U pos;
    Float2U texcoord;
};

RV start()
{
    lutry
    {
        Path p = get_process_path();
        p.pop_back();
        luexp(set_current_dir(p.encode().c_str()));
        auto device = get_main_device();
        // create pso
        {
            luset(desc_set_layout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
                DescriptorSetLayoutBinding::read_texture_view(TextureViewType::tex2d, 0, 1, ShaderVisibilityFlag::pixel),
                DescriptorSetLayoutBinding::sampler(1, 1, ShaderVisibilityFlag::pixel)
                })));

            IDescriptorSetLayout* ds_layout = desc_set_layout;

            luset(pipeline_layout, device->new_pipeline_layout(PipelineLayoutDesc(
                { &ds_layout, 1 }, PipelineLayoutFlag::allow_input_assembler_input_layout)));

            GraphicsPipelineStateDesc desc;
            InputBindingDesc input_bindings[] = {
                InputBindingDesc(0, sizeof(VertexData), InputRate::per_vertex)
            };
            InputAttributeDesc input_attributes[] = {
                InputAttributeDesc("POSITION", 0, 0, 0, 0, Format::rg32_float),
                InputAttributeDesc("TEXCOORD", 0, 1, 0, 8, Format::rg32_float)
            };
            desc.input_layout = InputLayoutDesc({input_bindings, 1}, {input_attributes, 2});
            desc.vs = LUNA_GET_SHADER_DATA(TestTextureVS);
            desc.ps = LUNA_GET_SHADER_DATA(TestTexturePS);
            desc.pipeline_layout = pipeline_layout;
            desc.depth_stencil_state = DepthStencilDesc(false, false);
            desc.num_color_attachments = 1;
            desc.color_formats[0] = Format::bgra8_unorm;

            luset(pso, device->new_graphics_pipeline_state(desc));

            luset(vb, device->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::vertex_buffer, sizeof(VertexData) * 4)));
            u32 incides[] = { 0, 1, 2, 1, 3, 2 };
            luset(ib, device->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::index_buffer, sizeof(incides))));
            void* mapped_data = nullptr;
            luexp(ib->map(0, 0, &mapped_data));
            memcpy(mapped_data, incides, sizeof(incides));
            ib->unmap(0, sizeof(incides));

            lulet(image_file, open_file("uv_checker.png", FileOpenFlag::read, FileCreationMode::open_existing));
            lulet(image_file_data, load_file_data(image_file));
            Image::ImageDesc image_desc;
            lulet(image_data, Image::read_image_file(image_file_data.data(), image_file_data.size(), Image::ImageFormat::rgba8_unorm, image_desc));
            tex_width = image_desc.width;
            tex_height = image_desc.height;
            
            luset(tex, device->new_texture(MemoryType::local, TextureDesc::tex2d(Format::rgba8_unorm,
                TextureUsageFlag::read_texture | TextureUsageFlag::copy_dest, image_desc.width, image_desc.height, 1, 1)));

            u32 copy_queue_index = get_command_queue_index();
            lulet(upload_cmdbuf, device->new_command_buffer(copy_queue_index));
            luexp(copy_resource_data(upload_cmdbuf, {
                CopyResourceData::write_texture(tex, SubresourceIndex(0, 0), 0, 0, 0, 
                    image_data.data(), image_desc.width * 4, image_desc.width * image_desc.height * 4, 
                    image_desc.width, image_desc.height, 1)}));
            luset(desc_set, device->new_descriptor_set(DescriptorSetDesc(desc_set_layout)));

            luexp(desc_set->update_descriptors(
                {
                    WriteDescriptorSet::read_texture_view(0, TextureViewDesc::tex2d(tex)),
                    WriteDescriptorSet::sampler(1, SamplerDesc(Filter::linear, Filter::linear, Filter::linear, TextureAddressMode::clamp,
                            TextureAddressMode::clamp, TextureAddressMode::clamp))
                }));
        }
    }
    lucatchret;
    return ok;
}

void draw()
{
    // prepare draw buffer. POSITION : TEXCOORD
        // 0----1
        // |    |
        // 2----3

    auto sz = get_window()->get_framebuffer_size();
    auto w = sz.x;
    auto h = sz.y;

    f32 width = tex_width;
    f32 height = tex_height;

    VertexData data[4] = {
        { { -width / w,  height / h },{ 0.0f, 0.0f } },
        { {  width / w,  height / h },{ 1.0f, 0.0f } },
        { { -width / w, -height / h },{ 0.0f, 1.0f } },
        { {  width / w, -height / h },{ 1.0f, 1.0f } }
    };

    void* mapped = nullptr;
    lupanic_if_failed(vb->map(0, 0, &mapped));
    memcpy(mapped, data, sizeof(data));
    vb->unmap(0, sizeof(data));

    auto cb = get_command_buffer();
    cb->resource_barrier({},
        {
            {tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::shader_read_ps, ResourceBarrierFlag::none},
            {get_back_buffer(), SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::color_attachment_write, ResourceBarrierFlag::discard_content}
        });
    RenderPassDesc desc;
    desc.color_attachments[0] = ColorAttachment(get_back_buffer(), LoadOp::clear, StoreOp::store, Color::black());
    cb->begin_render_pass(desc);
    cb->set_graphics_pipeline_state(pso);
    cb->set_graphics_pipeline_layout(pipeline_layout);
    cb->set_graphics_descriptor_set(0, desc_set);
    cb->set_vertex_buffers(0, {VertexBufferView(vb, 0, sizeof(VertexData) * 4, sizeof(VertexData))});
    cb->set_index_buffer({ ib, 0, 24, Format::r32_uint });
    cb->set_scissor_rect(RectI(0, 0, (i32)w, (i32)h));
    cb->set_viewport(Viewport(0.0f, 0.0f, (f32)w, (f32)h, 0.0f, 1.0f));
    cb->draw_indexed(6, 0, 0);
    cb->end_render_pass();
}

void resize(u32 width, u32 height)
{
}

void cleanup()
{
    desc_set_layout.reset();
    pipeline_layout.reset();
    desc_set.reset();
    pso.reset();
    tex.reset();
    vb.reset();
    ib.reset();
}

namespace Luna
{
    Window::AppStatus app_init(opaque_t* app_state, int argc, char* argv[])
    {
        bool r = Luna::init();
        if(!r) return Window::AppStatus::failing;
        lutry
        {
            luexp(add_modules({module_rhi_test_bed()}));
            luexp(init_modules());
            register_init_func(start);
            register_close_func(cleanup);
            register_resize_func(resize);
            register_draw_func(draw);
            luexp(RHITestBed::init());
        }
        lucatch
        {
            log_error("RHITest", "%s", explain(luerr));
            return Window::AppStatus::failing;
        }
        return Window::AppStatus::running;
    }

    Window::AppStatus app_update(opaque_t app_state)
    {
        auto window = RHITestBed::get_window();
        if(window->is_closed()) return Window::AppStatus::exiting;
        if(window->is_minimized())
        {
            sleep(100);
            return Window::AppStatus::running;
        }
        lutry
        {
            luexp(RHITestBed::update());
        }
        lucatch
        {
            log_error("RHITest", "%s", explain(luerr));
            return Window::AppStatus::failing;
        }
        return Window::AppStatus::running;
    }

    void app_close(opaque_t app_state, Window::AppStatus status)
    {
        RHITestBed::close();
        Luna::close();
    }
}