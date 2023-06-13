/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RenderPass.hpp
* @author JXMaster
* @date 2023/3/6
*/
#pragma once
#include "../RenderPass.hpp"
#include <Luna/Runtime/SelfIndexedHashMap.hpp>
#include <Luna/Runtime/Mutex.hpp>

namespace Luna
{
    namespace RG
    {
        struct RenderPassTypeDescExtractKey
        {
            const Name& operator()(const RenderPassTypeDesc& rhs) const
            {
                return rhs.name;
            }
        };
        extern SelfIndexedHashMap<Name, RenderPassTypeDesc, RenderPassTypeDescExtractKey> g_render_pass_types;
        extern Ref<IMutex> g_render_pass_types_mtx;
    }
}