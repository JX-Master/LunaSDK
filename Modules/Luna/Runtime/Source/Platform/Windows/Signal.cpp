/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Signal.cpp
* @author JXMaster
* @date 2022/3/10
*/
#include "../../OS.hpp"
#include "../../../Platform/Windows/MiniWin.hpp"

namespace Luna
{
	namespace OS
	{
		struct Signal
		{
			CRITICAL_SECTION m_cs;
			CONDITION_VARIABLE m_cv;
			volatile bool m_signaled;
			volatile bool m_manual_reset;
		};
		opaque_t new_signal(bool manual_reset)
		{
			Signal* ret = Luna::memnew<Signal>();
			::InitializeCriticalSection(&(ret->m_cs));
			::InitializeConditionVariable(&(ret->m_cv));
			ret->m_signaled = false;
			ret->m_manual_reset = manual_reset;
			return ret;
		}
		void delete_signal(opaque_t signal)
		{
			Signal* sig = (Signal*)signal;
			::DeleteCriticalSection(&(sig->m_cs));
			Luna::memdelete(sig);
		}
		void wait_signal(opaque_t signal)
		{
			Signal* sig = (Signal*)signal;
			::EnterCriticalSection(&(sig->m_cs));
			// If the signal is not signaled, waits until it gets signaled.
			while (!sig->m_signaled)
			{
				if (!::SleepConditionVariableCS(&(sig->m_cv), &(sig->m_cs), INFINITE))
				{
					// Failed to wait. This should never happen if the wait time is INFINITE.
					::LeaveCriticalSection(&(sig->m_cs));
					lupanic_always();
				}
			}
			// If not manual reset, consumes the signal so that other waiting threads
			// can not go here.
			if (!sig->m_manual_reset)
			{
				sig->m_signaled = false;
			}
			::LeaveCriticalSection(&(sig->m_cs));
		}
		bool try_wait_signal(opaque_t signal)
		{
			Signal* sig = (Signal*)signal;
			if (!::TryEnterCriticalSection(&(sig->m_cs)))
			{
				return false;
			}
			if (!sig->m_signaled)
			{
				if (!::SleepConditionVariableCS(&(sig->m_cv), &(sig->m_cs), 0))
				{
					// Failed to wait.
					::LeaveCriticalSection(&(sig->m_cs));
					return false;
				}
			}
			// If not manual reset, consumes the signal so that other waiting threads
			// can not go here.
			if (!sig->m_manual_reset)
			{
				sig->m_signaled = false;
			}
			::LeaveCriticalSection(&(sig->m_cs));
			return true;
		}
		void trigger_signal(opaque_t signal)
		{
			Signal* sig = (Signal*)signal;
			::EnterCriticalSection(&(sig->m_cs));
			sig->m_signaled = true;
			if (sig->m_manual_reset)
			{
				// Wake all threads.
				::WakeAllConditionVariable(&(sig->m_cv));
			}
			else
			{
				// Wake exactly one thread.
				::WakeConditionVariable(&(sig->m_cv));
			}
			::LeaveCriticalSection(&(sig->m_cs));
		}
		void reset_signal(opaque_t signal)
		{
			Signal* sig = (Signal*)signal;
			::EnterCriticalSection(&(sig->m_cs));
			sig->m_signaled = false;
			::LeaveCriticalSection(&(sig->m_cs));
		}
	}
}