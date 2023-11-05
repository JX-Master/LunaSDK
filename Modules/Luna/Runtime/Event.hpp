/*!
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
#include "Functional.hpp"
#include "Name.hpp"
namespace Luna
{
	template <typename _Func, typename _Alloc = Allocator>
	class Event
	{
		Vector<Pair<usize, Function<_Func>>, _Alloc> m_handlers;
		usize m_next_handle = 0;
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
				handler.second(forward<_Args>(args)...);
			}
		}
		//! Adds one new handler to the event.
		usize add_handler(const Function<_Func>& func)
		{
			usize handle = m_next_handle++;
			m_handlers.push_back(make_pair(handle, func));
			return handle;
		}
		usize add_handler(Function<_Func>&& func)
		{
			usize handle = m_next_handle++;
			m_handlers.push_back(make_pair(handle, move(func)));
			return handle;
		}
		void remove_handler(usize handle)
		{
			for (auto iter = m_handlers.begin(); iter != m_handlers.end(); ++iter)
			{
				if (iter->first == handle)
				{
					m_handlers.erase(iter);
					break;
				}
			}
		}
	};
}