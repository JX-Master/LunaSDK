#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/UniquePtr.hpp>
#include <Luna/Window/Window.hpp>
#include <Luna/Window/Event.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/RHIUtility/RHIUtility.hpp>
#include <Luna/ShaderCompiler/ShaderCompiler.hpp>
#include <Luna/RHI/ShaderCompileHelper.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/Runtime/Math/Matrix.hpp>
#include <Luna/RHIUtility/ResourceWriteContext.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/Image/Image.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/Window/AppMain.hpp>

#include <BoxVert.hpp>
#include <BoxPixel.hpp>

using namespace Luna;
struct DemoApp
{
    Ref<Window::IWindow> window;
    Ref<RHI::IDevice> dev;
    u32 queue;
    Ref<RHI::ICommandBuffer> cmdbuf;
    Ref<RHI::ISwapChain> swap_chain;
    Ref<RHI::IDescriptorSetLayout> dlayout;
    Ref<RHI::IDescriptorSet> desc_set;
    Ref<RHI::IPipelineLayout> playout;
    Ref<RHI::IPipelineState> pso;
    Ref<RHI::ITexture> depth_tex;
    Ref<RHI::IBuffer> vb;
    Ref<RHI::IBuffer> ib;
    Ref<RHI::IBuffer> ub;
    Ref<RHI::ITexture> file_tex;
    f32 camera_rotation = 0.0f;

