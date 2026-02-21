/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MakeSystem.hpp
* @author JXMaster
* @date 2026/2/9
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Path.hpp>
#include "MakeNode.hpp"
#include <Luna/Runtime/Result.hpp>

#ifndef LUNA_MAKE_SYSTEM_API
#define LUNA_MAKE_SYSTEM_API
#endif

namespace Luna
{
    namespace MakeSystem
    {
        struct IMakeSystem : virtual Interface
        {
            luiid("{24684269-d789-4b8b-af39-a65a642e29a6}");

            virtual const Path& get_build_dir() = 0;

            virtual i32 get_max_num_parallel_tasks() = 0;

            virtual RV make(Span<MakeNode*> targets) = 0;

            virtual void set_log_handler(const LogHandler& handler) = 0;
        };

        LUNA_MAKE_SYSTEM_API R<Ref<IMakeSystem>> new_make_system(const Path& build_dir, u32 max_num_parallel_tasks);
    }
}