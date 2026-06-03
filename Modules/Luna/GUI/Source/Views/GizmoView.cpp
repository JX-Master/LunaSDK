/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../GUI.hpp"

namespace Luna
{
    namespace GUI
    {
        static bool project_gizmo_point(const Float3& point, const Float4x4& view, const Float4x4& projection, const RectF& viewport_rect, Float2U& out)
        {
            Float4 clip = mul(Float4(point.x, point.y, point.z, 1.0f), mul(view, projection));
            if(abs(clip.w) < 0.00001f || isnan(clip.w) || isinf(clip.w)) return false;
            f32 ndc_x = clip.x / clip.w;
            f32 ndc_y = clip.y / clip.w;
            if(isnan(ndc_x) || isnan(ndc_y) || isinf(ndc_x) || isinf(ndc_y)) return false;
            out.x = viewport_rect.offset_x + (ndc_x * 0.5f + 0.5f) * viewport_rect.width;
            out.y = viewport_rect.offset_y + (0.5f - ndc_y * 0.5f) * viewport_rect.height;
            return true;
        }

        static RectF gizmo_line_hit_rect(const Float2U& a, const Float2U& b, f32 padding)
        {
            f32 min_x = min(a.x, b.x) - padding;
            f32 min_y = min(a.y, b.y) - padding;
            f32 max_x = max(a.x, b.x) + padding;
            f32 max_y = max(a.y, b.y) + padding;
            return RectF(min_x, min_y, max(max_x - min_x, 1.0f), max(max_y - min_y, 1.0f));
        }

        static Float3 gizmo_normalize_axis(const Float3& axis, const Float3& fallback)
        {
            f32 len = length(axis);
            if(len <= 0.0001f) return fallback;
            return axis / len;
        }

        static f32 gizmo_axis_pixels_per_unit(const Float2U& begin, const Float2U& end)
        {
            return length(Float2(end.x - begin.x, end.y - begin.y));
        }

        static f32 gizmo_compute_screen_factor(const Float3& origin, const Float4x4& view, const Float4x4& projection, const RectF& viewport_rect)
        {
            Float4x4 camera_world = inverse(view);
            Float3 camera_right = gizmo_normalize_axis(AffineMatrix::right(camera_world), Float3(1.0f, 0.0f, 0.0f));
            Float2U origin_screen;
            Float2U right_screen;
            if(!project_gizmo_point(origin, view, projection, viewport_rect, origin_screen) ||
                !project_gizmo_point(origin + camera_right, view, projection, viewport_rect, right_screen))
            {
                return 1.0f;
            }
            f32 pixels_per_world_unit = gizmo_axis_pixels_per_unit(origin_screen, right_screen);
            if(pixels_per_world_unit <= 0.0001f || isnan(pixels_per_world_unit) || isinf(pixels_per_world_unit))
            {
                return 1.0f;
            }
            f32 target_screen_length = clamp(min(viewport_rect.width, viewport_rect.height) * 0.11f, 56.0f, 96.0f);
            return max(target_screen_length / pixels_per_world_unit, 0.0001f);
        }

        static Float2U gizmo_state_default_pointer()
        {
            return Float2U(0.0f, 0.0f);
        }

        static Float3 gizmo_matrix_row3(const Float4x4& matrix, u32 row)
        {
            return Float3(matrix.r[row].x, matrix.r[row].y, matrix.r[row].z);
        }

        static void gizmo_set_matrix_row3(Float4x4& matrix, u32 row, const Float3& value)
        {
            matrix.r[row].x = value.x;
            matrix.r[row].y = value.y;
            matrix.r[row].z = value.z;
        }

        static bool gizmo_axis_delta_units(const Float2U& origin_screen, const Float2U& axis_screen, f32 axis_world_len, const Float2& screen_delta, f32& out_delta_units)
        {
            f32 axis_pixels = gizmo_axis_pixels_per_unit(origin_screen, axis_screen);
            if(axis_pixels <= 0.0001f) return false;
            Float2 screen_axis(axis_screen.x - origin_screen.x, axis_screen.y - origin_screen.y);
            screen_axis = normalize(screen_axis);
            out_delta_units = (screen_delta.x * screen_axis.x + screen_delta.y * screen_axis.y) * axis_world_len / axis_pixels;
            return true;
        }

