/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Base64.cpp
* @author JXMaster
* @date 2020/2/4
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "../Base64.hpp"

namespace Luna
{
	namespace Impl
	{
		constexpr c8 base64_encode_chars[] = u8"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		constexpr bool base64_is_valid_char(c8 c)
		{
			return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '+' || c == '/') ? true : false;
		}

		constexpr u8 base64_decode_char(c8 c)
		{
			if (c >= 'A' && c <= 'Z')
			{
				return c - 'A';
			}
			if (c >= 'a' && c <= 'z')
			{
				return c - 'a' + 26;
			}
			if (c >= '0' && c <= '9')
			{
				return c - '0' + 52;
			}
			if (c == '+')
			{
				return 62;
			}
			if (c == '/')
			{
				return 63;
			}
			// Should not be here.
			return 0;
		}
	}


	LUNA_RUNTIME_API usize base64_encode(c8* dst, usize dst_max_chars, const void* src, usize src_size_bytes)
	{
		u8 data_tuple[3];
		u8 str_tuple[4];
		u8 padding;
		usize data_cur{ 0 };
		usize str_cur{ 0 };
		const u8* data_buf = reinterpret_cast<const u8*>(src);
		while (data_cur < src_size_bytes && str_cur + 4 < dst_max_chars)
		{
			// fetch data.
			const u8* data_begin = data_buf + data_cur;
			data_tuple[0] = data_begin[0];
			padding = (src_size_bytes - data_cur) > 2 ? 0 : (u8)(3 + data_cur - src_size_bytes);
			data_tuple[1] = (padding == 2) ? 0 : data_begin[1];
			data_tuple[2] = (padding > 0) ? 0 : data_begin[2];
			// calculate
			str_tuple[0] = (data_tuple[0] & 0xfc) >> 2;
			str_tuple[1] = ((data_tuple[0] & 0x03) << 4) + ((data_tuple[1] & 0xf0) >> 4);
			str_tuple[2] = ((data_tuple[1] & 0x0f) << 2) + ((data_tuple[2] & 0xc0) >> 6);
			str_tuple[3] = data_tuple[2] & 0x3f;
			// encode
			c8* dst_begin = dst + str_cur;
			dst_begin[0] = Impl::base64_encode_chars[str_tuple[0]];
			dst_begin[1] = Impl::base64_encode_chars[str_tuple[1]];
			dst_begin[2] = (padding == 2) ? '=' : Impl::base64_encode_chars[str_tuple[2]];
			dst_begin[3] = (padding > 0) ? '=' : Impl::base64_encode_chars[str_tuple[3]];
			str_cur += 4;
			data_cur += 3;
		}
		dst[str_cur] = 0;
		return str_cur;
	}

	LUNA_RUNTIME_API usize base64_decode(void* dst, usize dst_max_bytes, const c8* src, usize src_size_chars)
	{
		u8 str_tuple[4];
		u8 padding;
		usize data_cur{ 0 };
		usize str_cur{ 0 };
		u8* data_t = reinterpret_cast<u8*>(dst);
		while ((data_cur < dst_max_bytes) && src[str_cur] && (str_cur < src_size_chars))
		{
			// decode
			padding = src[str_cur + 2] == '=' ? 2 : (src[str_cur + 3] == '=' ? 1 : 0);
			str_tuple[0] = Impl::base64_decode_char(src[str_cur]);
			str_tuple[1] = Impl::base64_decode_char(src[str_cur + 1]);
			str_tuple[2] = (padding == 2) ? 0 : Impl::base64_decode_char(src[str_cur + 2]);
			str_tuple[3] = padding ? 0 : Impl::base64_decode_char(src[str_cur + 3]);
			// construct data.
			data_t[data_cur] = (str_tuple[0] << 2) + ((str_tuple[1] & 0x30) >> 4);
			data_cur++;
			if (data_cur < dst_max_bytes && padding != 2)
			{
				data_t[data_cur] = ((str_tuple[1] & 0x0f) << 4) + ((str_tuple[2] & 0x3c) >> 2);
				data_cur++;
			}
			if (data_cur < dst_max_bytes && !padding)
			{
				data_t[data_cur] = ((str_tuple[2] & 0x03) << 6) + str_tuple[3];
				data_cur++;
			}
			str_cur += 4;
			if (padding)
			{
				break;
			}
		}
		return data_cur;
	}
}