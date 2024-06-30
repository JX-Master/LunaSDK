/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.hpp
* @author JXMaster
* @date 2022/4/17
*/
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Window/Window.hpp>
#include <Luna/RHI/SwapChain.hpp>
#include <Luna/VG/ShapeRenderer.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/VG/Shapes.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Runtime/Time.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/VG/TextArranger.hpp>
#include <Luna/Font/Font.hpp>
#include <Luna/HID/HID.hpp>
#include <Luna/HID/Mouse.hpp>
#include <Luna/HID/Keyboard.hpp>
#include <Luna/VG/VG.hpp>

namespace Luna
{
    Ref<Window::IWindow> g_window;

    Ref<RHI::ISwapChain> g_swap_chain;
    Ref<RHI::ICommandBuffer> g_command_buffer;
    u32 g_command_queue;

    Ref<VG::IShapeDrawList> g_shape_draw_list;
    Ref<VG::IShapeRenderer> g_shape_renderer;

    Ref<VG::IFontAtlas> g_font_atlas;

    Float3 g_camera_position;
    Float4 g_camera_rotation = Quaternion::identity();
    f32 g_camera_speed = 10.0f;
    bool g_camera_navigating = false;
    Float2 g_scene_click_pos;
}

using namespace Luna;

RV recreate_window_resources(u32 width, u32 height)
{
    using namespace RHI;
    lutry
    {
        if (width && height)
        {
            if (!g_swap_chain)
            {
                g_swap_chain = get_main_device()->new_swap_chain(g_command_queue, g_window, SwapChainDesc({ width, height, 2, Format::bgra8_unorm, true})).get();
            }
            else
            {
                luexp(g_swap_chain->reset({width, height, 2, Format::bgra8_unorm, true}));
            }
        }
    }
    lucatchret;
    return ok;
}

void on_window_resize(Window::IWindow* window, u32 width, u32 height)
{
    lupanic_if_failed(recreate_window_resources(width, height));
    lupanic_if_failed(g_shape_renderer->set_render_target(g_swap_chain->get_current_back_buffer().get()));
}

void on_window_close(Window::IWindow* window)
{
    window->close();
}

void on_mouse_down(Window::IWindow* window, HID::MouseButton button)
{
    if (button == HID::MouseButton::right)
    {
        g_camera_navigating = true;
        g_scene_click_pos = HID::get_mouse_pos();
    }
}

void on_mouse_up(Window::IWindow* window, HID::MouseButton button)
{
    if (button == HID::MouseButton::right)
    {
        g_camera_navigating = false;
    }
}

void init()
{
    lupanic_if_failed(add_modules({module_window(), module_rhi(), module_font(), module_vg(), module_hid()}));
    lupanic_if_failed(init_modules());
    // register event.
    g_window = Window::new_window("Luna Vector Graphics Test", Window::WindowDisplaySettings::as_windowed(), Window::WindowCreationFlag::resizable).get();
    g_window->get_mouse_down_event().add_handler(on_mouse_down);
    g_window->get_mouse_up_event().add_handler(on_mouse_up);
    auto sz = g_window->get_size();

    g_camera_position = Float3(sz.x / 2.0f, sz.y / 2.0f, -3000.0f);

    g_window->get_close_event().add_handler(on_window_close);
    g_window->get_framebuffer_resize_event().add_handler(on_window_resize);

    g_shape_draw_list = VG::new_shape_draw_list();

    auto dev = RHI::get_main_device();

    g_command_queue = U32_MAX;
    u32 num_queues = dev->get_num_command_queues();
    for (u32 i = 0; i < num_queues; ++i)
    {
        auto desc = dev->get_command_queue_desc(i);
        if (desc.type == RHI::CommandQueueType::graphics)
        {
            g_command_queue = i;
            break;
        }
    }

    lupanic_if_failed(recreate_window_resources(sz.x, sz.y));
    g_shape_renderer = VG::new_fill_shape_renderer().get();
    g_command_buffer = dev->new_command_buffer(g_command_queue).get();

    auto font = Font::get_default_font();
    g_font_atlas = VG::new_font_atlas();
}