        static void gizmo_scale_basis(Float4x4& matrix, i32 active_axis, f32 factor)
        {
            factor = max(factor, 0.001f);
            if(active_axis >= 0 && active_axis < 3)
            {
                gizmo_set_matrix_row3(matrix, (u32)active_axis, gizmo_matrix_row3(matrix, (u32)active_axis) * factor);
                return;
            }
            for(u32 i = 0; i < 3; ++i)
            {
                gizmo_set_matrix_row3(matrix, i, gizmo_matrix_row3(matrix, i) * factor);
            }
        }

        static void gizmo_rotate_basis(Float4x4& matrix, const Float3& axis, f32 angle)
        {
            Float4x4 rotation = AffineMatrix::make_rotation_axis_angle(axis, angle);
            for(u32 i = 0; i < 3; ++i)
            {
                Float3 row = gizmo_matrix_row3(matrix, i);
                Float4 rotated = mul(Float4(row.x, row.y, row.z, 0.0f), rotation);
                gizmo_set_matrix_row3(matrix, i, Float3(rotated.x, rotated.y, rotated.z));
            }
        }

        static Float4x4 gizmo_scale_delta_matrix(i32 active_axis, f32 factor)
        {
            if(active_axis == 0) return AffineMatrix::make_scaling(factor, 1.0f, 1.0f);
            if(active_axis == 1) return AffineMatrix::make_scaling(1.0f, factor, 1.0f);
            if(active_axis == 2) return AffineMatrix::make_scaling(1.0f, 1.0f, factor);
            return AffineMatrix::make_scaling(factor, factor, factor);
        }

        static void gizmo_draw_rotation_ring(IContext* context, const Float3& origin, const Float3& axis_a, const Float3& axis_b, f32 radius, const Float4x4& view, const Float4x4& projection,
            const RectF& viewport_rect, const Float4U& color, f32 width)
        {
            constexpr u32 segments = 48;
            Float2U previous_screen;
            bool previous_visible = false;
            for(u32 i = 0; i <= segments; ++i)
            {
                f32 angle = (f32)i / (f32)segments * 6.28318530717958647692f;
                Float3 point = origin + axis_a * cos(angle) * radius + axis_b * sin(angle) * radius;
                Float2U screen;
                bool visible = project_gizmo_point(point, view, projection, viewport_rect, screen);
                if(visible && previous_visible)
                {
                    draw_line(context, previous_screen, screen, color, width);
                }
                previous_screen = screen;
                previous_visible = visible;
            }
        }

