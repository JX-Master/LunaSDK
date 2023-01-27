/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Event.hpp
* @author JXMaster
* @date 2022/9/30
*/
#pragma once
#include "Vector.hpp"
namespace Luna
{
	template <typename _Function>
	class Event
	{
		Vector<_Function*> m_handlers;

	public:

		//! Remove all handlers of this event.
		void clear()
		{
			m_handlers.clear();
			m_handlers.shrink_to_fit();
		}

		//! Triggers the event and invokes all handlers.
		template <typename... _Args>
		void operator()(_Args&&... args)
		{
			for (auto& handler : m_handlers)
			{
				handler(forward<_Args>(args)...);
			}
		}

		//! Adds one new handler to the event.
		Event& operator+=(_Function* func)
		{
			m_handlers.push_back(func);
			return *this;
		}

		//! Removes one handler from the event.
		Event& operator-=(_Function* func)
		{
			for (auto iter = m_handlers.begin(); iter != m_handlers.end(); ++iter)
			{
				if (*iter == func)
				{
					m_handlers.swap_erase(iter);
					break;
				}
			}
			return *this;
		}
	};
}