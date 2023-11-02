/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Adapter.hpp
* @author JXMaster
* @date 2023/11/1
*/
#pragma once

#include "D3D12Common.hpp"
#include "../../Adapter.hpp"

namespace Luna
{
    namespace RHI
    {
        struct Adapter : IAdapter
        {
            lustruct("RHI::Adapter", "{81cec7a5-b7ed-4b53-9fa0-56dbcb3dd514}");
            luiimpl();

            ComPtr<IDXGIAdapter1> m_adapter;
            DXGI_ADAPTER_DESC1 m_desc;
            c8 m_name[256];

            RV init();
            virtual const c8* get_name() override
            {
                return m_name;
            }
        };

        extern Vector<Ref<IAdapter>> g_adapters;
        RV init_adapters();
    }
}