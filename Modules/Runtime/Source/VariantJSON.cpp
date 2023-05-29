/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file JSON.cpp
* @author JXMaster
* @date 2021/8/3
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "../VariantJSON.hpp"
#include "../Unicode.hpp"
#include "../Base64.hpp"
#include "../RingDeque.hpp"

namespace Luna
{
	namespace JSON
	{
		inline bool is_whitespace(c32 ch)
		{
			return (ch == 0x20) || (ch == 0xA0) || (ch == 0x0A) || (ch == 0x0D) || (ch == 0x09);
		}

		struct IReadContext
		{
			virtual void consume(c32 ch) = 0;
			virtual c32 next_char(usize index = 0) = 0;
			virtual void skip_whitespaces_and_comments() = 0;
			virtual u32 get_line() = 0;
			virtual u32 get_pos() = 0;
		};

		struct BufferReadContext : public IReadContext
		{
			const c8* src;
			const c8* cur;
			usize src_length;
			u32 line;
			u32 pos;

			void consume(c32 ch)
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
			c32 next_char(usize index = 0)
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
			virtual u32 get_line() { return line; }
			virtual u32 get_pos() { return pos; }
		private:
			void skip_single_line_comment()
			{
				lucheck(next_char() == '/' && next_char(1) == '/');
				consume('/');
				consume('/');
				c32 ch = next_char();
				if (!ch) return;
				while (ch != '\n')
				{
					consume(ch);
					ch = next_char();
					if (!ch) return;
				}
				consume(ch);// for \n.
			}
			void skip_multi_line_comment()
			{
				lucheck(next_char() == '/' && next_char(1) == '*');
				consume('/');
				consume('*');
				c32 ch = next_char();
				if (!ch) return;
			entry:
				while (ch != '*')
				{
					consume(ch);
					ch = next_char();
					if (!ch) return;
				}
				consume(ch); // for *.
				ch = next_char();
				if (!ch) return;
				if (ch == '/')
				{
					consume(ch); // for /.
					return;
				}
				else
				{
					goto entry;
				}
			}
		public:
			void skip_whitespaces_and_comments()
			{
				c32 ch = next_char();
				while (ch)
				{
					if (is_whitespace(ch))
					{
						consume(ch);
					}
					else if (ch == '/')
					{
						c32 ch2 = next_char(1);
						if (ch2 == '/')
						{
							skip_single_line_comment();
						}
						else if (ch2 == '*')
						{
							skip_multi_line_comment();
						}
						else
						{
							break;
						}
					}
					else break;
					ch = next_char();
				}
			}
		};

		struct StreamReadContext : public IReadContext
		{
			IStream* stream;
			RingDeque<c32> buffer;
			u32 line;
			u32 pos;
			void consume(c32 ch)
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
		private:
			R<c32> read_one_char_from_stream()
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
		public:
			c32 next_char(usize index = 0)
			{
				while (index >= buffer.size())
				{
					auto ch = read_one_char_from_stream();
					if (failed(ch) || !ch.get()) return 0;
					buffer.push_back(ch.get());
				}
				return buffer[index];
			}
			virtual u32 get_line() { return line; }
			virtual u32 get_pos() { return pos; }
		private:
			void skip_single_line_comment()
			{
				lucheck(next_char() == '/' && next_char(1) == '/');
				consume('/');
				consume('/');
				c32 ch = next_char();
				if (!ch) return;
				while (ch != '\n')
				{
					consume(ch);
					ch = next_char();
					if (!ch) return;
				}
				consume(ch);// for \n.
			}
			void skip_multi_line_comment()
			{
				lucheck(next_char() == '/' && next_char(1) == '*');
				consume('/');
				consume('*');
				c32 ch = next_char();
				if (!ch) return;
			entry:
				while (ch != '*')
				{
					consume(ch);
					ch = next_char();
					if (!ch) return;
				}
				consume(ch); // for *.
				ch = next_char();
				if (!ch) return;
				if (ch == '/')
				{
					consume(ch); // for /.
					return;
				}
				else
				{
					goto entry;
				}
			}
		public:
			void skip_whitespaces_and_comments()
			{
				c32 ch = next_char();
				while (ch)
				{
					if (is_whitespace(ch))
					{
						consume(ch);
					}
					else if (ch == '/')
					{
						c32 ch2 = next_char(1);
						if (ch2 == '/')
						{
							skip_single_line_comment();
						}
						else if (ch2 == '*')
						{
							skip_multi_line_comment();
						}
						else
						{
							break;
						}
					}
					else break;
					ch = next_char();
				}
			}
		};

