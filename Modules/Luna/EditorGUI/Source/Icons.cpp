/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Icons.cpp
* @author JXMaster
* @date 2026/7/22
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_EDITOR_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include "PhosphorCoreData.hpp"
#include <cstring>

namespace Luna
{
    namespace EditorGUI
    {
        namespace Internal
        {
            constexpr u32 ICON_DATA_VERSION = 1;
            constexpr u32 INVALID_ICON_LAYER = 0xFFFFFFFF;
            constexpr usize ICON_HEADER_SIZE = 64;
            constexpr usize ICON_LAYER_SIZE = 12;
            constexpr usize ICON_VARIANT_SIZE = 8;

            struct IconLayerData
            {
                u32 first_float = 0;
                u32 num_floats = 0;
                f32 opacity = 1.0f;
            };

            struct IconVariantData
            {
                u32 first_layer = INVALID_ICON_LAYER;
                u32 num_layers = 0;
            };

            struct IconData
            {
                IconName value = IconName::check;
                IconDesc desc;
            };

            static Ref<VG::IShapeBuffer> g_icon_shape_buffer;
            static Vector<IconLayerData> g_icon_layers;
            static Vector<IconVariantData> g_icon_variants;

            static u32 read_u32(const u8* data)
            {
                u32 value;
                memcpy(&value, data, sizeof(value));
                return value;
            }

            static f32 read_f32(const u8* data)
            {
                f32 value;
                memcpy(&value, data, sizeof(value));
                return value;
            }

            RV initialize_icons()
            {
                if(g_icon_shape_buffer) return ok;
                if(PHOSPHOR_CORE_SIZE < ICON_HEADER_SIZE ||
                    memcmp(PHOSPHOR_CORE_DATA, "LGUIICON", 8) != 0)
                {
                    return set_error(E_BAD_DATA, "The built-in GUI icon data header is invalid.");
                }
                const u8* data = PHOSPHOR_CORE_DATA;
                u32 version = read_u32(data + 8);
                u32 num_floats = read_u32(data + 12);
                u32 num_layers = read_u32(data + 16);
                u32 num_variants = read_u32(data + 20);
                if(version != ICON_DATA_VERSION || num_variants !=
                    (u32)IconName::count * (u32)IconWeight::count)
                {
                    return set_error(E_BAD_DATA, "The built-in GUI icon data version or table size is invalid.");
                }
                usize layer_offset = ICON_HEADER_SIZE;
                usize variant_offset = layer_offset + (usize)num_layers * ICON_LAYER_SIZE;
                usize float_offset = variant_offset + (usize)num_variants * ICON_VARIANT_SIZE;
                usize required_size = float_offset + (usize)num_floats * sizeof(f32);
                if(required_size != PHOSPHOR_CORE_SIZE)
                {
                    return set_error(E_BAD_DATA, "The built-in GUI icon data size is invalid.");
                }

                g_icon_layers.resize(num_layers);
                for(u32 i = 0; i < num_layers; ++i)
                {
                    const u8* source = data + layer_offset + (usize)i * ICON_LAYER_SIZE;
                    IconLayerData& layer = g_icon_layers[i];
                    layer.first_float = read_u32(source);
                    layer.num_floats = read_u32(source + 4);
                    layer.opacity = read_f32(source + 8);
                    if(layer.first_float > num_floats || layer.num_floats > num_floats - layer.first_float)
                    {
                        return set_error(E_BAD_DATA, "A built-in GUI icon layer is out of bounds.");
                    }
                }

                g_icon_variants.resize(num_variants);
                for(u32 i = 0; i < num_variants; ++i)
                {
                    const u8* source = data + variant_offset + (usize)i * ICON_VARIANT_SIZE;
                    IconVariantData& variant = g_icon_variants[i];
                    variant.first_layer = read_u32(source);
                    variant.num_layers = read_u32(source + 4);
                    if(variant.first_layer != INVALID_ICON_LAYER &&
                        (variant.first_layer > num_layers || variant.num_layers > num_layers - variant.first_layer))
                    {
                        return set_error(E_BAD_DATA, "A built-in GUI icon variant is out of bounds.");
                    }
                }

                g_icon_shape_buffer = VG::new_shape_buffer();
                Vector<f32>& points = g_icon_shape_buffer->get_shape_points(true);
                points.resize(num_floats);
                memcpy(points.data(), data + float_offset, (usize)num_floats * sizeof(f32));
                return ok;
            }

