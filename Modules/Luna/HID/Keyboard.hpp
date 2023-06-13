/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Keyboard.hpp
* @author JXMaster
* @date 2019/7/25
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include "KeyCode.hpp"

namespace Luna
{
	namespace HID
	{
		//! @interface IKeyboard
		//! Represent keyboard device.
		struct IKeyboard : virtual Interface
		{
			luiid("{076A4AB9-E843-4DDB-84BF-C542DA33D519}");
			//! Checks the if the specified key is pressed.
			//! @param[in] key The key to check.
			//! @return	`true` if the key is pressed, `false` otherwise.
			virtual bool get_key_state(KeyCode key) = 0;
		};
	}
}