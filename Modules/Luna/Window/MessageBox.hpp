/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MessageBox.hpp
* @author JXMaster
* @date 2022/10/31
*/
#pragma once
#include <Luna/Runtime/Result.hpp>
#ifndef LUNA_WINDOW_API
#define LUNA_WINDOW_API
#endif
namespace Luna
{
	namespace Window
	{
		enum class MessageBoxType : u32
		{
			ok = 1,
			ok_cancel = 2,
			retry_cancel = 3,
			yes_no = 4,
			yes_no_cancel = 5,
		};

		enum class MessageBoxIcon : u32
		{
			none = 0,
			information = 1,
			warning = 2,
			question = 3,
			error = 4
		};

		enum class MessageBoxButton : u32
		{
			ok = 1,
			cancel,
			retry,
			yes,
			no
		};

		//! Displays one message box dialog. The current thread blocks until the dialog is closed.
		//! @return Returns the clicked button.
		LUNA_WINDOW_API R<MessageBoxButton> message_box(const c8* text, const c8* caption, MessageBoxType type, MessageBoxIcon icon = MessageBoxIcon::none);
	}
}