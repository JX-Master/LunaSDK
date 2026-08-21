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
#include "../StreamableHTTP.hpp"
#include <Luna/Runtime/HashMap.hpp>
#include "MCPImpl.generated.hpp"

namespace Luna
{
    namespace MCP
    {
        struct ToolHeaderBinding
        {
            Vector<Name> property_path;
            Name header_name;
            Name value_type;
        };

        struct ToolEntry
        {
            ToolDesc desc;
            Variant modern_definition;
            Variant legacy_definition;
            Vector<ToolHeaderBinding> header_bindings;
        };

        enum class LegacyLifecycle : u8
        {
            uninitialized,
            wait_initialized,
            initialized,
        };

        struct [[luna::struct("{F10F753F-734B-4DF1-86D0-594FD296C4C4}")]] MCPServer : public IMCPServer
        {
            luiimpl();

            Ref<Frontend::IFrontend> m_frontend;
            ServerDesc m_desc;
            Variant m_server_info;
            Variant m_legacy_server_info;
            HashMap<Name, ToolEntry> m_tools;

            virtual RV set_tool(ToolDesc&& desc, bool overwrite = false) override;
            virtual RV remove_tool(const Name& name) override;
            virtual usize get_tool_count() override;
            virtual R<Ref<IMCPMessageProcessor>> new_message_processor(
                ProtocolVersion version) override;
        };

        struct [[luna::struct("{FB4D6198-04DC-435D-AF22-08B495D265F2}")]] MCPMessageProcessor : public IMCPMessageProcessor
        {
            luiimpl();

            Ref<MCPServer> m_server;
            ProtocolVersion m_protocol_version = ProtocolVersion::v2026_07_28;
            LegacyLifecycle m_legacy_lifecycle = LegacyLifecycle::uninitialized;

            virtual ProtocolVersion get_protocol_version() override;
            virtual MessageResult process_message(const Variant& message) override;
            virtual R<String> process_json(const c8* json, usize json_size = USIZE_MAX) override;
        };

        struct LegacyHTTPSession
        {
            Ref<IMCPMessageProcessor> processor;
            u64 last_activity_ticks = 0;
        };

        struct [[luna::struct("{E137A2B7-C34F-4EAC-B5A4-97275999F508}")]] StreamableHTTPState
        {
            Ref<MCPServer> server;
            StreamableHTTPServerOptions options;
            HashMap<Name, LegacyHTTPSession> legacy_sessions;
            u64 next_session_id = 1;
        };
    }
}
