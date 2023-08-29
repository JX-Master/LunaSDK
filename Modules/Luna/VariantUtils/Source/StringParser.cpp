/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StringParser.cpp
* @author JXMaster
* @date 2023/8/28
*/
#include "StringParser.hpp"

namespace Luna
{
    namespace VariantUtils
    {
        void BufferReadContext::consume(c32 ch)
        {
			if (ch == 0) return;
			cur += utf8_charspan(ch);
			if (ch == '\n')
			{
				pos = 1;
				++line;
			}
			else
			{
				++pos;
			}
        }
        c32 BufferReadContext::next_char(usize index)
        {
			const c8* next_cur = cur;
			while (index)
			{
				// advance characters.
				c32 ch = utf8_decode_char(next_cur);
				if (!ch) return 0;
				next_cur += utf8_charspan(ch);
				if ((usize)(next_cur - src) >= src_length) return 0;
				--index;
			}
			if ((usize)(next_cur - src) >= src_length) return 0;
			return utf8_decode_char(next_cur);
		}
        void StreamReadContext::consume(c32 ch)
        {
			if (ch == 0) return;
			buffer.pop_front();
			if (ch == '\n')
			{
				pos = 1;
				++line;
			}
			else
			{
				++pos;
			}
		}
        R<c32> StreamReadContext::read_one_char_from_stream()
		{
			c32 ret;
			lutry
			{
				c8 buf[6];
				usize read_bytes;
				luexp(stream->read(buf, sizeof(c8), &read_bytes));
				if (read_bytes != sizeof(c8)) return 0;
				usize charspan = utf8_charlen(buf[0]);
				if (charspan > 1)
				{
					luexp(stream->read((buf + 1), sizeof(c8) * (charspan - 1), &read_bytes));
					if (read_bytes != sizeof(c8) * (charspan - 1)) return 0;
				}
				ret = utf8_decode_char(buf);
			}
			lucatchret;
			return ret;
		}
        c32 StreamReadContext::next_char(usize index)
        {
			while (index >= buffer.size())
			{
				auto ch = read_one_char_from_stream();
				if (failed(ch) || !ch.get()) return 0;
				buffer.push_back(ch.get());
			}
			return buffer[index];
		}
    }
}