            void close_icons()
            {
                g_icon_variants.clear();
                g_icon_layers.clear();
                g_icon_shape_buffer = nullptr;
            }

            static const IconVariantData* find_variant(IconName value, IconWeight weight, bool fallback)
            {
                u32 icon_index = (u32)value;
                u32 weight_index = (u32)weight;
                if(icon_index >= (u32)IconName::count || weight_index >= (u32)IconWeight::count ||
                    g_icon_variants.empty()) return nullptr;
                const IconVariantData* variant = &g_icon_variants[
                    icon_index * (u32)IconWeight::count + weight_index];
                if(variant->first_layer == INVALID_ICON_LAYER && fallback)
                {
                    variant = &g_icon_variants[icon_index * (u32)IconWeight::count + (u32)IconWeight::regular];
                }
                return variant->first_layer == INVALID_ICON_LAYER ? nullptr : variant;
            }

            static f32 resolve_icon_size(GUI::IContext* context,
                const GUI::ElementHandle& element, const IconDesc& desc)
            {
                if(desc.size > 0.0f) return desc.size;
                return style_scalar(context, element, "gui.icon.size", 16.0f);
            }

            static GUI::MeasureResult measure_icon(GUI::IContext* context,
                const GUI::ElementHandle& element, const Float2U&, void* userdata)
            {
                GUI::MeasureResult result;
                IconData* data = (IconData*)userdata;
                if(data)
                {
                    f32 size = resolve_icon_size(context, element, data->desc);
                    result.desired = Float2U(size, size);
                }
                return result;
            }

            static RV draw_icon(GUI::IContext* context, const GUI::ElementHandle& element,
                GUI::DrawPhase, void* userdata)
            {
                IconData* data = (IconData*)userdata;
                if(!data || !g_icon_shape_buffer) return ok;
                const IconVariantData* variant = find_variant(data->value, data->desc.weight, true);
                if(!variant) return ok;

                Float4U tint = data->desc.tint;
                if(tint.w < 0.0f)
                {
                    tint = style_color(context, element, "gui.icon.color",
                        style_color(context, element, "gui.text.color", Float4U(1.0f)));
                }
                RectF rect = get_item_rect(context, element);
                f32 side = min(rect.width, rect.height);
                if(side <= 0.0f || tint.w <= 0.0f) return ok;
                RectF local_rect((rect.width - side) * 0.5f, (rect.height - side) * 0.5f, side, side);
                for(u32 i = 0; i < variant->num_layers; ++i)
                {
                    const IconLayerData& layer = g_icon_layers[variant->first_layer + i];
                    GUI::DrawCommand command;
                    command.type = GUI::DrawCommandType::shape;
                    command.rect_reference = GUI::DrawCommandRectReference::element;
                    command.rect = local_rect;
                    command.color = tint;
                    command.color.w *= layer.opacity < 0.999f ?
                        max(data->desc.secondary_opacity, 0.0f) * layer.opacity : layer.opacity;
                    command.shape.buffer = g_icon_shape_buffer;
                    command.shape.first_command = layer.first_float;
                    command.shape.num_commands = layer.num_floats;
                    command.shape.bounds = RectF(0.0f, 0.0f, 256.0f, 256.0f);
                    context->draw(command);
                }
                return ok;
            }
        }

        LUNA_EDITOR_GUI_API GUI::ElementHandle icon(GUI::IContext* context, id_t id, IconName value,
            const GUI::LayoutConfig& layout, const IconDesc& desc)
        {
            luassert(context && id);
            GUI::ElementHandle element = Internal::begin_element(context, id, "Icon", layout);
            Internal::IconData* data = Internal::allocate_frame<Internal::IconData>(context);
            data->value = value;
            data->desc = desc;
            GUI::LayoutCallbackConfig callbacks;
            callbacks.algorithm = Name("gui.icon");
            callbacks.measure_callback = Internal::measure_icon;
            callbacks.userdata = data;
            context->set_layout_callback_config(element, callbacks);
            GUI::DrawConfig draw;
            draw.name = Name("gui.icon");
            draw.callback = Internal::draw_icon;
            draw.userdata = data;
            context->set_draw_config(element, draw);
            context->end_element();
            return element;
        }

        LUNA_EDITOR_GUI_API bool has_icon(IconName value, IconWeight weight)
        {
            return Internal::find_variant(value, weight, false) != nullptr;
        }
    }
}