		static R<String> read_string_literal(IReadContext& ctx)
		{
			lucheck(ctx.next_char() == '"');
			ctx.consume('"');
			String s;
			c32 ch = ctx.next_char();
			while (ch)
			{
				if (ch == '\\')
				{
					ctx.consume(ch); // for '\\'
					ch = ctx.next_char();
					c32 ch2;
					switch (ch)
					{
					case '"':
						ch2 = '\"'; ctx.consume(ch); break;
					case '\\':
						ch2 = '\\'; ctx.consume(ch); break;
					case '/':
						ch2 = '/'; ctx.consume(ch); break;
					case 'b':
						ch2 = '\b'; ctx.consume(ch); break;
					case 'f':
						ch2 = '\f'; ctx.consume(ch); break;
					case 'n':
						ch2 = '\n'; ctx.consume(ch); break;
					case 'r':
						ch2 = '\r'; ctx.consume(ch); break;
					case 't':
						ch2 = '\t'; ctx.consume(ch); break;
					case '0':
						ch2 = '\0'; ctx.consume(ch); break;
					case '\'':
						ch2 = '\''; ctx.consume(ch); break;
					case 'u':
					{
						ctx.consume(ch); // for u
						ch = ctx.next_char();
						u32 unicode_i = 0;
						for (u32 i = 0; i < 4; ++i)
						{
							if (!(ch >= '0' && ch <= '9') && !(ch >= 'a' && ch <= 'f') && !(ch >= 'A' && ch <= 'F'))
							{
								return set_error(BasicError::format_error(), "Invalid Unicode number at line %d, pos %d.", ctx.get_line(), ctx.get_pos());
							}
							unicode_i <<= 4;
							if (ch >= '0' && ch <= '9')
							{
								unicode_i += ch - '0';
							}
							else if (ch >= 'a' && ch <= 'f')
							{
								unicode_i += ch - 'a' + 10;
							}
							else
							{
								unicode_i += ch - 'A' + 10;
							}
							ctx.consume(ch);
							ch = ctx.next_char();
						}
						ch2 = (c32)unicode_i;
					}
					break;
					default:
						return set_error(BasicError::format_error(), "Invalid character appeared after \"\\\" at line %d, pos %d.", ctx.get_line(), ctx.get_pos());
					}
					c8 buf[6];
					usize buf_count = utf8_encode_char(buf, ch2);
					for (usize i = 0; i < buf_count; ++i)
					{
						s.push_back(buf[i]);
					}
					ch = ctx.next_char();
					continue;
				}
				if (ch == '"')
				{
					ctx.consume(ch); // for ".
					break;
				}
				c8 buf[6];
				usize buf_count = utf8_encode_char(buf, ch);
				for (usize i = 0; i < buf_count; ++i)
				{
					s.push_back(buf[i]);
				}
				ctx.consume(ch);
				ch = ctx.next_char();
			}
			return s;
		}
		static R<Variant> read_value(IReadContext& ctx);
		static R<Variant> read_object(IReadContext& ctx)
		{
			lucheck(ctx.next_char() == '{');
			ctx.consume('{');
			ctx.skip_whitespaces_and_comments();
			Variant v(VariantType::object);
			c32 ch = ctx.next_char();
			while (ch && ch != '}')
			{
				if (ch != '"') return set_error(BasicError::format_error(), "The object field must start with a string name (line %d pos %d).", ctx.get_line(), ctx.get_pos());
				R<String> name_str = read_string_literal(ctx);
				if (failed(name_str)) return name_str.errcode();
				ctx.skip_whitespaces_and_comments();
				ch = ctx.next_char();
				if (ch != ':') return set_error(BasicError::format_error(), "':' expected at the end of the field name (line %d pos %d).", ctx.get_line(), ctx.get_pos());
				ctx.consume(ch);
				R<Variant> val = read_value(ctx);
				if (failed(val)) return val.errcode();
				v.insert(Name(name_str.get()), move(val.get()));
				ctx.skip_whitespaces_and_comments();
				ch = ctx.next_char();
				if (ch == '}') break;
				if (ch != ',') set_error(BasicError::format_error(), "',' expected at the end of the field (line %d pos %d).", ctx.get_line(), ctx.get_pos());
				ctx.consume(ch);
				ctx.skip_whitespaces_and_comments();
				ch = ctx.next_char();
			}
			if (!ch) return set_error(BasicError::format_error(), "Unexpected EOF occurred at line %d, pos %d.", ctx.get_line(), ctx.get_pos());
			ctx.consume('}');
			return v;
		}

