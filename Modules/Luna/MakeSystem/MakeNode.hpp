/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MakeNode.hpp
* @author JXMaster
* @date 2026/2/9
*/
#pragma once
#include <Luna/Runtime/TypeInfo.hpp>
#include <Luna/Runtime/Path.hpp>
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Runtime/Ref.hpp>
#include "MakeCommand.hpp"

namespace Luna
{
    namespace MakeSystem
    {
        struct MakeNode
        {
            //! The absolute path for this node.
            //! This must be unique for every node.
            Path path;

            //! The information to display when buildiong this node.
            String display_info;

            //! The dependency nodes for this node.
            Vector<MakeNode*> dependencies;

            //! The command to execute for this node.
            //! If this is null, the node is treated as already updated.
            Ref<IMakeCommand> command;

            //! Whether this node has a real file.
            bool has_file;

            //! When this is set to `true`, the current node will always be built regardless of whether it
            //! needs to be.
            bool force_rebuild;
        };
    }
}