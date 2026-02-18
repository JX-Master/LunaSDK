/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Signal.cpp
* @author JXMaster
* @date 2022/3/10
*/
#include "../Signal.hpp"
#include "../../../Assert.hpp"

namespace Luna
{
    namespace Platform
    {
        void new_signal(bool manual_reset, Signal& out_signal)
        {
            ::InitializeCriticalSection(&(out_signal.m_cs));
            ::InitializeConditionVariable(&(out_signal.m_cv));
            out_signal.m_signaled = false;
            out_signal.m_manual_reset = manual_reset;
        }
        void delete_signal(Signal& signal)
        {
            ::DeleteCriticalSection(&(signal.m_cs));
        }
        void wait_signal(Signal& signal)
        {
            ::EnterCriticalSection(&(signal.m_cs));
            // If the signal is not signaled, waits until it gets signaled.
            while (!signal.m_signaled)
            {
                if (!::SleepConditionVariableCS(&(signal.m_cv), &(signal.m_cs), INFINITE))
                {
                    // Failed to wait. This should never happen if the wait time is INFINITE.
                    ::LeaveCriticalSection(&(signal.m_cs));
                    lupanic_always();
                }
            }
            // If not manual reset, consumes the signal so that other waiting threads
            // can not go here.
            if (!signal.m_manual_reset)
            {
                signal.m_signaled = false;
            }
            ::LeaveCriticalSection(&(signal.m_cs));
        }
        bool try_wait_signal(Signal& signal)
        {
            if (!::TryEnterCriticalSection(&(signal.m_cs)))
            {
                return false;
            }
            if (!signal.m_signaled)
            {
                if (!::SleepConditionVariableCS(&(signal.m_cv), &(signal.m_cs), 0))
                {
                    // Failed to wait.
                    ::LeaveCriticalSection(&(signal.m_cs));
                    return false;
                }
            }
            // If not manual reset, consumes the signal so that other waiting threads
            // can not go here.
            if (!signal.m_manual_reset)
            {
                signal.m_signaled = false;
            }
            ::LeaveCriticalSection(&(signal.m_cs));
            return true;
        }
        void trigger_signal(Signal& signal)
        {
            ::EnterCriticalSection(&(signal.m_cs));
            signal.m_signaled = true;
            if (signal.m_manual_reset)
            {
                // Wake all threads.
                ::WakeAllConditionVariable(&(signal.m_cv));
            }
            else
            {
                // Wake exactly one thread.
                ::WakeConditionVariable(&(signal.m_cv));
            }
            ::LeaveCriticalSection(&(signal.m_cs));
        }
        void reset_signal(Signal& signal)
        {
            ::EnterCriticalSection(&(signal.m_cs));
            signal.m_signaled = false;
            ::LeaveCriticalSection(&(signal.m_cs));
        }
    }
}