		static R<Variant> read_array(IReadContext& ctx)
		{
			lucheck(ctx.next_char() == '[');
			ctx.consume('[');
			ctx.skip_whitespaces_and_comments();
			Variant v(VariantType::array);
			c32 ch = ctx.next_char();
			while (ch && ch != ']')
			{
				R<Variant> val = read_value(ctx);
				if (failed(val)) return val.errcode();
				v.push_back(move(val.get()));
				ctx.skip_whitespaces_and_comments();
				ch = ctx.next_char();
				if (ch == ']') break;
				if (ch != ',') set_error(BasicError::format_error(), "',' expected at the end of every array item (line %d pos %d).", ctx.get_line(), ctx.get_pos());
				ctx.consume(ch);
				ctx.skip_whitespaces_and_comments();
				ch = ctx.next_char();
			}
			if (!ch) return set_error(BasicError::format_error(), "Unexpected EOF occurred at line %d, pos %d.", ctx.get_line(), ctx.get_pos());
			ctx.consume(']');
			return v;
		}

		static R<Variant> read_blob(const String& str)
		{
			// Check if this is a blob.
			if (!memcmp(str.c_str(), "@blob@", 6 * sizeof(c8)))
			{
				c8* end_chr;
				u64 size = strtoll(str.c_str() + 6, &end_chr, 10);
				if (*end_chr != '@') return BasicError::failure();
				u64 alignment = strtoll(end_chr + 1, &end_chr, 10);
				if (*end_chr != '@') return BasicError::failure();
				Blob data(size, alignment);
				base64_decode(data.data(), data.size(), end_chr + 1);
				return Variant(move(data));
			}
			return BasicError::failure();
		}

		static R<Variant> read_string_or_blob(IReadContext& ctx)
		{
			R<String> s = read_string_literal(ctx);
			if (failed(s)) return s.errcode();
			auto blob = read_blob(s.get());
			if (blob.valid()) return blob;
			return Variant(Name(move(s.get())));
		}

		static Variant read_number(IReadContext& ctx)
		{
			String32 integral;
			String32 decimal;
			String32 exponent;
			bool is_integral_positive = true;
			bool is_exponent_positive = true;
			bool is_floating_point = false;
			c32 ch = ctx.next_char();
			// Process negative value.
			if (ch == '-')
			{
				is_integral_positive = false;
				ctx.consume(ch);
				ch = ctx.next_char();
			}
			// Process integral part.
			while (ch)
			{
				if (ch >= '0' && ch <= '9')
				{
					integral.push_back(ch);
					ctx.consume(ch);
					ch = ctx.next_char();
					continue;
				}
				else
				{
					break;
				}
			}
			// Process decimal part.
			if (ch == '.')
			{
				is_floating_point = true;
				ctx.consume(ch);
				ch = ctx.next_char();
				while (ch)
				{
					if (ch >= '0' && ch <= '9')
					{
						decimal.push_back(ch);
						ctx.consume(ch);
						ch = ctx.next_char();
						continue;
					}
					else
					{
						break;
					}
				}
			}
			// Process exponent part.
			if (ch == 'e' || ch == 'E')
			{
				is_floating_point = true;
				ctx.consume(ch);
				ch = ctx.next_char();
				if (ch == '+' || ch == '-')
				{
					is_exponent_positive = (ch == '+');
					ctx.consume(ch);
					ch = ctx.next_char();
				}
				while (ch)
				{
					if (ch >= '0' && ch <= '9')
					{
						exponent.push_back(ch);
						ctx.consume(ch);
						ch = ctx.next_char();
						continue;
					}
					else
					{
						break;
					}
				}
			}
			// Parse.
			if (is_floating_point)
			{
				f64 value = 0.0;
				for (auto& i : integral)
				{
					value *= 10;
					value += i - '0';
				}
				f64 decimal_base = 0.1;
				for (auto& i : decimal)
				{
					value += decimal_base * (i - '0');
					decimal_base /= 10;
				}
				i64 exp = 0;
				for (auto& i : exponent)
				{
					exp *= 10;
					exp += i - '0';
				}
				exp = is_exponent_positive ? exp : -exp;
				while (exp > 0)
				{
					value *= 10.0;
					exp--;
				}
				while (exp < 0)
				{
					value *= 0.1;
					exp++;
				}
				value = is_integral_positive ? value : -value;
				return Variant(value);
			}
			else if (is_integral_positive)
			{
				u64 value = 0;
				for (auto i : integral)
				{
					value *= 10;
					value += i - '0';
				}
				return Variant(value);
			}
			else
			{
				i64 value = 0;
				for (auto i : integral)
				{
					value *= 10;
					value += i - '0';
				}
				return Variant(value);
			}
		}

