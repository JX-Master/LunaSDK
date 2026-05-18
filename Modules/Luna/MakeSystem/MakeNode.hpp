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
        //! Defines the scheduling/update semantics of one make node.
        enum class MakeNodeKind : u8
        {
            //! A real file node. Its outputs must exist after the command runs.
            file = 0,
            //! A phony aggregation node. It is considered out of date when one
            //! dependency is out of date, but it does not require a real output.
            phony = 1,
            //! A virtual node tracked only by the MakeSystem cache.
            virtual_node = 2
        };

        struct MakeNode
        {
            //! The absolute path for this node.
            //! This must be unique for every node.
            Path path;

            //! The information to display when buildiong this node.
            String display_info;

            //! The dependency nodes for this node.
            //! @details File inputs are also represented by MakeNode objects,
            //! usually with kind=file and command=null.
            Vector<MakeNode*> dependencies;

            //! Dependencies that only order this node after other nodes. They do
            //! not make this node out of date by timestamp, but must finish
            //! before this node can run.
            Vector<MakeNode*> order_only_dependencies;

            //! Reproducible action metadata used for incremental decisions.
            MakeAction action;

            //! Additional file nodes produced by this node's command.
            //! @details The node itself is the primary output when kind=file.
            //! These nodes are side outputs of the same command and should not
            //! bind their own command.
            Vector<MakeNode*> outputs;

            //! Depfile nodes produced by this node's command.
            //! @details Depfiles are side outputs and are parsed after command
            //! execution to discover implicit dependency file paths.
            Vector<MakeNode*> depfiles;

            //! The command to execute for this node.
            //! If this is null, the node is treated as already updated.
            Ref<IMakeCommand> command;

            //! The node kind used by the v1 scheduler.
            MakeNodeKind kind = MakeNodeKind::file;

            //! When this is set to `true`, the current node will always be built regardless of whether it
            //! needs to be.
            bool force_rebuild = false;
        };
    }
}
