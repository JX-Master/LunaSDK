/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorPrimitives.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "EditorInternal.hpp"
#include <Luna/GUI/EditorWidgets.hpp>
#include <Luna/VG/TextArranger.hpp>

namespace Luna
{
    namespace GUI
    {
        static GUICore::StyleValue style_value(GUICore::IContext* context, const Name& entry, const GUICore::StyleValue& default_value)
        {
            if(!context)
            {
                return default_value;
            }
            return context->get_style_value(context->current_style(), entry, default_value);
        }

        static Ref<CoreLayoutUserdataArenaState> layout_userdata_arena_state(GUICore::IContext* context)
        {
            id_t state_id = GUICore::make_state_id<CoreLayoutUserdataArenaState>(0);
            Ref<CoreLayoutUserdataArenaState> state;
            if(object_t state_obj = context->get_state(state_id))
            {
                object_retain(state_obj);
                state.attach(state_obj);
            }
            else
            {
                state = new_object<CoreLayoutUserdataArenaState>();
            }
            if(state->generation != context->generation())
            {
                state->generation = context->generation();
                state->block_index = 0;
                state->offset = 0;
            }
            lupanic_if_failed(context->set_state(state_id, state.object(), GUICore::StateLifetime::next_frame));
            return state;
        }

        static void* allocate_layout_userdata_raw(GUICore::IContext* context, usize size, usize alignment)
        {
            luassert(context);
            Ref<CoreLayoutUserdataArenaState> state = layout_userdata_arena_state(context);
            size = max(size, (usize)1);
            alignment = max(alignment, (usize)1);
            constexpr usize block_alignment = 16;
            constexpr usize default_block_size = 4096;
            usize required_block_size = max(default_block_size, align_upper(size, block_alignment));
            for(;;)
            {
                if(state->block_index >= state->blocks.size())
                {
                    state->blocks.push_back(Blob(required_block_size, block_alignment));
                }
                Blob& block = state->blocks[state->block_index];
                if(block.size() < size)
                {
                    if(state->offset == 0)
                    {
                        block.resize(required_block_size, false);
                    }
                    else
                    {
                        ++state->block_index;
                        state->offset = 0;
                        continue;
                    }
                }
                usize offset = align_upper(state->offset, alignment);
                if(offset + size <= block.size())
                {
                    state->offset = offset + size;
                    return (void*)((byte_t*)block.data() + offset);
                }
                ++state->block_index;
                state->offset = 0;
            }
        }

        struct TextMeasureDesc
        {
            const c8* text = nullptr;
            usize text_size = 0;
            f32 font_size = 16.0f;
        };

        static TextMeasureDesc* allocate_text_measure_desc(GUICore::IContext* context, const c8* text, f32 font_size)
        {
            usize text_size = text ? strlen(text) : 0;
            usize total_size = sizeof(TextMeasureDesc) + text_size + 1;
            byte_t* memory = (byte_t*)allocate_layout_userdata_raw(context, total_size, alignof(TextMeasureDesc));
            TextMeasureDesc* desc = (TextMeasureDesc*)memory;
            c8* copied_text = (c8*)(memory + sizeof(TextMeasureDesc));
            if(text_size)
            {
                memcpy(copied_text, text, text_size);
            }
            copied_text[text_size] = 0;
            desc->text = copied_text;
            desc->text_size = text_size;
            desc->font_size = font_size;
            return desc;
        }

        static GUICore::FontDesc resolve_measure_font(GUICore::IContext* context, const Name& id)
        {
            GUICore::FontDesc font = context->get_font(id);
            if(!font.font)
            {
                font.font = Font::get_default_font();
                font.font_index = 0;
            }
            return font;
        }

