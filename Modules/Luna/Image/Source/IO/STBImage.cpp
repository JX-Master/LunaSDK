/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file STBImage.cpp
* @author JXMaster
* @date 2020/3/6
*/
#define STB_IMAGE_IMPLEMENTATION
#include "STBImage.hpp"

namespace Luna
{
	namespace Image
	{
		stbi_io_callbacks stbi_iocb;
		int stbi_read(void *user, char *data, int size)
		{
			ISeekableStream** stream = reinterpret_cast<ISeekableStream**>(user);
			usize read_bytes;
			RV r = (*stream)->read(data, (usize)size, &read_bytes);
			if (!r.valid())
			{
				return 0;
			}
			return (int)read_bytes;
		}

		void stbi_skip(void *user, int n)
		{
			ISeekableStream** stream = reinterpret_cast<ISeekableStream**>(user);
			auto _ = (*stream)->seek(n, SeekMode::current);
		}

		int stbi_eof(void *user)
		{
			ISeekableStream** stream = reinterpret_cast<ISeekableStream**>(user);
			return ((*stream)->tell().get() >= (*stream)->get_size());
		}

		void stbi_init()
		{
			stbi_iocb.eof = stbi_eof;
			stbi_iocb.read = stbi_read;
			stbi_iocb.skip = stbi_skip;
		}
	}
}