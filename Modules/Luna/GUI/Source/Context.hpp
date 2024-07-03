/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Context.hpp
* @author JXMaster
* @date 2024/6/28
*/
#pragma once
#include "../Context.hpp"
#include "NkHeader.h"

namespace Luna
{
    namespace GUI
    {
        inline enum nk_keys encode_nk_key(HID::KeyCode key)
        {
            using namespace HID;
            switch(key)
            {
                case KeyCode::shift: 
                case KeyCode::l_shift:
                case KeyCode::r_shift: return NK_KEY_SHIFT;
                case KeyCode::ctrl:
                case KeyCode::l_ctrl:
                case KeyCode::r_ctrl: return NK_KEY_CTRL;
                case KeyCode::del: return NK_KEY_DEL;
                case KeyCode::enter: return NK_KEY_ENTER;
                case KeyCode::tab: return NK_KEY_TAB;
                case KeyCode::backspace: return NK_KEY_BACKSPACE;
                case KeyCode::up: return NK_KEY_UP;
                case KeyCode::down: return NK_KEY_DOWN;
                case KeyCode::left: return NK_KEY_LEFT;
                case KeyCode::right: return NK_KEY_RIGHT;
                default: break;
            }
            return NK_KEY_NONE;
        }

        struct Context : IContext
        {
            lustruct("GUI::Context", "31512b61-63b3-4225-9757-69f6041d44ac");
            luiimpl();

            struct nk_context m_ctx;
            struct nk_user_font m_font;
            struct nk_allocator m_allocator;

            // Viewport size.
            UInt2U m_viewport_size;

            // Font info.
            Ref<VG::IFontAtlas> m_font_atlas;

            void init();
            ~Context();

            virtual UInt2U get_viewport_size() override
            {
                return m_viewport_size;
            }
            virtual void set_viewport_size(const UInt2U& size) override
            {
                m_viewport_size = size;
            }
            virtual f32 get_font_size() override
            {
                return m_font.height;
            }
            virtual void set_font_size(f32 font_size) override
            {
                m_font.height = font_size;
            }
            virtual VG::IFontAtlas* get_font_atlas() override
            {
                return m_font_atlas;
            }
            virtual void set_font_atlas(VG::IFontAtlas* font_atlas) override
            {
                m_font_atlas = font_atlas;
            }
            virtual void begin_frame() override
            {
                nk_clear(&m_ctx);
            }
            virtual void begin_input() override
            {
                nk_input_begin(&m_ctx);
            }
            virtual void input_mouse_move(i32 x, i32 y) override
            {
                nk_input_motion(&m_ctx, x, y);
            }
            virtual void input_key(HID::KeyCode key, bool pressed) override
            {
                enum nk_keys nkk = encode_nk_key(key);
                if (nkk != NK_KEY_NONE)
                {
                    nk_input_key(&m_ctx, nkk, pressed);
                }
            }
            virtual void input_mouse_button(HID::MouseButton button, i32 x, i32 y, bool pressed) override
            {
                using namespace HID;
                enum nk_buttons nkb;
                switch(button)
                {
                    case MouseButton::left: nkb = NK_BUTTON_LEFT; break;
                    case MouseButton::right: nkb = NK_BUTTON_RIGHT; break;
                    case MouseButton::middle: nkb = NK_BUTTON_MIDDLE; break;
                    default: return;
                }
                nk_input_button(&m_ctx, nkb, x, y, pressed);
            }
            virtual void input_mouse_wheel(f32 scroll_x, f32 scroll_y) override
            {
                struct nk_vec2 scroll;
                scroll.x = scroll_x;
                scroll.y = scroll_y;
                nk_input_scroll(&m_ctx, scroll);
            }
            virtual void input_character(c32 ch) override
            {
                nk_input_unicode(&m_ctx, (nk_rune)ch);
            }
            virtual void end_input() override
            {
                nk_input_end(&m_ctx);
            }
            virtual void render(VG::IShapeDrawList* draw_list) override;
        };
    }
}