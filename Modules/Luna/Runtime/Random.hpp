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
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeRandom Generating random numbers
	//! @}

	//! @addtogroup RuntimeRandom
	//! @{
		
	//! @interface IRandom
	//! Represents a Pseudo-random number generator.
	struct IRandom : virtual Interface
	{
		luiid("{c5a542f4-36be-45e7-8dba-9cb74adff098}");

		//! Sets random seed for the generator.
		//! @param[in] seed The new random seed to set.
		virtual void set_seed(u32 seed) = 0;
		//! Generates one random 32-bit unsigned integer.
		//! @return Returns the generated number.
		virtual u32 gen_u32() = 0;
		//! Generates one random 32-bit signed integer.
		//! @return Returns the generated number.
		virtual i32 gen_i32() = 0;
		//! Generates one random 64-bit unsigned integer.
		//! @return Returns the generated number.
		virtual u64 gen_u64() = 0;
		//! Generates one random 64-bit signed integer.
		//! @return Returns the generated number.
		virtual i64 gen_i64() = 0;
		//! Generates one random 32-bit floating-point number.
		//! @param[in] range_begin The minimum number that will be generated.
		//! @param[in] range_end The maximum number that will be generated.
		//! @return Returns the generated number in range [`range_begin`, `range_end`].
		virtual f32 gen_f32(f32 range_begin, f32 range_end) = 0;
		//! Generates one random 64-bit floating-point number.
		//! @param[in] range_begin The minimum number that will be generated.
		//! @param[in] range_end The maximum number that will be generated.
		//! @return Returns the generated number in range [`range_begin`, `range_end`].
		virtual f64 gen_f64(f64 range_begin, f64 range_end) = 0;
		//! Generates one random GUID (Globally Unique Identifier).
		//! @return Returns the generated GUID.
		virtual Guid gen_guid() = 0;
	};

	//! Creates one new random number generator.
	//! @param[in] initial_seed The initial seed for the generator.
	//! @return Returns the created random number generator.
	LUNA_RUNTIME_API Ref<IRandom> new_random_number_generator(u32 initial_seed);
	//! Generates one random 32-bit unsigned integer.
	//! @return Returns the generated number.
	LUNA_RUNTIME_API u32 random_u32();
	//! Generates one random 32-bit signed integer.
	//! @return Returns the generated number.
	LUNA_RUNTIME_API i32 random_i32();
	//! Generates one random 64-bit unsigned integer.
	//! @return Returns the generated number.
	LUNA_RUNTIME_API u64 random_u64();
	//! Generates one random 64-bit signed integer.
	//! @return Returns the generated number.
	LUNA_RUNTIME_API i64 random_i64();
	//! Generates one random 32-bit floating-point number.
	//! @param[in] range_begin The minimum number that will be generated.
	//! @param[in] range_end The maximum number that will be generated.
	//! @return Returns the generated number in range [`range_begin`, `range_end`].
	LUNA_RUNTIME_API f32 random_f32(f32 range_begin, f32 range_end);
	//! Generates one random 64-bit floating-point number.
	//! @param[in] range_begin The minimum number that will be generated.
	//! @param[in] range_end The maximum number that will be generated.
	//! @return Returns the generated number in range [`range_begin`, `range_end`].
	LUNA_RUNTIME_API f64 random_f64(f64 range_begin, f64 range_end);
	//! Generates one random GUID (Globally Unique Identifier).
	//! @return Returns the generated GUID.
	LUNA_RUNTIME_API Guid random_guid();

	//! @}
}