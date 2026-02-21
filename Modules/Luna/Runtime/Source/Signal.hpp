/*!
* This file is a portion of LunaSDK.
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
#include "Platform/Signal.hpp"
#include "../Interface.hpp"
namespace Luna
{
    struct Signal : ISignal
    {
        lustruct("Signal", "{95a2e5b2-d48a-4f19-bfb8-22c273c0ad4b}");
        luiimpl();

        Platform::Signal m_data;

        Signal(bool manual_reset)
        {
            Platform::new_signal(manual_reset, m_data);
        }
        ~Signal()
        {
            Platform::delete_signal(m_data);
        }
        virtual void wait() override
        {
            Platform::wait_signal(m_data);
        }
        virtual bool try_wait() override
        {
            return Platform::try_wait_signal(m_data);
        }
        virtual void trigger() override
        {
            Platform::trigger_signal(m_data);
        }
        virtual void reset() override
        {
            Platform::reset_signal(m_data);
        }
    };
}