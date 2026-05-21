/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUI.hpp
* @author JXMaster
* @date 2026/5/21
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Any.hpp>
#include <Luna/Runtime/Span.hpp>
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Runtime/String.hpp>
#include <Luna/Runtime/Name.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/Runtime/Math/Math.hpp>
#include <Luna/RHI/RHI.hpp>

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        using GUIID = u64;

        enum class GUIInputEventType : u8
        {
            pointer_enter,
            pointer_leave,
            pointer_move,
            pointer_down,
            pointer_up,
            pointer_wheel,
            key_down,
            key_up,
            text_utf8,
            focus,
            blur
        };

        enum class GUIPointerButton : u8
        {
            left,
            right,
            middle,
            extra1,
            extra2
        };

        enum class GUIKey : u16
        {
            unknown,
            tab,
            left,
            right,
            up,
            down,
            enter,
            esc,
            backspace,
            del,
            space,
            a,
            b,
            c,
            d,
            e,
            f,
            g,
            h,
            i,
            j,
            k,
            l,
            m,
            n,
            o,
            p,
            q,
            r,
            s,
            t,
            u,
            v,
            w,
            x,
            y,
            z
        };

        enum class GUIKeyModifierFlag : u8
        {
            none = 0x00,
            ctrl = 0x01,
            shift = 0x02,
            alt = 0x04,
            system = 0x08
        };

        enum class GUINodeKind : u8
        {
            root,
            column,
            row,
            scroll_view,
            window,
            text,
            button,
            checkbox,
            input_text,
            image,
            collapsing_header
        };

        struct GUIFrameDesc
        {
            Float2U surface_size = Float2U(0.0f);
            UInt2U framebuffer_size = UInt2U(0, 0);
            f32 dpi_scale = 1.0f;
            f32 delta_time = 1.0f / 60.0f;
        };

        struct GUIInputEvent
        {
            GUIInputEventType type = GUIInputEventType::pointer_move;
            u64 device_id = 0;
            u64 pointer_id = 0;
            Float2U position = Float2U(0.0f);
            Float2U wheel_delta = Float2U(0.0f);
            GUIPointerButton button = GUIPointerButton::left;
            GUIKey key = GUIKey::unknown;
            GUIKeyModifierFlag modifiers = GUIKeyModifierFlag::none;
            String text;
        };

        struct GUIItemHandle
        {
            object_t context = nullptr;
            GUIID id = 0;
            u64 generation = 0;
        };

        template <typename _Ty>
        struct GUIStateKey
        {
            Name name;
            _Ty default_value;
        };

        struct GUISize
        {
            f32 width = 0.0f;
            f32 height = 0.0f;

            static GUISize fixed(f32 width, f32 height)
            {
                GUISize r;
                r.width = width;
                r.height = height;
                return r;
            }
        };

        struct GUINode
        {
            GUIID id = 0;
            GUINodeKind kind = GUINodeKind::root;
            u32 parent = U32_MAX;
            u32 first_child = U32_MAX;
            u32 last_child = U32_MAX;
            u32 next_sibling = U32_MAX;
            u32 depth = 0;
            String text;
            Ref<RHI::ITexture> texture;
            GUISize requested_size;
            bool* bool_value = nullptr;
            String* string_value = nullptr;
            bool interactive = false;
        };

        struct GUIDescription
        {
            u64 generation = 0;
            Vector<GUINode> nodes;
        };

        namespace GUIState
        {
            LUNA_GUI_API GUIStateKey<bool> clicked();
            LUNA_GUI_API GUIStateKey<bool> double_clicked();
            LUNA_GUI_API GUIStateKey<bool> hovered();
            LUNA_GUI_API GUIStateKey<bool> active();
            LUNA_GUI_API GUIStateKey<bool> focused();
            LUNA_GUI_API GUIStateKey<bool> open();
            LUNA_GUI_API GUIStateKey<bool> value_changed();
            LUNA_GUI_API GUIStateKey<RectF> rect();
            LUNA_GUI_API GUIStateKey<RectF> clip_rect();
        }

        struct IGUIContext : virtual Interface
        {
            luiid("{E58F6F6C-48A9-42AB-86F3-898419C207BC}");

            virtual void begin_frame(const GUIFrameDesc& desc) = 0;
            virtual void add_input_event(const GUIInputEvent& event) = 0;
            virtual void add_input_events(Span<const GUIInputEvent> events) = 0;
            virtual R<GUIDescription> end_build() = 0;
            virtual RV submit(const GUIDescription& desc) = 0;
            virtual RV render(RHI::ICommandBuffer* cmdbuf, RHI::ITexture* render_target) = 0;
        };

        LUNA_GUI_API Ref<IGUIContext> new_context(RHI::IDevice* device = nullptr);

        LUNA_GUI_API void set_current_context(IGUIContext* context);
        LUNA_GUI_API IGUIContext* get_current_context();

        LUNA_GUI_API void PushID(u64 id);
        LUNA_GUI_API void PushID(const void* ptr);
        LUNA_GUI_API void PushID(const c8* str);
        LUNA_GUI_API void PopID();

        LUNA_GUI_API GUIItemHandle BeginColumn(const c8* label = nullptr);
        LUNA_GUI_API void EndColumn();
        LUNA_GUI_API GUIItemHandle BeginRow(const c8* label = nullptr);
        LUNA_GUI_API void EndRow();
        LUNA_GUI_API GUIItemHandle BeginScrollView(const c8* label, const GUISize& size);
        LUNA_GUI_API void EndScrollView();
        LUNA_GUI_API GUIItemHandle BeginWindow(const c8* label, const GUISize& size = GUISize());
        LUNA_GUI_API void EndWindow();

        LUNA_GUI_API GUIItemHandle Button(const c8* label);
        LUNA_GUI_API GUIItemHandle Text(const c8* text);
        LUNA_GUI_API GUIItemHandle Checkbox(const c8* label, bool* value);
        LUNA_GUI_API GUIItemHandle InputText(const c8* label, String& value);
        LUNA_GUI_API GUIItemHandle Image(RHI::ITexture* texture, const GUISize& size);
        LUNA_GUI_API GUIItemHandle CollapsingHeader(const c8* label);

        LUNA_GUI_API const Any* get_item_state_any(GUIItemHandle handle, const Name& key);
        LUNA_GUI_API void set_item_state_any(GUIItemHandle handle, const Name& key, const Any& value);
        LUNA_GUI_API void remove_item_state(GUIItemHandle handle, const Name& key);

        template <typename _Ty>
        _Ty GetItemState(GUIItemHandle handle, const GUIStateKey<_Ty>& key)
        {
            const Any* value = get_item_state_any(handle, key.name);
            if(!value) return key.default_value;
            const _Ty* typed_value = value->as<_Ty>();
            return typed_value ? *typed_value : key.default_value;
        }

        template <typename _Ty>
        void SetItemState(GUIItemHandle handle, const GUIStateKey<_Ty>& key, const _Ty& value)
        {
            set_item_state_any(handle, key.name, Any(value));
        }

        LUNA_GUI_API bool IsItemClicked(GUIItemHandle handle);
        LUNA_GUI_API bool IsItemDoubleClicked(GUIItemHandle handle);
        LUNA_GUI_API bool IsItemHovered(GUIItemHandle handle);
        LUNA_GUI_API bool IsItemActive(GUIItemHandle handle);
        LUNA_GUI_API bool IsItemFocused(GUIItemHandle handle);

        LUNA_GUI_API Module* module_gui();
    }
}