        LUNA_GUI_API ItemHandle gizmo(IContext* context, const c8* label, Float4x4& world_matrix, const Float4x4& view, const Float4x4& projection, const RectF& viewport_rect,
            GizmoOperation operation, GizmoMode mode, f32 snap, bool enabled, bool orthographic,
            Float4x4* delta_matrix, bool* is_mouse_hover, bool* is_mouse_moving, bool* edited)
        {
            (void)orthographic;
            if(delta_matrix) *delta_matrix = Float4x4::identity();
            if(is_mouse_hover) *is_mouse_hover = false;
            if(is_mouse_moving) *is_mouse_moving = false;
            if(edited) *edited = false;

            push_id(context, label ? label : "Gizmo");

            Float3 origin = AffineMatrix::translation(world_matrix);
            Float3 axes[3];
            if(mode == GizmoMode::local)
            {
                axes[0] = gizmo_normalize_axis(AffineMatrix::right(world_matrix), Float3(1.0f, 0.0f, 0.0f));
                axes[1] = gizmo_normalize_axis(AffineMatrix::up(world_matrix), Float3(0.0f, 1.0f, 0.0f));
                axes[2] = gizmo_normalize_axis(AffineMatrix::forward(world_matrix), Float3(0.0f, 0.0f, 1.0f));
            }
            else
            {
                axes[0] = Float3(1.0f, 0.0f, 0.0f);
                axes[1] = Float3(0.0f, 1.0f, 0.0f);
                axes[2] = Float3(0.0f, 0.0f, 1.0f);
            }

            Float2U origin_screen;
            Float2U axis_screen[3];
            bool visible = project_gizmo_point(origin, view, projection, viewport_rect, origin_screen);
            f32 axis_world_len = gizmo_compute_screen_factor(origin, view, projection, viewport_rect);
            for(u32 i = 0; i < 3; ++i)
            {
                visible = project_gizmo_point(origin + axes[i] * axis_world_len, view, projection, viewport_rect, axis_screen[i]) && visible;
            }
            if(!visible)
            {
                ItemHandle invalid = hit_box(context, "Invalid", RectF(-10000.0f, -10000.0f, 1.0f, 1.0f));
                pop_id(context);
                return invalid;
            }

            Float4U axis_colors[3] = {
                Float4U(0.92f, 0.22f, 0.25f, 1.0f),
                Float4U(0.24f, 0.80f, 0.28f, 1.0f),
                Float4U(0.25f, 0.46f, 0.96f, 1.0f)
            };

            ItemHandle axis_handles[3];
            for(u32 i = 0; i < 3; ++i)
            {
                c8 axis_label[8];
                snprintf(axis_label, 8, "Axis%u", i);
                axis_handles[i] = hit_box(context, axis_label, gizmo_line_hit_rect(origin_screen, axis_screen[i], 7.0f));
            }
            ItemHandle center_handle = hit_box(context, "Center", RectF(origin_screen.x - 8.0f, origin_screen.y - 8.0f, 16.0f, 16.0f));

            i32 active_axis = -1;
            for(i32 i = 0; i < 3; ++i)
            {
                if(is_item_active(axis_handles[i]))
                {
                    active_axis = i;
                    break;
                }
            }
            if(active_axis < 0 && is_item_active(center_handle))
            {
                active_axis = 3;
            }

            bool hovered = is_item_hovered(center_handle);
            for(u32 i = 0; i < 3; ++i)
            {
                hovered = hovered || is_item_hovered(axis_handles[i]);
            }
            if(is_mouse_hover) *is_mouse_hover = hovered;
            if(is_mouse_moving) *is_mouse_moving = active_axis >= 0;

            if(operation == GizmoOperation::rotate)
            {
                gizmo_draw_rotation_ring(context, origin, axes[1], axes[2], axis_world_len, view, projection, viewport_rect,
                    is_item_hovered(axis_handles[0]) || is_item_active(axis_handles[0]) ? Float4U(1.0f, 0.95f, 0.65f, 1.0f) : axis_colors[0], 3.0f);
                gizmo_draw_rotation_ring(context, origin, axes[2], axes[0], axis_world_len, view, projection, viewport_rect,
                    is_item_hovered(axis_handles[1]) || is_item_active(axis_handles[1]) ? Float4U(1.0f, 0.95f, 0.65f, 1.0f) : axis_colors[1], 3.0f);
                gizmo_draw_rotation_ring(context, origin, axes[0], axes[1], axis_world_len, view, projection, viewport_rect,
                    is_item_hovered(axis_handles[2]) || is_item_active(axis_handles[2]) ? Float4U(1.0f, 0.95f, 0.65f, 1.0f) : axis_colors[2], 3.0f);
            }
            for(u32 i = 0; i < 3; ++i)
            {
                bool axis_hot = is_item_hovered(axis_handles[i]) || is_item_active(axis_handles[i]);
                Float4U color = axis_hot ? Float4U(1.0f, 0.95f, 0.65f, 1.0f) : axis_colors[i];
                f32 width = axis_hot ? 4.0f : 3.0f;
                draw_line(context, origin_screen, axis_screen[i], color, operation == GizmoOperation::rotate ? 2.0f : width);
                draw_circle(context, axis_screen[i], axis_hot ? 6.0f : 5.0f, color);
            }
            draw_circle(context, origin_screen, is_item_hovered(center_handle) || is_item_active(center_handle) ? 8.0f : 6.0f,
                is_item_hovered(center_handle) || is_item_active(center_handle) ? Float4U(1.0f, 0.95f, 0.65f, 1.0f) : Float4U(1.0f));

            StateKey<i32> active_axis_key { Name("gui.gizmo.active_axis"), -1 };
            StateKey<Float2U> last_pointer_key { Name("gui.gizmo.last_pointer"), gizmo_state_default_pointer() };
            i32 last_active_axis = get_item_state(center_handle, active_axis_key);
            Float2U pointer = get_pointer_position(context);
            Float2U last_pointer = get_item_state(center_handle, last_pointer_key);

            bool changed = false;
            if(enabled && active_axis >= 0)
            {
                if(last_active_axis == active_axis)
                {
                    Float2 screen_delta(pointer.x - last_pointer.x, pointer.y - last_pointer.y);
                    if(operation == GizmoOperation::translate)
                    {
                        Float3 world_delta(0.0f, 0.0f, 0.0f);
                        if(active_axis < 3)
                        {
                            f32 delta_units = 0.0f;
                            if(gizmo_axis_delta_units(origin_screen, axis_screen[active_axis], axis_world_len, screen_delta, delta_units))
                            {
                                if(snap > 0.0f && abs(delta_units) >= snap)
                                {
                                    delta_units = (delta_units > 0.0f ? 1.0f : -1.0f) * snap;
                                }
                                world_delta = axes[active_axis] * delta_units;
                            }
                        }
                        else
                        {
                            Float4x4 camera_world = inverse(view);
                            Float3 camera_right = gizmo_normalize_axis(AffineMatrix::right(camera_world), Float3(1.0f, 0.0f, 0.0f));
                            Float3 camera_up = gizmo_normalize_axis(AffineMatrix::up(camera_world), Float3(0.0f, 1.0f, 0.0f));
                            Float2U right_screen;
                            Float2U up_screen;
                            f32 right_ppu = project_gizmo_point(origin + camera_right, view, projection, viewport_rect, right_screen) ?
                                gizmo_axis_pixels_per_unit(origin_screen, right_screen) : 0.0f;
                            f32 up_ppu = project_gizmo_point(origin + camera_up, view, projection, viewport_rect, up_screen) ?
                                gizmo_axis_pixels_per_unit(origin_screen, up_screen) : 0.0f;
                            if(right_ppu > 0.0001f && up_ppu > 0.0001f)
                            {
                                world_delta = camera_right * (screen_delta.x / right_ppu) - camera_up * (screen_delta.y / up_ppu);
                            }
                        }
                        changed = length(world_delta) > 0.000001f;
                        if(changed)
                        {
                            world_matrix.r[3].x += world_delta.x;
                            world_matrix.r[3].y += world_delta.y;
                            world_matrix.r[3].z += world_delta.z;
                            if(delta_matrix) *delta_matrix = AffineMatrix::make_translation(world_delta);
                        }
                    }
                    else if(operation == GizmoOperation::scale)
                    {
                        f32 factor = 1.0f;
                        if(active_axis < 3)
                        {
                            f32 delta_units = 0.0f;
                            if(gizmo_axis_delta_units(origin_screen, axis_screen[active_axis], axis_world_len, screen_delta, delta_units))
                            {
                                factor += delta_units;
                            }
                        }
                        else
                        {
                            factor += (screen_delta.x - screen_delta.y) * 0.01f;
                        }
                        factor = max(factor, 0.001f);
                        changed = abs(factor - 1.0f) > 0.00001f;
                        if(changed)
                        {
                            gizmo_scale_basis(world_matrix, active_axis, factor);
                            if(delta_matrix) *delta_matrix = gizmo_scale_delta_matrix(active_axis, factor);
                        }
                    }
                    else if(operation == GizmoOperation::rotate)
                    {
                        Float3 axis = active_axis < 3 ? axes[active_axis] : gizmo_normalize_axis(AffineMatrix::forward(inverse(view)), Float3(0.0f, 0.0f, 1.0f));
                        f32 delta_angle = 0.0f;
                        if(active_axis < 3)
                        {
                            f32 pixels_per_unit = gizmo_axis_pixels_per_unit(origin_screen, axis_screen[active_axis]);
                            if(pixels_per_unit > 0.0001f)
                            {
                                Float2 screen_axis(axis_screen[active_axis].x - origin_screen.x, axis_screen[active_axis].y - origin_screen.y);
                                screen_axis = normalize(screen_axis);
                                Float2 tangent(-screen_axis.y, screen_axis.x);
                                delta_angle = (screen_delta.x * tangent.x + screen_delta.y * tangent.y) * 0.01f;
                            }
                        }
                        else
                        {
                            delta_angle = (screen_delta.x + screen_delta.y) * 0.01f;
                        }
                        if(snap > 0.0f && abs(delta_angle) >= snap)
                        {
                            delta_angle = (delta_angle > 0.0f ? 1.0f : -1.0f) * snap;
                        }
                        changed = abs(delta_angle) > 0.00001f;
                        if(changed)
                        {
                            gizmo_rotate_basis(world_matrix, axis, delta_angle);
                            if(delta_matrix) *delta_matrix = AffineMatrix::make_rotation_axis_angle(axis, delta_angle);
                        }
                    }
                }
            }

            if(changed)
            {
                if(edited) *edited = true;
            }
            set_item_state(center_handle, active_axis_key, active_axis);
            set_item_state(center_handle, last_pointer_key, pointer);

            pop_id(context);
            return center_handle;
        }
    }
}