		static R<Variant> read_value(IReadContext& ctx)
		{
			ctx.skip_whitespaces_and_comments();
			c32 ch = ctx.next_char();
			if (ch == '\0')
			{
				return set_error(BasicError::format_error(), "Unexpected EOF reached at at line %u, pos %u.", ctx.get_line(), ctx.get_pos());
			}
			else if (ch == '{')
			{
				return read_object(ctx);
			}
			else if (ch == '[')
			{
				return read_array(ctx);
			}
			else if (ch == '"')
			{
				return read_string_or_blob(ctx);
			}
			else if (ch == 't' && ctx.next_char(1) == 'r' && ctx.next_char(2) == 'u' && ctx.next_char(3) == 'e')
			{
				ctx.consume('t');
				ctx.consume('r');
				ctx.consume('u');
				ctx.consume('e');
				return Variant(true);
			}
			else if (ch == 'f' && ctx.next_char(1) == 'a' && ctx.next_char(2) == 'l' && ctx.next_char(3) == 's' && ctx.next_char(4) == 'e')
			{
				// Check false.
				ctx.consume('f');
				ctx.consume('a');
				ctx.consume('l');
				ctx.consume('s');
				ctx.consume('e');
				return Variant(false);
			}
			else if (ch == 'n' && ctx.next_char(1) == 'u' && ctx.next_char(2) == 'l' && ctx.next_char(3) == 'l')
			{
				// Check null.
				ctx.consume('n');
				ctx.consume('u');
				ctx.consume('l');
				ctx.consume('l');
				return Variant(VariantType::null);
			}
			else if (ch == '-' || ch == '0' || (ch >= '1' && ch <= '9'))
			{
				return read_number(ctx);
			}
			else
			{
				return set_error(BasicError::format_error(), "Unrecognized token: %c(0x%0x) at line %n, pos %n.", (c8)ch, (u32)ch, ctx.get_line(), ctx.get_pos());
			}
		}

		inline void write_indents(String& s, u32 num_indents)
		{
			for (u32 i = 0; i < num_indents; ++i)
			{
				s.push_back('\t');
			}
		}

		static void write_string_value(String& s, const Name& v)
		{
			s.push_back('"');

			const c8* cur = v.c_str();
			while (cur < (v.c_str() + strlen(v.c_str())))
			{
				c32 ch = utf8_decode_char(cur);
				switch (ch)
				{
				case '\"':
					s.append("\\\"");
					break;
				case '\\':
					s.append("\\\\");
					break;
				case '/':
					s.append("\\/");
					break;
				case '\b':
					s.append("\\\b");
					break;
				case '\f':
					s.append("\\\f");
					break;
				case '\n':
					s.append("\\\n");
					break;
				case '\r':
					s.append("\\\r");
					break;
				case '\t':
					s.append("\\\t");
					break;
				case '\a':
					s.append("\\\a");
					break;
				case '\v':
					s.append("\\\v");
					break;
				default:
					s.append(cur, utf8_charspan(ch));
					break;
				}
				cur += utf8_charspan(ch);
			}
			s.push_back('"');
		}

