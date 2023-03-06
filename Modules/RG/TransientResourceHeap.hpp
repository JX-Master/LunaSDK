/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TransientResourceHeap.hpp
* @author JXMaster
* @date 2023/3/5
*/
#pragma once
#include <RHI/RHI.hpp>

#ifndef LUNA_RG_API
#define LUNA_RG_API
#endif

namespace Luna
{
    namespace RG
    {
        struct ITransientResourceHeap : virtual Interface
        {
            luiid("{d3db2576-eba9-4e6e-8d18-580b8997d3a4}");

            virtual R<Ref<RHI::IResource>> allocate(const RHI::ResourceDesc& desc, const RHI::ClearValue* optimized_clear_value = nullptr) = 0;

            virtual void release(RHI::IResource* res) = 0;
        };

        LUNA_RG_API Ref<ITransientResourceHeap> new_transient_resource_heap(RHI::IDevice* device);
    }
}