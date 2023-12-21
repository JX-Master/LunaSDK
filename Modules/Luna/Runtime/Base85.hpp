/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Base85.hpp
* @author JXMaster
* @date 2023/8/27
*/
#pragma once
#include "Base.hpp"

#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif

namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeBase85 Base85 encoding/decoding
	//! @}

	//! @addtogroup RuntimeBase85
	//! @{

	//! Get the encoded base85 string size from the raw data size.
	//! @param[in] raw_size The size of the raw binary data in bytes. This size must be times of 4.
	//! @return The size of the encoded string in bytes. The string size does not include the null-terminator.
	inline constexpr usize base85_get_encoded_size(usize raw_size)
	{
		return raw_size / 4 * 5;
	}

	//! Get the decoded binary size from the encoded base85 string size.
	//! @param[in] encoded_size The size of the encoded string, in bytes, and not including the null
	//! terminator. The size of the string must be times of 5.
	//! @return The size of the decoded raw data, in bytes.
	inline constexpr usize base85_get_decoded_size(usize encoded_size)
	{
		return encoded_size / 5 * 4;
	}

	//! Encode a binary data to a base85 string.
	//! @param[in] dst The character buffer used to hold the encoded base85 string.
	//! @param[in] dst_max_chars The maximum characters the `dst` buffer can hold, including the null-terminator.
	//! @param[in] src The source binary data.
	//! @param[in] src_size_bytes The size of the source binary data in bytes.
	//! @return Returned the number of characters outputted into `dst` buffer.
	//! @par Valid Usage
	//! * If `src_size_bytes` is not `0`, `src_size_bytes` must be times of 4.
	LUNA_RUNTIME_API usize base85_encode(c8* dst, usize dst_max_chars, const void* src, usize src_size_bytes);

	//! Decode a base85 string to binary data. The system assumes the string passed in is a valid base85 string.
	//! @param[in] dst The binary buffer used to hold the decoded data.
	//! @param[in] dst_max_bytes The maximum bytes the `dst` buffer can hold.
	//! @param[in] src The null-terminated base85 source string.
	//! @param[in] src_size_chars The maximum characters to read in the `src` string. Specify `USIZE_MAX` to read
	//! the full string.
	//! @return Returns the number of bytes decoded into the `dst` buffer.
	//! @par Valid Usage
	//! * If `src_size_bytes` is neither `0` nor `USIZE_MAX`, `src_size_bytes` must be times of 5.
	LUNA_RUNTIME_API usize base85_decode(void* dst, usize dst_max_bytes, const c8* src, usize src_size_chars = USIZE_MAX);

	//! @}
}