		static void write_blob_value(String& s, const void* data, usize data_size, usize data_alignment)
		{
			s.push_back('"');
			c8 buf[128];
			snprintf(buf, 128, "@blob@%llu@%llu@", (long long unsigned int)data_size, (long long unsigned int)data_alignment);
			s.append(buf);
			usize encoded_size = base64_get_encoded_size(data_size);
			usize offset = s.size();
			s.resize(s.size() + encoded_size, '0');
			base64_encode(s.data() + offset, encoded_size, data, data_size);
			s.push_back('"');
		}

		static void write_value(const Variant& v, String& s, bool indent, u32 base_indent)
		{
			switch (v.type())
			{
			case VariantType::null:
			case VariantType::pointer:
				s.append("null");
				break;
			case VariantType::object:
			{
				if (v.empty())
				{
					s.append("{}"); // prevent indent for empty object.
				}
				else
				{
					s.push_back('{');
					if (indent)
					{
						++base_indent;
						s.push_back('\n');
					}
					usize count = 0;
					for (auto& i : v.key_values())
					{
						if (indent)
						{
							write_indents(s, base_indent);
						}
						write_string_value(s, i.first);
						s.push_back(':');
						//if (indent && (i.second.type() == VariantType::array || i.second.type() == VariantType::object) && !i.second.empty())
						//{
						//	s.push_back('\n');
						//	write_indents(s, base_indent);
						//}
						if (indent)
						{
							s.push_back(' ');
						}
						write_value(i.second, s, indent, base_indent);
						if (count != v.size() - 1) s.push_back(',');
						if (indent)
						{
							s.push_back('\n');
						}
						++count;
					}
					if (indent)
					{
						--base_indent;
						write_indents(s, base_indent);
					}
					s.push_back('}');
				}
			}
			break;
			case VariantType::array:
			{
				if (v.empty())
				{
					s.append("[]");
				}
				else
				{
					s.push_back('[');
					//if (indent)
					//{
					//	++base_indent;
					//	s.push_back('\n');
					//}
					for (usize i = 0; i < v.size(); ++i)
					{
						/*if (indent)
						{
							write_indents(s, base_indent);
						}*/
						write_value(v[i], s, indent, base_indent);
						if (i != v.size() - 1) s.push_back(',');
						/*if (indent)
						{
							s.push_back('\n');
						}*/
					}
					//if (indent)
					//{
					//	--base_indent;
					//	write_indents(s, base_indent);
					//}
					s.push_back(']');
				}
			}
			break;
			case VariantType::number:
			{
				c8 buf[64];
				switch (v.number_type())
				{
				case VariantNumberType::number_f64:
					snprintf(buf, 64, "%f", v.fnum()); break;
				case VariantNumberType::number_i64:
					snprintf(buf, 64, "%lld", (long long int)v.inum()); break;
				case VariantNumberType::number_u64:
					snprintf(buf, 64, "%llu", (long long unsigned int)v.unum()); break;
                default: lupanic(); break;
				}
				s.append(buf);
			}
			break;
			case VariantType::string:
				write_string_value(s, v.str());
				break;
			case VariantType::boolean:
				s.append(v.boolean() ? "true" : "false");
				break;
			case VariantType::blob:
				write_blob_value(s, v.blob_data(), v.blob_size(), v.blob_alignment());
				break;
			}
		}
	}

	LUNA_RUNTIME_API R<Variant> json_read(const c8* src, usize src_size)
	{
		lucheck(src);
		JSON::BufferReadContext ctx;
		ctx.src = src;
		ctx.cur = src;
		ctx.src_length = src_size;
		ctx.line = 1;
		ctx.pos = 1;
		return read_value(ctx);
	}
	LUNA_RUNTIME_API R<Variant> json_read(IStream* stream)
	{
		lucheck(stream);
		JSON::StreamReadContext ctx;
		ctx.stream = stream;
		ctx.line = 1;
		ctx.pos = 1;
		return read_value(ctx);
	}

	LUNA_RUNTIME_API String json_write(const Variant& v, bool indent)
	{
		String r;
		JSON::write_value(v, r, indent, 0);
		return r;
	}

	LUNA_RUNTIME_API RV json_write(IStream* stream, const Variant& v, bool indent)
	{
		String data = json_write(v, indent);
		return stream->write(data.data(), data.size());
	}
}
