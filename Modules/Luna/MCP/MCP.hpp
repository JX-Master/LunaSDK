/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file MCP.hpp
* @author JXMaster
* @date 2026/8/14
* @brief Model Context Protocol server APIs.
*/
#pragma once
#include <Luna/Frontend/Frontend.hpp>
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Name.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Variant.hpp>
#include <Luna/Runtime/String.hpp>
#include "MCP.generated.hpp"

#ifndef LUNA_MCP_API
#define LUNA_MCP_API
#endif

namespace Luna
{
    //! @addtogroup MCP MCP
    //! The MCP module exposes selected Frontend functions through Model Context Protocol.
    //! @{

    namespace MCP
    {
        //! The modern MCP protocol revision implemented by this module.
        constexpr const c8* MODERN_PROTOCOL_VERSION = "2026-07-28";
        //! The legacy MCP protocol revision implemented by this module.
        constexpr const c8* LEGACY_PROTOCOL_VERSION = "2025-06-18";
        //! The preferred MCP protocol revision implemented by this module.
        constexpr const c8* PROTOCOL_VERSION = MODERN_PROTOCOL_VERSION;

        //! Specifies one MCP protocol revision implemented by this module.
        enum class ProtocolVersion : u8
        {
            //! Legacy revision with an initialization lifecycle.
            v2025_06_18,
            //! Modern stateless-core revision.
            v2026_07_28,
        };

        //! Specifies where an MCP response may be cached.
        enum class CacheScope : u8
        {
            //! The response may be reused across authorization contexts.
            public_cache,
            //! The response may be reused only in the same authorization context.
            private_cache,
        };

        //! Describes the MCP server implementation and its cache policy.
        struct ServerDesc
        {
            //! Required implementation name.
            Name name;
            //! Required implementation version.
            Name version;
            //! Optional human-readable implementation title.
            Name title;
            //! Optional implementation description.
            Name description;
            //! Optional implementation website URL.
            Name website_url;
            //! Optional icon array using the MCP `Icon[]` schema.
            Variant icons;
            //! Optional natural-language instructions returned by `server/discover`.
            Name instructions;
            //! `server/discover` cache lifetime in milliseconds.
            u64 discovery_ttl_ms = 0;
            //! `tools/list` cache lifetime in milliseconds.
            u64 tools_ttl_ms = 0;
            //! Cache scope used by discovery and tool-list responses.
            CacheScope cache_scope = CacheScope::private_cache;
        };

        //! Describes one MCP tool exported from a Frontend function.
        struct ToolDesc
        {
            //! Required MCP tool name.
            Name name;
            //! Required URL of the mapped Frontend function resource.
            Name frontend_url;
            //! Optional human-readable tool title.
            Name title;
            //! Optional tool description.
            Name description;
            //! Required JSON Schema object. Its root `type` must be `object`.
            Variant input_schema;
            //! Optional JSON Schema object describing structured output.
            Variant output_schema;
            //! Optional MCP `ToolAnnotations` object.
            Variant annotations;
            //! Optional MCP `Icon[]` array.
            Variant icons;
            //! Optional MCP tool `_meta` object.
            Variant metadata;
        };

        //! Contains the result of processing one decoded MCP message.
        struct MessageResult
        {
            //! Whether a response should be sent. Notifications set this to `false`.
            bool has_response = false;
            //! The JSON-RPC response object when `has_response` is `true`.
            Variant response;
        };

        //! Configures the synchronous standard IO MCP runner.
        struct StdioServerOptions
        {
            //! Maximum number of bytes in one input message, excluding its newline terminator.
            usize max_message_size = 16 * 1024 * 1024;
        };

        //! @interface IMCPMessageProcessor
        //! Processes one ordered MCP protocol conversation for one fixed protocol revision.
        //! @details A processor retains its server. A `2025-06-18` processor owns one independent
        //! initialization lifecycle. A `2026-07-28` processor is stateless between messages.
        //! Processors and their server are not thread-safe and must not be accessed concurrently.
        struct [[Luna::interface("{E13ACDDE-D04F-4ED4-AABC-BB2372DCE807}")]] IMCPMessageProcessor : virtual Interface
        {
            //! Gets the protocol revision processed by this object.
            virtual ProtocolVersion get_protocol_version() = 0;

