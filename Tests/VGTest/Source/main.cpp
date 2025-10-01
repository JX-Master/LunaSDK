/*!
* This file is a portion of LunaSDK.
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
#include <Luna/Runtime/Log.hpp>

#include <Luna/Window/AppMain.hpp>

namespace Luna
{
    struct App
    {
        Ref<Window::IWindow> window;

        Ref<RHI::ISwapChain> swap_chain;
        Ref<RHI::ICommandBuffer> command_buffer;
        u32 command_queue;
    
        Ref<VG::IShapeDrawList> shape_draw_list;
        Ref<VG::IShapeRenderer> shape_renderer;
    
        Ref<VG::IFontAtlas> font_atlas;
    
        Float3 camera_position;
        Float4 camera_rotation = Quaternion::identity();
        f32 camera_speed = 10.0f;
        bool camera_navigating = false;
        Float2 scene_click_pos;
    };

    RV recreate_window_resources(App* app, u32 width, u32 height)
    {
        using namespace RHI;
        lutry
        {
            if (width && height)
            {
                if (!app->swap_chain)
                {
                    luset(app->swap_chain, get_main_device()->new_swap_chain(app->command_queue, app->window, SwapChainDesc({ width, height, 2, Format::bgra8_unorm, true})));
                }
                else
                {
                    luexp(app->swap_chain->reset({width, height, 2, Format::bgra8_unorm, true}));
                }
            }
        }
        lucatchret;
        return ok;
    }

    void on_window_resize(App* app, Window::IWindow* window, u32 width, u32 height)
    {
        lupanic_if_failed(recreate_window_resources(app, width, height));
    }

    void on_window_close(Window::IWindow* window)
    {
        window->close();
    }

    void on_mouse_down(App* app, Window::IWindow* window, HID::MouseButton button)
    {
        if (button == HID::MouseButton::right)
        {
            app->camera_navigating = true;
            app->scene_click_pos = HID::get_mouse_pos();
        }
    }

    void on_mouse_up(App* app, Window::IWindow* window, HID::MouseButton button)
    {
        if (button == HID::MouseButton::right)
        {
            app->camera_navigating = false;
        }
    }

    Window::AppStatus app_init(opaque_t* app_state, int argc, char* argv[])
    {
        bool r = Luna::init();
        if(!r) return Window::AppStatus::failing;
        lutry
        {
            luexp(add_modules({module_window(), module_rhi(), module_font(), module_vg(), module_hid()}));
            luexp(init_modules());
            App* app = memnew<App>();
            *app_state = app;
            // register event.
            luset(app->window, Window::new_window("Luna Vector Graphics Test", Window::WindowDisplaySettings::as_windowed(), Window::WindowCreationFlag::resizable));
            app->window->get_events().mouse_down.add_handler([app](Window::IWindow* window, HID::MouseButton button) { on_mouse_down(app, window, button); });
            app->window->get_events().mouse_up.add_handler([app](Window::IWindow* window, HID::MouseButton button) { on_mouse_up(app, window, button); });
            auto sz = app->window->get_size();

            app->camera_position = Float3(sz.x / 2.0f, sz.y / 2.0f, -3000.0f);

            app->window->get_events().close.add_handler(on_window_close);
            app->window->get_events().framebuffer_resize.add_handler([app](Window::IWindow* window, u32 width, u32 height) { on_window_resize(app, window, width, height); });

            app->shape_draw_list = VG::new_shape_draw_list();

            auto dev = RHI::get_main_device();

            app->command_queue = U32_MAX;
            u32 num_queues = dev->get_num_command_queues();
            for (u32 i = 0; i < num_queues; ++i)
            {
                auto desc = dev->get_command_queue_desc(i);
                if (desc.type == RHI::CommandQueueType::graphics)
                {
                    app->command_queue = i;
                    break;
                }
            }

            luexp(recreate_window_resources(app, sz.x, sz.y));
            app->shape_renderer = VG::new_fill_shape_renderer();
            luset(app->command_buffer, dev->new_command_buffer(app->command_queue));

            auto font = Font::get_default_font();
            app->font_atlas = VG::new_font_atlas();
        }
        lucatch
        {
            log_error("VGTest", "%s", explain(luerr));
            return Window::AppStatus::failing;
        }
        return Window::AppStatus::running;
    }
    Window::AppStatus app_update(opaque_t app_state)
    {
        lutry
        {
            App* app = (App*)app_state;
            if (app->window->is_closed()) return Window::AppStatus::exiting;
            if (app->window->is_minimized())
            {
                sleep(100);
                return Window::AppStatus::running;
            }

            if (app->camera_navigating)
            {
                auto mouse_pos = HID::get_mouse_pos();
                auto mouse_delta = mouse_pos - app->scene_click_pos;
                app->scene_click_pos = mouse_pos;
                // Rotate camera based on mouse delta.
                auto rot = app->camera_rotation;
                auto rot_mat = AffineMatrix::make_rotation(rot);

                // Key control.
                auto left = AffineMatrix::left(rot_mat);
                auto forward = AffineMatrix::forward(rot_mat);
                auto up = AffineMatrix::up(rot_mat);

                f32 camera_speed = app->camera_speed;
                if (HID::get_key_state(HID::KeyCode::l_shift))
                {
                    camera_speed *= 2.0f;
                }

                if (HID::get_key_state(HID::KeyCode::w))
                {
                    app->camera_position += forward * camera_speed;
                }
                if (HID::get_key_state(HID::KeyCode::a))
                {
                    app->camera_position += left * camera_speed;
                }
                if (HID::get_key_state(HID::KeyCode::s))
                {
                    app->camera_position += -forward * camera_speed;
                }
                if (HID::get_key_state(HID::KeyCode::d))
                {
                    app->camera_position += -left * camera_speed;
                }
                if (HID::get_key_state(HID::KeyCode::q))
                {
                    app->camera_position += -up * camera_speed;
                }
                if (HID::get_key_state(HID::KeyCode::e))
                {
                    app->camera_position += up * camera_speed;
                }
                auto eular = AffineMatrix::euler_angles(rot_mat);
                eular += {deg_to_rad((f32)mouse_delta.y / 10.0f), deg_to_rad((f32)mouse_delta.x / 10.0f), 0.0f};
                eular.x = clamp(eular.x, deg_to_rad(-85.0f), deg_to_rad(85.0f));
                app->camera_rotation = Quaternion::from_euler_angles(eular);
            }

            auto window_sz = app->window->get_size();
            {
                const c8* text = "Vector Graphics";
                VG::TextArrangeSection section;
                section.font_size = 128;
                section.font_file = Font::get_default_font();
                section.font_index = 0;
                RectF bounding_rect = RectF(0, 0, window_sz.x, window_sz.y - 100.0f);
                auto arrange_result = VG::arrange_text(text, 15, {&section, 1}, bounding_rect, VG::TextAlignment::begin, VG::TextAlignment::center);
                VG::commit_text_arrange_result(arrange_result, { &section, 1 }, app->font_atlas, app->shape_draw_list);
            }

            constexpr f32 shape_scale = 2.0f;

            app->shape_draw_list->set_shape_buffer(nullptr);
            auto& points = app->shape_draw_list->get_shape_buffer()->get_shape_points(true);
            u32 offset = (u32)points.size();
            VG::ShapeBuilder::add_rectangle_filled(points, 0, 0, 100, 100);
            u32 end_offset = (u32)points.size();
            Float2 draw_pos = { window_sz.x / 2.0f - 350.0f * shape_scale, window_sz.y - 500.0f * shape_scale };
            app->shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_pink());
            
            offset = end_offset;
            VG::ShapeBuilder::add_rectangle_bordered(points, 0, 0, 100, 100, 5, -2.5f);
            end_offset = (u32)points.size();
            draw_pos.y += 150.0f * shape_scale;
            app->shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_pink());

            offset = end_offset;
            VG::ShapeBuilder::add_rounded_rectangle_filled(points, 0, 0, 100, 100, 10);
            end_offset = (u32)points.size();
            draw_pos.x += 150.0f * shape_scale;
            draw_pos.y -= 150.0f * shape_scale;
            app->shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_yellow());

            offset = end_offset;
            VG::ShapeBuilder::add_rounded_rectangle_bordered(points, 0, 0, 100, 100, 10, 5, -2.5f);
            end_offset = (u32)points.size();
            draw_pos.y += 150.0f * shape_scale;
            app->shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_yellow());

            offset = end_offset;
            VG::ShapeBuilder::add_triangle_filled(points, 0, 0, 50, 100, 100, 0);
            end_offset = (u32)points.size();
            draw_pos.x += 150.0f * shape_scale;
            draw_pos.y -= 150.0f * shape_scale;
            app->shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_green());

            offset = end_offset;
            VG::ShapeBuilder::add_triangle_bordered(points, 0, 0, 50, 100, 100, 0, 5, -2.5f);
            end_offset = (u32)points.size();
            draw_pos.y += 150.0f * shape_scale;
            app->shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_green());

            offset = end_offset;
            VG::ShapeBuilder::add_circle_filled(points, 50, 50, 50);
            end_offset = (u32)points.size();
            draw_pos.x += 150.0f * shape_scale;
            draw_pos.y -= 150.0f * shape_scale;
            app->shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_blue());

            offset = end_offset;
            VG::ShapeBuilder::add_circle_bordered(points, 50, 50, 50, 5, -2.5f);
            end_offset = (u32)points.size();
            draw_pos.y += 150.0f * shape_scale;
            app->shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_blue());

            offset = end_offset;
            VG::ShapeBuilder::add_axis_aligned_ellipse_filled(points, 50, 50, 50, 25);
            end_offset = (u32)points.size();
            draw_pos.x += 150.0f * shape_scale;
            draw_pos.y -= 150.0f * shape_scale;
            app->shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_steel_blue());

            offset = end_offset;
            VG::ShapeBuilder::add_axis_aligned_ellipse_bordered(points, 50, 50, 50, 25, 5, -2.5f);
            end_offset = (u32)points.size();
            draw_pos.y += 150.0f * shape_scale;
            app->shape_draw_list->draw_shape(offset, end_offset - offset, draw_pos, draw_pos + 100.0f * shape_scale, { 0.0f, 0.0f }, { 100.0f, 100.0f }, Color::light_steel_blue());
            
            luexp(app->shape_draw_list->compile());

            RHI::RenderPassDesc desc;
            lulet(texture, app->swap_chain->get_current_back_buffer());
            desc.color_attachments[0] = RHI::ColorAttachment(texture, RHI::LoadOp::clear, RHI::StoreOp::store, Float4U{ 0.0f });
            app->command_buffer->begin_render_pass(desc);
            app->command_buffer->end_render_pass();

            luexp(app->shape_renderer->begin(app->swap_chain->get_current_back_buffer().get()));

            Float4x4 proj_matrix = ProjectionMatrix::make_perspective_fov(PI / 3.0f, (f32)window_sz.x / (f32)window_sz.y, 0.3f, 10000.0f);
            Float4x4 view_matrix = inverse(AffineMatrix::make(app->camera_position, app->camera_rotation, Float3(1.0f)));
            Float4x4U mat = Float4x4U(mul(view_matrix, proj_matrix));

            app->shape_renderer->draw(app->shape_draw_list->get_vertex_buffer(), app->shape_draw_list->get_index_buffer(),  app->shape_draw_list->get_draw_calls(), &mat);

            luexp(app->shape_renderer->end());
            app->shape_renderer->submit(app->command_buffer);

            app->command_buffer->resource_barrier({},
                {
                    {app->swap_chain->get_current_back_buffer().get(), RHI::SubresourceIndex(0, 0), RHI::TextureStateFlag::automatic, RHI::TextureStateFlag::present, RHI::ResourceBarrierFlag::none}
                });

            luexp(app->command_buffer->submit({}, {}, true));
            app->command_buffer->wait();

            luexp(app->swap_chain->present());
            luexp(app->command_buffer->reset());
            app->shape_draw_list->reset();
        }
        lucatch
        {
            log_error("VGTest", "%s", explain(luerr));
            return Window::AppStatus::failing;
        }
        return Window::AppStatus::running;
    }
    void app_close(opaque_t app_state, Window::AppStatus status)
    {
        App* app = (App*)app_state;
        memdelete(app);
        Luna::close();
    }
}