/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VM.hpp
* @author JXMaster
* @date 2022/7/4
*/
#include "TypeInfo.hpp"
namespace Luna
{
	enum class OpCode : u16
	{
		// Null operation.
		nop = 0,
		// Allocates `N` bytes on the stack, and increases SP.
		push,
		// Deallocates `n` bytes from the stack, and decreases SP.
		pop,
		// Sets all memory to zero.
		zero,
		// Default-constructs the value on the specified address.
		ctor,
		// Default-destructs 
		dtor,
		cctor,
		mctor,
		cassign,
		massign,
	};
}