/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file D3D12RHI.cpp
* @author JXMaster
* @date 2019/7/10
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "../../RHI.hpp"
#include <d3d12.h>
#include "D3D12Device.hpp"
#include "D3D12SwapChain.hpp"
#include <Luna/Runtime/Unicode.hpp>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#include "../RHI.hpp"

#include "D3D12PipelineLayout.hpp"
#include "D3D12PipelineState.hpp"
#include "D3D12CommandBuffer.hpp"
#include "D3D12SwapChain.hpp"
#include "D3D12DescriptorSet.hpp"
#include "D3D12DescriptorSetLayout.hpp"
#include "D3D12QueryHeap.hpp"
#include "D3D12Fence.hpp"
#include "D3D12Adapter.hpp"
#include "RHI.meta.generated.hpp"

namespace Luna
{
    namespace RHI
    {
        ComPtr<IDXGIFactory5> g_dxgi;
        Ref<IDevice> g_main_device;

        RV render_api_init()
        {
            Meta::register_RHI_types();

            HRESULT hr = ::CreateDXGIFactory1(IID_PPV_ARGS(&g_dxgi));
            if (FAILED(hr))
            {
                return encode_hresult(hr);
            }
            lutry
            {
                luexp(init_adapters());
                auto adapters = get_adapters();
    #if (defined(LUNA_RHI_DEBUG) || defined(LUNA_DEBUG)) && (LUNA_PLATFORM_VERSION >= LUNA_PLATFORM_VERSION_WIN10)
                ComPtr<ID3D12Debug> debug;
                D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
                if (debug) debug->EnableDebugLayer();
    #endif
                luset(g_main_device, new_device(adapters[0]));
            }
            lucatchret;
            return ok;
        }
        void render_api_close()
        {
            g_main_device = nullptr;
            g_adapters.clear();
            g_adapters.shrink_to_fit();
            g_dxgi = nullptr;
        }
        LUNA_RHI_API BackendType get_backend_type()
        {
            return BackendType::d3d12;
        }
    }
}
