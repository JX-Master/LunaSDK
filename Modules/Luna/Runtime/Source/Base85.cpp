/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Base85.cpp
* @author JXMaster
* @date 2023/8/27
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "../Base85.hpp"

namespace Luna
{
	LUNA_RUNTIME_API usize base85_encode(c8* dst, usize dst_max_chars, const void* src, usize src_size_bytes)
	{
		u8 str_tuple[5];
		u32 data_int;
		usize data_cur{ 0 };
		usize str_cur{ 0 };
		const u8* data_buf = reinterpret_cast<const u8*>(src);
		while (data_cur + 4 <= src_size_bytes && str_cur + 5 < dst_max_chars)
		{
			// construct 32-bit unsigned integer using 4 bytes (little endian).
			const u8* data_begin = data_buf + data_cur;
			data_int = (u32)data_begin[3] + (((u32)data_begin[2]) << 8) + (((u32)data_begin[1]) << 16) + (((u32)data_begin[0]) << 24);
			// calculate
			str_tuple[4] = (u8)(data_int % 85);
			data_int /= 85;
			str_tuple[3] = (u8)(data_int % 85);
			data_int /= 85;
			str_tuple[2] = (u8)(data_int % 85);
			data_int /= 85;
			str_tuple[1] = (u8)(data_int % 85);
			data_int /= 85;
			str_tuple[0] = (u8)(data_int % 85);
			// encode
			c8* dst_begin = dst + str_cur;
			dst_begin[0] = str_tuple[0] + 33;
			dst_begin[1] = str_tuple[1] + 33;
			dst_begin[2] = str_tuple[2] + 33;
			dst_begin[3] = str_tuple[3] + 33;
			dst_begin[4] = str_tuple[4] + 33;
			data_cur += 4;
			str_cur += 5;
		}
		dst[str_cur] = 0;
		return str_cur;
	}
	LUNA_RUNTIME_API usize base85_decode(void* dst, usize dst_max_bytes, const c8* src, usize src_size_chars)
	{
		u8 str_tuple[5];
		u32 data_int;
		usize data_cur{ 0 };
		usize str_cur{ 0 };
		u8* data_buf = reinterpret_cast<u8*>(dst);
		if (src_size_chars == USIZE_MAX) src_size_chars = strlen(src);
		while (data_cur + 4 <= dst_max_bytes && str_cur + 5 <= src_size_chars)
		{
			const c8* src_begin = src + str_cur;
			str_tuple[0] = src_begin[0] - 33;
			str_tuple[1] = src_begin[1] - 33;
			str_tuple[2] = src_begin[2] - 33;
			str_tuple[3] = src_begin[3] - 33;
			str_tuple[4] = src_begin[4] - 33;
			data_int = ((u32)str_tuple[4]) + ((u32)str_tuple[3]) * 85 + ((u32)str_tuple[2]) * (85 * 85) + 
				((u32)str_tuple[1]) * (85 * 85 * 85) + ((u32)str_tuple[0]) * (85 * 85 * 85 * 85);
			u8* data_begin = data_buf + data_cur;
			data_begin[3] = (u8)(data_int & 0xFF);
			data_begin[2] = (u8)((data_int >> 8) & 0xFF);
			data_begin[1] = (u8)((data_int >> 16) & 0xFF);
			data_begin[0] = (u8)((data_int >> 24) & 0xFF);
			data_cur += 4;
			str_cur += 5;
		}
		return data_cur;
	}
}