    RV init();
    RV update();
    bool is_exiting();
    RV resize(u32 width, u32 height);
};
struct Vertex
{
    Float3U position;
    Float2U texcoord;
};
RV DemoApp::init()
{
    lutry
    {
        // On desktop platforms, we need to create our own window, but in 
        // mobile platforms, the window is already created for us, and we
        // just use that window.
#if defined(LUNA_PLATFORM_DESKTOP)
        luset(window, Window::new_window("DemoApp"));
#elif defined(LUNA_PLATFORM_MOBILE)
        window = Window::get_system_window();
#endif
        Window::set_event_handler([](object_t event, void* userdata){
            DemoApp* app = (DemoApp*)userdata;
            if(Window::WindowFramebufferResizeEvent* e = cast_object<Window::WindowFramebufferResizeEvent>(event))
            {
                lupanic_if_failed(app->resize(e->width, e->height));
            }
        }, this);
        dev = RHI::get_main_device();
        using namespace RHI;
        queue = U32_MAX;
        u32 num_queues = dev->get_num_command_queues();
        for (u32 i = 0; i < num_queues; ++i)
        {
            auto desc = dev->get_command_queue_desc(i);
            if (desc.type == CommandQueueType::graphics && test_flags(desc.flags, CommandQueueFlag::presenting))
            {
                queue = i;
                break;
            }
        }
        if(queue == U32_MAX) return BasicError::not_supported();
        luset(cmdbuf, dev->new_command_buffer(queue));
        luset(swap_chain, dev->new_swap_chain(queue, window, SwapChainDesc(0, 0, 2, Format::bgra8_unorm, true)));
        luset(dlayout, dev->new_descriptor_set_layout (DescriptorSetLayoutDesc ({
            DescriptorSetLayoutBinding:: uniform_buffer_view (0, 1, ShaderVisibilityFlag::vertex),
            DescriptorSetLayoutBinding:: read_texture_view (TextureViewType:: tex2d, 1, 1, ShaderVisibilityFlag::pixel),
            DescriptorSetLayoutBinding:: sampler (2, 1, ShaderVisibilityFlag::pixel)
        })));
        luset(desc_set, dev->new_descriptor_set (DescriptorSetDesc (dlayout)));
        luset(playout, dev->new_pipeline_layout(PipelineLayoutDesc({dlayout}, 
            PipelineLayoutFlag::allow_input_assembler_input_layout)));
        GraphicsPipelineStateDesc ps_desc;
        ps_desc.primitive_topology = PrimitiveTopology::triangle_list;
        ps_desc.rasterizer_state = RasterizerDesc ();
        ps_desc.depth_stencil_state = DepthStencilDesc (true, true, CompareFunction::less_equal);
        ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
        InputAttributeDesc input_attributes[] = {
            InputAttributeDesc("POSITION", 0, 0, 0, 0, Format::rgb32_float),
            InputAttributeDesc("TEXCOORD", 0, 1, 0, 12, Format::rg32_float)
        };
        InputBindingDesc input_bindings[] = {
            InputBindingDesc(0, 20, InputRate::per_vertex)
        };
        ps_desc.input_layout.attributes = {input_attributes, 2};
        ps_desc.input_layout.bindings = {input_bindings, 1};
        ps_desc.vs = LUNA_GET_SHADER_DATA(BoxVert);
        ps_desc.ps = LUNA_GET_SHADER_DATA(BoxPixel);
        ps_desc.pipeline_layout = playout;
        ps_desc.num_color_attachments = 1;
        ps_desc.color_formats[0] = Format::rgba8_unorm;
        ps_desc.depth_stencil_format = Format::d32_float;
        luset(pso, dev->new_graphics_pipeline_state(ps_desc));
        auto window_size = window->get_framebuffer_size();
        luset(depth_tex, dev->new_texture(MemoryType::local, TextureDesc::tex2d(Format::d32_float, TextureUsageFlag::depth_stencil_attachment, window_size.x, window_size.y, 1, 1)));
        Vertex vertices[] = {
            {{+0.5, -0.5, -0.5}, {0.0, 1.0}} , {{+0.5, +0.5, -0.5}, {0.0, 0.0}} ,
            {{+0.5, +0.5, +0.5}, {1.0, 0.0}} , {{+0.5, -0.5, +0.5}, {1.0, 1.0}} ,
            {{+0.5, -0.5, +0.5}, {0.0, 1.0}} , {{+0.5, +0.5, +0.5}, {0.0, 0.0}} ,
            {{-0.5, +0.5, +0.5}, {1.0, 0.0}} , {{-0.5, -0.5, +0.5}, {1.0, 1.0}} ,
            {{-0.5, -0.5, +0.5}, {0.0, 1.0}} , {{-0.5, +0.5, +0.5}, {0.0, 0.0}} ,
            {{-0.5, +0.5, -0.5}, {1.0, 0.0}} , {{-0.5, -0.5, -0.5}, {1.0, 1.0}} ,
            {{-0.5, -0.5, -0.5}, {0.0, 1.0}} , {{-0.5, +0.5, -0.5}, {0.0, 0.0}} ,
            {{+0.5, +0.5, -0.5}, {1.0, 0.0}} , {{+0.5, -0.5, -0.5}, {1.0, 1.0}} ,
            {{-0.5, +0.5, -0.5}, {0.0, 1.0}} , {{-0.5, +0.5, +0.5}, {0.0, 0.0}} ,
            {{+0.5, +0.5, +0.5}, {1.0, 0.0}} , {{+0.5, +0.5, -0.5}, {1.0, 1.0}} ,
            {{+0.5, -0.5, -0.5}, {0.0, 1.0}} , {{+0.5, -0.5, +0.5}, {0.0, 0.0}} ,
            {{-0.5, -0.5, +0.5}, {1.0, 0.0}} , {{-0.5, -0.5, -0.5}, {1.0, 1.0}}
        };
        u32 indices[] = {
            0, 1, 2, 0, 2, 3, 
            4, 5, 6, 4, 6, 7, 
            8, 9, 10, 8, 10, 11,
            12, 13, 14, 12, 14, 15,
            16, 17, 18, 16, 18, 19,
            20, 21, 22, 20, 22, 23
        };
        luset(vb, dev->new_buffer(MemoryType::local, BufferDesc(BufferUsageFlag::vertex_buffer | BufferUsageFlag::copy_dest, sizeof(vertices))));
        luset(ib, dev->new_buffer(MemoryType::local, BufferDesc(BufferUsageFlag::index_buffer | BufferUsageFlag::copy_dest, sizeof(indices))));
        auto ub_align = dev->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment;
        luset(ub, dev->new_buffer(MemoryType::upload, BufferDesc(BufferUsageFlag::uniform_buffer, align_upper(sizeof(Float4x4), ub_align))));
        
        Ref<RHIUtility::IResourceWriteContext> writer = RHIUtility::new_resource_write_context(dev);
        lulet(vb_data, writer->write_buffer(vb, 0, sizeof(vertices)));
        lulet(ib_data, writer->write_buffer(ib, 0, sizeof(indices)));
        memcpy(vb_data, vertices, sizeof(vertices));
        memcpy(ib_data, indices, sizeof(indices));
        luexp(writer->commit(cmdbuf, true));
        lulet(image_file, open_file("Luna.png", FileOpenFlag::read, FileCreationMode::open_existing));
        lulet(image_file_data, load_file_data(image_file));
        Image::ImageDesc image_desc;
        lulet(image_data, Image::read_image_file(image_file_data.data(), image_file_data.size(), Image::ImageFormat::rgba8_unorm, image_desc));
        luset(file_tex, dev->new_texture(MemoryType::local, TextureDesc::tex2d(Format::rgba8_unorm, 
            TextureUsageFlag::copy_dest | TextureUsageFlag::read_texture, image_desc.width, image_desc.height, 1, 1)));
        writer->reset();
        u32 row_pitch, slice_pitch;
        lulet(image_buffer, writer->write_texture(file_tex, SubresourceIndex(0, 0),
            0, 0, 0, image_desc.width, image_desc.height, 1, row_pitch, slice_pitch));
        memcpy_bitmap(image_buffer, image_data.data(), image_desc.width * 4, image_desc.height, row_pitch, image_desc.width * 4);
        luexp(writer->commit(cmdbuf, true));
        luexp (desc_set->update_descriptors ({
            WriteDescriptorSet::uniform_buffer_view(0, BufferViewDesc::uniform_buffer(ub)),
            WriteDescriptorSet::read_texture_view(1, TextureViewDesc::tex2d(file_tex)),
            WriteDescriptorSet::sampler(2, SamplerDesc(Filter::linear, Filter::linear, Filter::linear, TextureAddressMode::clamp, TextureAddressMode::clamp, TextureAddressMode::clamp))
        }));

    }
    lucatchret;
    return ok;
}
RV DemoApp::update()
{
    Window::poll_events();
    if(window->is_closed()) return ok;
    if(window->is_minimized()) return ok;
    lutry
    {
        camera_rotation += 1.0f;
        Float3 camera_pos(cosf(camera_rotation / 180.0f * PI) * 3.0f, 1.0f, sinf(camera_rotation / 180.0f * PI) * 3.0f);
        Float4x4 camera_mat = AffineMatrix::make_look_at(camera_pos, Float3(0, 0, 0), Float3(0, 1, 0));
        auto window_sz = window->get_framebuffer_size();
        camera_mat = mul(camera_mat, ProjectionMatrix::make_perspective_fov(PI / 3.0f, (f32)window_sz.x / (f32) window_sz.y, 0.001f, 100.0f));
        void* camera_mapped;
        luexp(ub->map(0, 0, &camera_mapped));
        memcpy(camera_mapped, &camera_mat, sizeof(Float4x4));
        ub->unmap(0, sizeof(Float4x4));
        using namespace RHI;
        lulet(back_buffer, swap_chain->get_current_back_buffer());
        cmdbuf->resource_barrier({
            BufferBarrier(ub, BufferStateFlag::automatic, BufferStateFlag::uniform_buffer_vs),
            BufferBarrier(vb, BufferStateFlag::automatic, BufferStateFlag::vertex_buffer),
            BufferBarrier(ib, BufferStateFlag::automatic, BufferStateFlag::index_buffer)
        }, {
            TextureBarrier(file_tex, TEXTURE_BARRIER_ALL_SUBRESOURCES, TextureStateFlag::automatic, TextureStateFlag::shader_read_ps),
            TextureBarrier(back_buffer, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::color_attachment_write),
            TextureBarrier(depth_tex, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::depth_stencil_attachment_write)
        });
        RenderPassDesc desc;
        desc.color_attachments[0] = ColorAttachment(back_buffer, LoadOp::clear, StoreOp::store, Float4U(0.0f));
        desc.depth_stencil_attachment = DepthStencilAttachment(depth_tex, false, LoadOp::clear, StoreOp::store, 1.0f);
        cmdbuf->begin_render_pass(desc);
        cmdbuf->set_graphics_pipeline_layout(playout);
        cmdbuf->set_graphics_pipeline_state(pso);
        cmdbuf->set_graphics_descriptor_set(0, desc_set);
        auto sz = vb->get_desc().size;
        cmdbuf->set_vertex_buffers(0, {VertexBufferView(vb, 0, sz, sizeof(Vertex))});
        sz = ib->get_desc().size;
        cmdbuf->set_index_buffer(IndexBufferView(ib, 0, sz, Format::r32_uint));
        cmdbuf->set_scissor_rect(RectI(0, 0, (i32)window_sz.x, (i32)window_sz.y));
        cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)window_sz.x, (f32)window_sz.y, 0.0f, 1.0f));
        cmdbuf->draw_indexed(36, 0, 0);
        cmdbuf->end_render_pass();
        cmdbuf->resource_barrier({}, {
            TextureBarrier(back_buffer, SubresourceIndex(0, 0), TextureStateFlag::automatic, TextureStateFlag::present)
        });
        luexp(cmdbuf->submit({}, {}, true));
        cmdbuf->wait();
        luexp(cmdbuf->reset());
        luexp(swap_chain->present());
    }
    lucatchret;
    return ok;
}
bool DemoApp::is_exiting()
{
    return window->is_closed();
}
RV DemoApp::resize(u32 width, u32 height)
{
    lutry
    {
        using namespace RHI;
        if(width && height)
        {
            luexp(swap_chain->reset({width, height, 2, Format::unknown, true}));
            luset(depth_tex, dev->new_texture(MemoryType:: local, TextureDesc::tex2d(Format::d32_float, TextureUsageFlag::depth_stencil_attachment, width, height, 1, 1)));
        }
    }
    lucatchret;
    return ok;
}

RV run_app()
{
    log_info("DemoApp", "App started.");
    auto result = add_modules({
        module_window(),
        module_rhi(),
        module_rhi_utility(),
        module_shader_compiler()
    });
    if(failed(result)) return result;
    result = init_modules();
    if(failed(result)) return result;
    UniquePtr<DemoApp> app (memnew<DemoApp>());
    result = app->init();
    if(failed(result)) return result;
    while(!app->is_exiting())
    {
        result = app->update();
        if(failed(result)) return result;
    }
    return ok;
}
int luna_main(int argc, const char* argv[])
{
    if(!Luna::init()) return -1;
    RV result = run_app();
    if(failed(result)) log_error("DemoApp", "%s", explain(result.errcode()));
    Luna::close();
    return 0;
}