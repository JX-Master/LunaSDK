/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file MCPImpl.hpp
* @author JXMaster
* @date 2026/8/14
* @brief Internal MCP server declarations.
*/
#pragma once
#include "../MCP.hpp"
#include <Luna/Runtime/HashMap.hpp>
#include "MCPImpl.generated.hpp"

namespace Luna
{
    namespace MCP
    {
        struct ToolEntry
        {
            ToolDesc desc;
            Variant modern_definition;
            Variant legacy_definition;
        };

        enum class ProtocolFacility : u8
        {
            undetermined,
            modern,
            legacy_uninitialized,
            legacy_wait_initialized,
            legacy_initialized,
        };

        struct [[luna::struct("{F10F753F-734B-4DF1-86D0-594FD296C4C4}")]] MCPServer : public IMCPServer
        {
            luiimpl();

            Ref<Frontend::IFrontend> m_frontend;
            ServerDesc m_desc;
            Variant m_server_info;
            Variant m_legacy_server_info;
            HashMap<Name, ToolEntry> m_tools;
            ProtocolFacility m_protocol_facility = ProtocolFacility::undetermined;

            virtual RV set_tool(ToolDesc&& desc, bool overwrite = false) override;
            virtual RV remove_tool(const Name& name) override;
            virtual usize get_tool_count() override;
            virtual MessageResult process_message(const Variant& message) override;
            virtual R<String> process_json(const c8* json, usize json_size = USIZE_MAX) override;
        };
    }
}
