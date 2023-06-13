/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file JSON.hpp
* @author JXMaster
* @date 2021/4/19
*/
#pragma once
#include "Variant.hpp"
#include "Result.hpp"
#include "Stream.hpp"
namespace Luna
{
	LUNA_RUNTIME_API R<Variant> json_read(const c8* src, usize src_size = USIZE_MAX);
	LUNA_RUNTIME_API R<Variant> json_read(IStream* stream);
	LUNA_RUNTIME_API String json_write(const Variant& v, bool indent = true);
	LUNA_RUNTIME_API RV json_write(IStream* stream, const Variant& v, bool indent = true);
}