/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Signal.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "../../Base.hpp"
#if defined(LUNA_PLATFORM_WINDOWS)
#include "../../Platform/Windows/MiniWin.hpp"
#elif defined(LUNA_PLATFORM_POSIX)

#endif

namespace Luna
{
    namespace Platform
    {
        struct Signal
        {
#if defined(LUNA_PLATFORM_WINDOWS)
            CRITICAL_SECTION m_cs;
            CONDITION_VARIABLE m_cv;
            bool m_signaled;
            bool m_manual_reset;
#elif defined(LUNA_PLATFORM_POSIX)

#endif
        };
        //! Creates one signal object.
        void new_signal(bool manual_reset, Signal& out_signal);

        //! Destroys one signal object.
        void delete_signal(Signal& signal);

        //! Waits for the signal to be triggered.
        void wait_signal(Signal& signal);

        //! Try to wait for the signal is triggered, returns immediately if the signal is not triggered.
        bool try_wait_signal(Signal& signal);

        //! Triggers the signal.
        void trigger_signal(Signal& signal);

        //! Resets the signal to non-triggered state. 
        //! This operation is valid only for the signal with `manual_reset` set to `true` when creating the signal.
        void reset_signal(Signal& signal);
    }
}