            //! Processes one already-decoded MCP message.
            //! @param[in] message One JSON-compatible JSON-RPC request or notification object.
            //! @return A response object for a request, or no response for a valid notification.
            //! @details JSON-RPC batch arrays are rejected. Notifications produce no response.
            //! Legacy tool requests are rejected until `notifications/initialized` is received.
            virtual MessageResult process_message(const Variant& message) = 0;

            //! Strictly decodes, processes, and strictly encodes one complete MCP JSON message.
            //! @param[in] json The input byte range. The range need not be null-terminated.
            //! @param[in] json_size The exact byte count, or @ref USIZE_MAX for a null-terminated string.
            //! @return The compact response JSON, or an empty string for a notification.
            //! Invalid JSON is returned as a JSON-RPC parse-error response.
            virtual R<String> process_json(const c8* json, usize json_size = USIZE_MAX) = 0;
        };

        //! @interface IMCPServer
        //! Owns a shared MCP service definition and exposes selected Frontend functions as tools.
        //! @details The server retains its Frontend object and contains no client lifecycle state.
        //! The server, its processors, and the retained Frontend are not thread-safe; callers must
        //! serialize all access to them.
        struct [[Luna::interface("{2CCFD788-B394-473C-AFEF-6826E67B15D3}")]] IMCPServer : virtual Interface
        {
            //! Registers or replaces one exported MCP tool.
            //! @param[in] desc The tool descriptor and Frontend mapping to consume.
            //! @param[in] overwrite Whether an existing export with the same MCP name may be replaced.
            //! @return Returns @ref E_BAD_ARGUMENTS if the descriptor is invalid or the
            //! mapped Frontend resource is not a function. Returns @ref E_ALREADY_EXISTS
            //! when the name exists and `overwrite` is `false`.
            virtual RV set_tool(ToolDesc&& desc, bool overwrite = false) = 0;

            //! Removes an exported MCP tool. Removing an absent name succeeds without effect.
            //! @param[in] name The MCP tool name to remove.
            //! @return Returns @ref E_BAD_ARGUMENTS if `name` is empty.
            virtual RV remove_tool(const Name& name) = 0;

            //! Gets the number of currently exported tools.
            //! @return The number of entries in this server's MCP tool export registry.
            virtual usize get_tool_count() = 0;

            //! Creates one independent processor for a fixed protocol revision.
            //! @param[in] version The protocol revision processed by the new object.
            //! @return The new processor, or @ref E_BAD_ARGUMENTS for an invalid revision.
            virtual R<Ref<IMCPMessageProcessor>> new_message_processor(
                ProtocolVersion version) = 0;
        };

        //! Creates a dual-protocol MCP tools server backed by `frontend`.
        //! @param[in] frontend The Frontend instance to retain and invoke.
        //! @param[in] desc The implementation identity and cache configuration to copy.
        //! @return Returns @ref E_BAD_ARGUMENTS if `frontend` or `desc` is invalid.
        LUNA_MCP_API R<Ref<IMCPServer>> new_server(
            Frontend::IFrontend* frontend,
            const ServerDesc& desc);

        //! Runs a synchronous newline-delimited MCP transport on process standard IO.
        //! @param[in] server The shared MCP server to expose.
        //! @param[in] options Framing and resource-limit options.
        //! @return Returns `ok` after a complete-frame input EOF, or an IO, framing, or
        //! serialization error.
        //! @details Standard output must be reserved exclusively for MCP response frames while this
        //! function is running. The runner selects one protocol facility from the first valid request
        //! and owns the corresponding message processor until the connection ends.
        LUNA_MCP_API RV run_stdio_server(
            IMCPServer* server,
            const StdioServerOptions& options = StdioServerOptions());

        //! Returns the MCP module pointer for use with @ref add_module.
        //! @return The process-global MCP module object.
        LUNA_MCP_API Module* module_mcp();
    }

    //! @}
}
