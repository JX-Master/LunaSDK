/*!
* This file is a portion of Luna SDK.
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
	namespace NetworkError
	{
		LUNA_NETWORK_API errcat_t errtype()
		{
			static errcat_t e = get_error_category_by_name("NetworkError");
			return e;
		}
		LUNA_NETWORK_API ErrCode not_connected()
		{
			static ErrCode e = get_error_code_by_name("NetworkError", "not_connected");
			return e;
		}
		LUNA_NETWORK_API ErrCode already_connected()
		{
			static ErrCode e = get_error_code_by_name("NetworkError", "already_connected");
			return e;
		}
		LUNA_NETWORK_API ErrCode network_down()
		{
			static ErrCode e = get_error_code_by_name("NetworkError", "network_down");
			return e;
		}
		LUNA_NETWORK_API ErrCode address_not_supported()
		{
			static ErrCode e = get_error_code_by_name("NetworkError", "address_not_supported");
			return e;
		}
		LUNA_NETWORK_API ErrCode address_in_use()
		{
			static ErrCode e = get_error_code_by_name("NetworkError", "address_in_use");
			return e;
		}
		LUNA_NETWORK_API ErrCode address_not_available()
		{
			static ErrCode e = get_error_code_by_name("NetworkError", "address_not_available");
			return e;
		}
		LUNA_NETWORK_API ErrCode network_reset()
		{
			static ErrCode e = get_error_code_by_name("NetworkError", "network_reset");
			return e;
		}
		LUNA_NETWORK_API ErrCode connection_refused()
		{
			static ErrCode e = get_error_code_by_name("NetworkError", "connection_refused");
			return e;
		}
		LUNA_NETWORK_API ErrCode connection_aborted()
		{
			static ErrCode e = get_error_code_by_name("NetworkError", "connection_aborted");
			return e;
		}
		LUNA_NETWORK_API ErrCode connection_reset()
		{
			static ErrCode e = get_error_code_by_name("NetworkError", "connection_reset");
			return e;
		}
		LUNA_NETWORK_API ErrCode network_unreachable()
		{
			static ErrCode e = get_error_code_by_name("NetworkError", "network_unreachable");
			return e;
		}
		LUNA_NETWORK_API ErrCode host_unreachable()
		{
			static ErrCode e = get_error_code_by_name("NetworkError", "host_unreachable");
			return e;
		}
		LUNA_NETWORK_API ErrCode protocol_not_supported()
		{
			static ErrCode e = get_error_code_by_name("NetworkError", "protocol_not_supported");
			return e;
		}
		LUNA_NETWORK_API ErrCode host_not_found()
		{
			static ErrCode e = get_error_code_by_name("NetworkError", "host_not_found");
			return e;
		}
		LUNA_NETWORK_API ErrCode service_not_found()
		{
			static ErrCode e = get_error_code_by_name("NetworkError", "service_not_found");
			return e;
		}
	}
}