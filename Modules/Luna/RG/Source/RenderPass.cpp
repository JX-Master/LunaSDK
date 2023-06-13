/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RenderPass.cpp
* @author JXMaster
* @date 2023/3/6
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RG_API LUNA_EXPORT
#include "RenderPass.hpp"

namespace Luna
{
    namespace RG
    {
        SelfIndexedHashMap<Name, RenderPassTypeDesc, RenderPassTypeDescExtractKey> g_render_pass_types;
        Ref<IMutex> g_render_pass_types_mtx;

        LUNA_RG_API void register_render_pass_type(const RenderPassTypeDesc& desc)
        {
            MutexGuard guard(g_render_pass_types_mtx);
            g_render_pass_types.insert(desc);
        }

        LUNA_RG_API void get_render_pass_types(Vector<Name>& out_render_pass_types)
        {
            MutexGuard guard(g_render_pass_types_mtx);
            for(auto& i : g_render_pass_types)
            {
                out_render_pass_types.push_back(i.name);
            }
        }

        LUNA_RG_API R<RenderPassTypeDesc> get_render_pass_type_desc(const Name& render_pass)
        {
            MutexGuard guard(g_render_pass_types_mtx);
            auto iter = g_render_pass_types.find(render_pass);
            if(iter == g_render_pass_types.end()) return BasicError::not_found();
            return *iter;
        }
    }
}