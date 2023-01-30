/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Random.hpp
* @author JXMaster
* @date 2019/3/24
*/
#pragma once
#include "Interface.hpp"
#include "Ref.hpp"
namespace Luna
{
	//! @interface IRandom
	//! Represents a Pseudo-Random number generator.
	struct IRandom : virtual Interface
	{
		luiid("{c5a542f4-36be-45e7-8dba-9cb74adff098}");

		virtual void set_seed(u32 seed) = 0;
		virtual u32 gen_u32() = 0;
		virtual i32 gen_i32() = 0;
		virtual u64 gen_u64() = 0;
		virtual i64 gen_i64() = 0;
		virtual f32 gen_f32(f32 range_begin, f32 range_end) = 0;
		virtual f64 gen_f64(f64 range_begin, f64 range_end) = 0;
		virtual Guid gen_guid() = 0;
	};

	LUNA_RUNTIME_API Ref<IRandom> new_random_number_generator(u32 initial_seed);

	LUNA_RUNTIME_API u32 random_u32();
	LUNA_RUNTIME_API i32 random_i32();
	LUNA_RUNTIME_API u64 random_u64();
	LUNA_RUNTIME_API i64 random_i64();
	LUNA_RUNTIME_API f32 random_f32(f32 range_begin, f32 range_end);
	LUNA_RUNTIME_API f64 random_f64(f64 range_begin, f64 range_end);
	LUNA_RUNTIME_API Guid random_guid();
}