/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file STBImageWrite.cpp
* @author JXMaster
* @date 2020/3/7
*/
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "STBImageWrite.hpp"
#include <Luna/Runtime/Stream.hpp>

namespace Luna
{
	namespace Image
	{
		void stbi_write_func(void* context, void* data, int size)
		{
			ISeekableStream** stream = (ISeekableStream**)context;
			auto _ = (*stream)->write(data, (usize)size);
		}
	}
}