        static GUICore::MeasureResult measure_text_content(GUICore::IContext* context,
            const GUICore::ElementHandle&, const Float2U& available_content_size, void* userdata)
        {
            GUICore::MeasureResult result;
            TextMeasureDesc* desc = (TextMeasureDesc*)userdata;
            if(!context || !desc || !desc->text_size || desc->font_size <= 0.0f)
            {
                return result;
            }
            GUICore::FontDesc font = resolve_measure_font(context, Name());
            if(!font.font)
            {
                return result;
            }
            f32 text_width = max(available_content_size.x, 1.0f);
            f32 text_height = max(available_content_size.y, desc->font_size * 2.0f);
            VG::TextArrangeSection section;
            section.font_file = font.font;
            section.font_index = font.font_index;
            section.font_size = desc->font_size;
            section.color = Float4U(1.0f);
            section.num_chars = desc->text_size;
            VG::TextArrangeResult arranged = VG::arrange_text(desc->text, desc->text_size,
                Span<const VG::TextArrangeSection>(&section, 1), RectF(0.0f, 0.0f, text_width, text_height),
                VG::TextAlignment::center, VG::TextAlignment::begin);
            result.desired = Float2U(arranged.bounding_rect.width, arranged.bounding_rect.height);
            result.maximum = Float2U(F32_MAX, F32_MAX);
            return result;
        }

        static GUICore::DrawCommand element_rect_command(GUICore::DrawCommandType type, const Float4U& color,
            f32 radius = 0.0f)
        {
            GUICore::DrawCommand command;
            command.type = type;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            command.color = color;
            command.radius = radius;
            return command;
        }

        LUNA_GUI_API GUICore::ElementHandle text(GUICore::IContext* context, GUICore::id_t id, const c8* text,
            const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            GUICore::ElementHandle element = Internal::begin_element(context, id, text ? text : "text");
            GUICore::DrawCommand command;
            command.type = GUICore::DrawCommandType::text;
            command.rect_reference = GUICore::DrawCommandRectReference::element;
            command.rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            command.color = style_value(context, Name("gui.editor.text.color"),
                GUICore::style_f32x4(Float4U(0.86f, 0.88f, 0.92f, 1.0f))).number;
            command.font_size = style_value(context, Name("gui.editor.text.font_size"), GUICore::style_f32(16.0f)).number.x;
            context->set_layout_config(element, layout);
            GUICore::LayoutCallbackConfig layout_callbacks;
            layout_callbacks.algorithm = Name("gui.editor.text");
            layout_callbacks.measure_callback = measure_text_content;
            layout_callbacks.userdata = allocate_text_measure_desc(context, text, command.font_size);
            context->set_layout_callback_config(element, layout_callbacks);
            command.horizontal_alignment = VG::TextAlignment::begin;
            command.vertical_alignment = VG::TextAlignment::center;
            command.text = text ? text : "";
            context->draw(command);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle image(GUICore::IContext* context, GUICore::id_t id, RHI::ITexture* texture,
            const GUICore::LayoutConfig& layout, ImageFlag flags)
        {
            luassert(context && id);
            GUICore::ElementHandle element = Internal::begin_element(context, id, "image");
            context->set_layout_config(element, layout);
            GUICore::DrawCommand command = element_rect_command(GUICore::DrawCommandType::image, Float4U(1.0f));
            command.texture = texture;
            command.min_texcoord = Float2U(0.0f, test_flags(flags, ImageFlag::flip_y) ? 1.0f : 0.0f);
            command.max_texcoord = Float2U(1.0f, test_flags(flags, ImageFlag::flip_y) ? 0.0f : 1.0f);
            command.nearest_sampler = test_flags(flags, ImageFlag::nearest);
            context->draw(command);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle shape(GUICore::IContext* context, GUICore::id_t id, const GUICore::ShapeDesc& shape,
            const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            GUICore::ElementHandle element = Internal::begin_element(context, id, "shape");
            context->set_layout_config(element, layout);
            GUICore::DrawCommand command = element_rect_command(GUICore::DrawCommandType::shape, Float4U(1.0f));
            command.shape = shape;
            context->draw(command);
            context->end_element();
            return element;
        }

        LUNA_GUI_API GUICore::ElementHandle hit_box(GUICore::IContext* context, GUICore::id_t id,
            const GUICore::LayoutConfig& layout)
        {
            luassert(context && id);
            GUICore::ElementHandle element = Internal::begin_element(context, id, "hit_box");
            context->set_layout_config(element, layout);
            GUICore::Interactable interactable;
            interactable.pointer_hit_behavior = GUICore::PointerHitBehavior::target;
            set_flags(interactable.flags, GUICore::InteractableFlag::hoverable);
            set_flags(interactable.flags, GUICore::InteractableFlag::activatable);
            set_flags(interactable.flags, GUICore::InteractableFlag::focusable);
            context->set_interactable(element, interactable);
            context->end_element();
            return element;
        }
    }
}
