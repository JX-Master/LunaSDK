/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Events.hpp
* @author JXMaster
* @date 2022/9/30
*/
#pragma once
#include "../Window.hpp"
namespace Luna
{
	namespace Window
	{
		struct WindowEvents
		{
			Event<window_close_event_handler_t> close;
			Event<window_focus_event_handler_t> focus;
			Event<window_lose_focus_event_handler_t> lose_focus;
			Event<window_show_event_handler_t> show;
			Event<window_hide_event_handler_t> hide;
			Event<window_resize_event_handler_t> resize;
			Event<window_framebuffer_resize_event_handler_t> framebuffer_resize;
			Event<window_move_event_handler_t> move;
			Event<window_begin_resize_move_t> begin_resize_move;
			Event<window_end_resize_move_t> end_resize_move;
			Event<window_dpi_changed_event_handler_t> dpi_changed;
			Event<window_key_down_event_handler_t> key_down;
			Event<window_key_up_event_handler_t> key_up;
			Event<window_input_character_event_handler_t> input_character;
			Event<window_mouse_move_event_handler_t> mouse_move;
			Event<window_mouse_down_event_handler_t> mouse_down;
			Event<window_mouse_up_event_handler_t> mouse_up;
			Event<window_mouse_wheel_event_handler_t> mouse_wheel;
			Event<window_touch_event_handler_t> touch;
			Event<window_drop_file_event_handler_t> drop_file;

			void reset()
			{
				close.clear();
				focus.clear();
				lose_focus.clear();
				show.clear();
				hide.clear();
				resize.clear();
				framebuffer_resize.clear();
				move.clear();
				begin_resize_move.clear();
				end_resize_move.clear();
				dpi_changed.clear();
				key_down.clear();
				key_up.clear();
				input_character.clear();
				mouse_move.clear();
				mouse_down.clear();
				mouse_up.clear();
				mouse_wheel.clear();
				touch.clear();
				drop_file.clear();
			}
		};
	}
}