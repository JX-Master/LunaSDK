/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file NetworkError.cpp
* @author JXMaster
* @date 2022/6/2
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_NETWORK_API LUNA_EXPORT
#include "../Network.hpp"

namespace Luna
{
    namespace Network
    {
        RV register_error_codes()
        {
            if (!register_error_category(ERROR_CATEGORY, "Network") ||
                !register_error_code(E_NOT_CONNECTED, "not_connected", "The socket is not connected.") ||
                !register_error_code(E_ALREADY_CONNECTED, "already_connected", "The socket is already connected.") ||
                !register_error_code(E_NETWORK_DOWN, "network_down", "The network subsystem has failed.") ||
                !register_error_code(E_ADDRESS_NOT_SUPPORTED, "address_not_supported", "The address family is not supported.") ||
                !register_error_code(E_ADDRESS_IN_USE, "address_in_use", "The address is already in use.") ||
                !register_error_code(E_ADDRESS_NOT_AVAILABLE, "address_not_available", "The address is not available.") ||
                !register_error_code(E_NETWORK_RESET, "network_reset", "The network connection was reset.") ||
                !register_error_code(E_CONNECTION_REFUSED, "connection_refused", "The connection attempt was refused.") ||
                !register_error_code(E_CONNECTION_ABORTED, "connection_aborted", "The connection was aborted.") ||
                !register_error_code(E_CONNECTION_RESET, "connection_reset", "The connection was reset by its peer.") ||
                !register_error_code(E_NETWORK_UNREACHABLE, "network_unreachable", "The network is unreachable.") ||
                !register_error_code(E_HOST_UNREACHABLE, "host_unreachable", "The host is unreachable.") ||
                !register_error_code(E_PROTOCOL_NOT_SUPPORTED, "protocol_not_supported", "The protocol is not supported.") ||
                !register_error_code(E_HOST_NOT_FOUND, "host_not_found", "The host was not found.") ||
                !register_error_code(E_SERVICE_NOT_FOUND, "service_not_found", "The network service was not found."))
            {
                return set_error(E_ALREADY_EXISTS, "Network error metadata conflicts with an existing registration.");
            }
            return ok;
        }

    }
}
