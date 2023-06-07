/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Signal.hpp
* @author JXMaster
* @date 2018/12/22
* @brief Windows implementation of Sync System.
*/
#pragma once
#include "../Signal.hpp"
#include "OS.hpp"
#include "../Interface.hpp"
namespace Luna
{
	struct Signal : ISignal
	{
		lustruct("Signal", "{95a2e5b2-d48a-4f19-bfb8-22c273c0ad4b}");
		luiimpl();

		opaque_t m_handle;

		Signal(bool manual_reset)
		{
			m_handle = OS::new_signal(manual_reset);
		}
		~Signal()
		{
			OS::delete_signal(m_handle);
		}
		virtual void wait() override
		{
			OS::wait_signal(m_handle);
		}
		virtual bool try_wait() override
		{
			return OS::try_wait_signal(m_handle);
		}
		virtual void trigger() override
		{
			OS::trigger_signal(m_handle);
		}
		virtual void reset() override
		{
			OS::reset_signal(m_handle);
		}
	};
}