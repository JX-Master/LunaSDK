/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Adapter.hpp
* @author JXMaster
* @date 2023/7/12
*/
#pragma once
#include "Common.hpp"
#include "../../Adapter.hpp"

namespace Luna
{
    namespace RHI
    {
        struct Adapter : IAdapter
        {
            lustruct("RHI::Adapter", "{0e5be888-fd9b-4036-a292-7d77ae01f111}");
            luiimpl();

            NSPtr<MTL::Device> m_device;
            c8 m_name[256];

            void init();
            virtual const c8* get_name() override
            {
                return m_name;
            }
        };
        extern Vector<Ref<IAdapter>> g_adapters;
        void init_adapters();
    }
}