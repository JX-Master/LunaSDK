/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Adapter.h
* @author JXMaster
* @date 2023/7/12
*/
#pragma once
#include "Common.h"
#include "../../Adapter.hpp"
#include <Luna/Runtime/String.hpp>

namespace Luna
{
    namespace RHI
    {
        struct Adapter : IAdapter
        {
            lustruct("RHI::Adapter", "{0e5be888-fd9b-4036-a292-7d77ae01f111}");
            luiimpl();

            id<MTLDevice> m_device = nil;
            String m_name;

            void init();
            virtual const c8* get_name() override
            {
                return m_name.c_str();
            }
        };
        extern Vector<Ref<IAdapter>> g_adapters;
        void init_adapters();
    }
}