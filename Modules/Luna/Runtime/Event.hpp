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
	//! @addtogroup Runtime
    //! @{

	//! Represents one event that once triggered, invokes all handlers registered to it.
	template <typename _Func, typename _Alloc = Allocator>
	class Event
	{
		Vector<Pair<usize, Function<_Func>>, _Alloc> m_handlers;
		usize m_next_handle = 0;
	public:
		//! Removes all handlers registered to this event.
		void clear()
		{
			m_handlers.clear();
			m_handlers.shrink_to_fit();
		}
		//! Triggers this event and invokes all handlers.
		//! @param[in] args Event arguments that will be broadcasted to every handler of this event. 
		template <typename... _Args>
		void operator()(_Args&&... args)
		{
			for (auto& handler : m_handlers)
			{
				handler.second(forward<_Args>(args)...);
			}
		}
		//! Adds one new handler to the event.
		//! @param[in] func The handler to add to this event.
		//! @return Returns one integer that can be used to remove this handler manually.
		usize add_handler(const Function<_Func>& func)
		{
			usize handle = m_next_handle++;
			m_handlers.push_back(make_pair(handle, func));
			return handle;
		}
		//! Adds one new handler to the event.
		//! @param[in] func The handler to add to this event.
		//! @return Returns one integer that can be used to remove this handler manually.
		usize add_handler(Function<_Func>&& func)
		{
			usize handle = m_next_handle++;
			m_handlers.push_back(make_pair(handle, move(func)));
			return handle;
		}
		//! Removes one registered handler.
		//! @param[in] handle The integer returned by @ref add_handler for the handler to remove.
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

	//! @}
}