void run()
{
    while (true)
    {
        //new_frame();
        Window::poll_events();

        if (g_window->is_closed()) break;
        if (g_window->is_minimized())
        {
            sleep(100);
            continue;
        }

        if (g_camera_navigating)
        {
            auto mouse_pos = HID::get_mouse_pos();
            auto mouse_delta = mouse_pos - g_scene_click_pos;
            g_scene_click_pos = mouse_pos;
            // Rotate camera based on mouse delta.
            auto rot = g_camera_rotation;
            auto rot_mat = AffineMatrix::make_rotation(rot);

            // Key control.
            auto left = AffineMatrix::left(rot_mat);
            auto forward = AffineMatrix::forward(rot_mat);
            auto up = AffineMatrix::up(rot_mat);

            f32 camera_speed = g_camera_speed;
            if (HID::get_key_state(HID::KeyCode::l_shift))
            {
                camera_speed *= 2.0f;
            }

            if (HID::get_key_state(HID::KeyCode::w))
            {
                g_camera_position += forward * camera_speed;
            }
            if (HID::get_key_state(HID::KeyCode::a))
            {
                g_camera_position += left * camera_speed;
            }
            if (HID::get_key_state(HID::KeyCode::s))
            {
                g_camera_position += -forward * camera_speed;
            }
            if (HID::get_key_state(HID::KeyCode::d))
            {
                g_camera_position += -left * camera_speed;
            }
            if (HID::get_key_state(HID::KeyCode::q))
            {
                g_camera_position += -up * camera_speed;
            }
            if (HID::get_key_state(HID::KeyCode::e))
            {
                g_camera_position += up * camera_speed;
            }
            auto eular = AffineMatrix::euler_angles(rot_mat);
            eular += {deg_to_rad((f32)mouse_delta.y / 10.0f), deg_to_rad((f32)mouse_delta.x / 10.0f), 0.0f};
            eular.x = clamp(eular.x, deg_to_rad(-85.0f), deg_to_rad(85.0f));
            g_camera_rotation = Quaternion::from_euler_angles(eular);
        }

        auto window_sz = g_window->get_size();
        {
            const c8* text = "Vector Graphics";
            VG::TextArrangeSection section;
            section.font_size = 128;
            section.font_file = Font::get_default_font();
            section.font_index = 0;
            RectF bounding_rect = RectF(0, 0, window_sz.x, window_sz.y - 100.0f);
            auto arrange_result = VG::arrange_text(text, 15, {&section, 1}, bounding_rect, VG::TextAlignment::begin, VG::TextAlignment::center);
            lupanic_if_failed(VG::commit_text_arrange_result(arrange_result, { &section, 1 }, g_font_atlas, g_shape_draw_list));
        }

        constexpr f32 shape_scale = 2.0f;

        g_shape_draw_list->set_shape_buffer(nullptr);
        auto& points = g_shape_draw_list->get_shape_points();
        u32 offset = (u32)points.size();
        VG::ShapeBuilder::add_rectangle_filled(points, 0, 0, 100, 100);
        u32 end_offset = (u32)points.size();
        Float2 draw_pos = { window_sz.x / 2.0f - 350.0f * shape_scale, window_sz.y - 500.0f * shape_scale };
        g_shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_pink());
        
        offset = end_offset;
        VG::ShapeBuilder::add_rectangle_bordered(points, 0, 0, 100, 100, 5, -2.5f);
        end_offset = (u32)points.size();
        draw_pos.y += 150.0f * shape_scale;
        g_shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_pink());

        offset = end_offset;
        VG::ShapeBuilder::add_rounded_rectangle_filled(points, 0, 0, 100, 100, 10);
        end_offset = (u32)points.size();
        draw_pos.x += 150.0f * shape_scale;
        draw_pos.y -= 150.0f * shape_scale;
        g_shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_yellow());

        offset = end_offset;
        VG::ShapeBuilder::add_rounded_rectangle_bordered(points, 0, 0, 100, 100, 10, 5, -2.5f);
        end_offset = (u32)points.size();
        draw_pos.y += 150.0f * shape_scale;
        g_shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_yellow());

        offset = end_offset;
        VG::ShapeBuilder::add_triangle_filled(points, 0, 0, 50, 100, 100, 0);
        end_offset = (u32)points.size();
        draw_pos.x += 150.0f * shape_scale;
        draw_pos.y -= 150.0f * shape_scale;
        g_shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_green());

        offset = end_offset;
        VG::ShapeBuilder::add_triangle_bordered(points, 0, 0, 50, 100, 100, 0, 5, -2.5f);
        end_offset = (u32)points.size();
        draw_pos.y += 150.0f * shape_scale;
        g_shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_green());

        offset = end_offset;
        VG::ShapeBuilder::add_circle_filled(points, 50, 50, 50);
        end_offset = (u32)points.size();
        draw_pos.x += 150.0f * shape_scale;
        draw_pos.y -= 150.0f * shape_scale;
        g_shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_blue());

        offset = end_offset;
        VG::ShapeBuilder::add_circle_bordered(points, 50, 50, 50, 5, -2.5f);
        end_offset = (u32)points.size();
        draw_pos.y += 150.0f * shape_scale;
        g_shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_blue());

        offset = end_offset;
        VG::ShapeBuilder::add_axis_aligned_ellipse_filled(points, 50, 50, 50, 25);
        end_offset = (u32)points.size();
        draw_pos.x += 150.0f * shape_scale;
        draw_pos.y -= 150.0f * shape_scale;
        g_shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_steel_blue());

        offset = end_offset;
        VG::ShapeBuilder::add_axis_aligned_ellipse_bordered(points, 50, 50, 50, 25, 5, -2.5f);
        end_offset = (u32)points.size();
        draw_pos.y += 150.0f * shape_scale;
        g_shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_steel_blue());
        
        lupanic_if_failed(g_shape_draw_list->compile());

        RHI::RenderPassDesc desc;
        auto texture = g_swap_chain->get_current_back_buffer().get();
        desc.color_attachments[0] = RHI::ColorAttachment(texture, RHI::LoadOp::clear, RHI::StoreOp::store, Float4U{ 0.0f });
        g_command_buffer->begin_render_pass(desc);
        g_command_buffer->end_render_pass();

        lupanic_if_failed(g_shape_renderer->set_render_target(g_swap_chain->get_current_back_buffer().get()));

        Float4x4 proj_matrix = ProjectionMatrix::make_perspective_fov(PI / 3.0f, (f32)window_sz.x / (f32)window_sz.y, 0.3f, 10000.0f);
        Float4x4 view_matrix = inverse(AffineMatrix::make(g_camera_position, g_camera_rotation, Float3(1.0f)));
        Float4x4U mat = Float4x4U(mul(view_matrix, proj_matrix));

        lupanic_if_failed(g_shape_renderer->render(g_command_buffer, g_shape_draw_list->get_vertex_buffer(), g_shape_draw_list->get_index_buffer(),  g_shape_draw_list->get_draw_calls(), &mat));

        g_command_buffer->resource_barrier({},
            {
                {g_swap_chain->get_current_back_buffer().get(), RHI::SubresourceIndex(0, 0), RHI::TextureStateFlag::automatic, RHI::TextureStateFlag::present, RHI::ResourceBarrierFlag::none}
            });

        lupanic_if_failed(g_command_buffer->submit({}, {}, true));
        g_command_buffer->wait();

        lupanic_if_failed(g_swap_chain->present());
        lupanic_if_failed(g_command_buffer->reset());
        g_shape_draw_list->reset();
    }
}
void shutdown()
{
    g_window = nullptr;

    g_swap_chain = nullptr;
    g_command_buffer = nullptr;
    g_shape_draw_list = nullptr;
    g_shape_renderer = nullptr;
    g_font_atlas = nullptr;
}
int main()
{
    Luna::init();
    ::init();
    run();
    ::shutdown();
    Luna::close();
}
