/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Adapter.cpp
* @author JXMaster
* @date 2023/11/1
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "Adapter.hpp"

namespace Luna
{
    namespace RHI
    {
        Vector<Ref<IAdapter>> g_adapters;
        RV Adapter::init()
        {
            HRESULT hr = m_adapter->GetDesc1(&m_desc);
            if(FAILED(hr)) return encode_hresult(hr);
            utf16_to_utf8(m_name, 256, (char16_t*)m_desc.Description);
            return ok;
        }
        RV init_adapters()
        {
            lutry
            {
                g_adapters.clear();
                ComPtr<IDXGIAdapter1> ada;
                u32 index = 0;
                while (true)
                {
                    HRESULT hr = g_dxgi->EnumAdapters1(index, ada.ReleaseAndGetAddressOf());
                    if (FAILED(hr)) break;
                    Ref<Adapter> adapter = new_object<Adapter>();
                    adapter->m_adapter = move(ada);
                    luexp(adapter->init());
                    g_adapters.push_back(Ref<IAdapter>(adapter));
                    ++index;
                }
            }
            lucatchret;
            return ok;
        }
        LUNA_RHI_API Vector<Ref<IAdapter>> get_adapters()
        {
            return g_adapters;
        }
    }
}