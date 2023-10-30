/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file XML.cpp
* @author JXMaster
* @date 2023/10/12
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VARIANT_UTILS_API LUNA_EXPORT
#include "../XML.hpp"
#include "StringParser.hpp"

namespace Luna
{
    namespace VariantUtils
    {
        Name _name;
        Name _attributes;
        Name _content;
        void xml_init()
        {
            _name = "name";
            _attributes = "attributes";
            _content = "content";
        }
        void xml_close()
        {
            _name.reset();
            _attributes.reset();
            _content.reset();
        }
        LUNA_VARIANT_UTILS_API Variant new_xml_element(const Name& name)
        {
            Variant element(VariantType::object);
            element[_name] = name;
            element[_attributes] = Variant(VariantType::object);
            element[_content] = Variant(VariantType::array);
            return element;
        }
        LUNA_VARIANT_UTILS_API Name get_xml_name(const Variant& xml_element)
        {
            return xml_element[_name].str();
        }
        LUNA_VARIANT_UTILS_API void set_xml_name(Variant& xml_element, const Name& name)
        {
            xml_element[_name] = name;
        }
        LUNA_VARIANT_UTILS_API const Variant& get_xml_attributes(const Variant& xml_element)
        {
            return xml_element[_attributes];
        }
		LUNA_VARIANT_UTILS_API Variant& get_xml_attributes(Variant& xml_element)
        {
            return xml_element[_attributes];
        }
        LUNA_VARIANT_UTILS_API const Variant& get_xml_content(const Variant& xml_element)
        {
            return xml_element[_content];
        }
        LUNA_VARIANT_UTILS_API Variant& get_xml_content(Variant& xml_element)
        {
            return xml_element[_content];
        }
        LUNA_VARIANT_UTILS_API const Variant& find_first_xml_child_element(const Variant& xml_element, const Name& name, usize start_index, usize* out_index)
        {
            auto& content = get_xml_content(xml_element);
            for(usize i = start_index; i < content.size(); ++i)
            {
                if(content[i].type() == VariantType::object && get_xml_name(content[i]) == name)
                {
                    if(out_index) *out_index = i;
                    return content[i];
                }
            }
            if(out_index) *out_index = USIZE_MAX;
            return Variant::npos();
        }
        static void skip_single_line_comment(IReadContext& ctx)
        {
            lucheck(ctx.next_char() == '<' && ctx.next_char(1) == '!' && ctx.next_char(2) == '-' && ctx.next_char(3) == '-');
            ctx.consume('<');
			ctx.consume('!');
            ctx.consume('-');
            ctx.consume('-');
            c32 ch = ctx.next_char();
            while(ch)
            {
                if(ch == '-')
                {
                    c32 chs[2];
                    chs[0] = ctx.next_char(1);
                    chs[1] = ctx.next_char(2);
                    if(chs[0] == '-' && chs[1] == '>')
                    {
                        ctx.consume('-');
                        ctx.consume('-');
                        ctx.consume('>');
                        return;
                    }
                }
                ctx.consume(ch);
                ch = ctx.next_char();
            }
        }
        static void skip_whitespaces_and_comments(IReadContext& ctx)
        {
            c32 ch = ctx.next_char();
			while (ch)
			{
				if (is_whitespace(ch))
				{
					ctx.consume(ch);
				}
				else if (ch == '<')
				{
					c32 chs[3];
                    chs[0] = ctx.next_char(1);
                    chs[1] = ctx.next_char(2);
                    chs[2] = ctx.next_char(3);
                    if(chs[0] == '!' && chs[1] == '-' && chs[2] == '-')
                    {
                        skip_single_line_comment(ctx);
                    }
					else
					{
						break;
					}
				}
				else break;
				ch = ctx.next_char();
			}
        }
        static void skip_comments(IReadContext& ctx)
        {
            c32 ch = ctx.next_char();
			while (ch)
			{
				if (ch == '<')
				{
					c32 chs[3];
                    chs[0] = ctx.next_char(1);
                    chs[1] = ctx.next_char(2);
                    chs[2] = ctx.next_char(3);
                    if(chs[0] == '!' && chs[1] == '-' && chs[2] == '-')
                    {
                        skip_single_line_comment(ctx);
                    }
					else
					{
						break;
					}
				}
				else break;
				ch = ctx.next_char();
			}
        }
        static RV skip_xml_header(IReadContext& ctx)
        {
            // <?xml version="1.0" encoding="UTF-8"?>
            skip_whitespaces_and_comments(ctx);
            c32 chs[5];
            chs[0] = ctx.next_char(0);
            chs[1] = ctx.next_char(1);
            chs[2] = ctx.next_char(2);
            chs[3] = ctx.next_char(3);
            chs[4] = ctx.next_char(4);
            if(!(
                chs[0] == '<' &&
                chs[1] == '?' &&
                (chs[2] == 'x' || chs[2] == 'X') &&
                (chs[3] == 'm' || chs[3] == 'M') &&
                (chs[4] == 'l' || chs[4] == 'L')
            ))
            {
                return set_error(BasicError::format_error(), "'<?xml' or '<?XML' expected at the beginning of the document (line %d pos %d).", ctx.get_line(), ctx.get_pos());
            }
            ctx.consume(chs[0]);
            ctx.consume(chs[1]);
            ctx.consume(chs[2]);
            ctx.consume(chs[3]);
            ctx.consume(chs[4]);
            skip_whitespaces_and_comments(ctx);
            // Currently we don't handle xml header data.
            chs[0] = ctx.next_char(0);
            while(chs[0])
            {
                if(chs[0] == '?')
                {
                    chs[1] = ctx.next_char(1);
                    if(chs[1] == '>')
                    {
                        ctx.consume(chs[0]);
                        ctx.consume(chs[1]);
                        return ok;
                    }
                }
                ctx.consume(chs[0]);
                chs[0] = ctx.next_char(0);
            }
            return set_error(BasicError::format_error(), "Unexpected EOF occurred at line %d, pos %d.", ctx.get_line(), ctx.get_pos());
        }
        inline bool is_name_start_char(u32 ch)
        {
            return ch == ':' || 
                (ch >= 'A' && ch <= 'Z') ||
                ch == '_' ||
                (ch >= 'a' && ch <= 'z') ||
                (ch >= 0xC0 && ch <= 0xD6) ||
                (ch >= 0xD8 && ch <= 0xF6) ||
                (ch >= 0xF8 && ch <= 0x2FF) ||
                (ch >= 0x370 && ch <= 0x37D) ||
                (ch >= 0x37F && ch <= 0x1FFF) ||
                (ch >= 0x200C && ch <= 0x200D) ||
                (ch >= 0x2070 && ch <= 0x218F) ||
                (ch >= 0x2C00 && ch <= 0x2FEF) ||
                (ch >= 0x3001 && ch <= 0xD7FF) ||
                (ch >= 0xF900 && ch <= 0xFDCF) ||
                (ch >= 0xFDF0 && ch <= 0xFFFD) ||
                (ch >= 0x10000 && ch <= 0xEFFFF);
        }
        inline bool is_name_char(u32 ch)
        {
            return is_name_start_char(ch) ||
                ch == '-' ||
                ch == '.' ||
                (ch >= '0' && ch <= '9') ||
                ch == 0xB7 ||
                (ch >= 0x0300 && ch <= 0x036F) ||
                (ch >= 0x203F && ch <= 0x2040);
        }
        static void read_xml_name(IReadContext& ctx, String& dst)
        {
            c32 ch = ctx.next_char();
            if(!is_name_start_char((u32)ch))
            {
                return;
            }
            c8 utf8_buf[8];
            usize size = utf8_encode_char(utf8_buf, ch);
            dst.append(utf8_buf, size);
            ctx.consume(ch);
            ch = ctx.next_char();
            while(is_name_char(ch))
            {
                size = utf8_encode_char(utf8_buf, ch);
                dst.append(utf8_buf, size);
                ctx.consume(ch);
                ch = ctx.next_char();
            }
        }
        inline bool is_hex(c32 ch)
        {
            u32 ch_num = (u32)ch;
            return (ch_num >= '0' && ch_num <= '9') || (ch_num >= 'a' && ch_num <= 'f') || (ch_num >= 'A' && ch_num <= 'F'); 
        }
        inline u8 atohex(c32 ch)
		{
            u32 ch_num = (u32)ch;
			return ch_num >= '0' && ch_num <= '9' ? ch_num - '0' :
				ch_num >= 'a' && ch_num <= 'f' ? ch_num - 'a' + 10 :
				ch_num >= 'A' && ch_num <= 'F' ? ch_num - 'A' + 10 : 0;
		}
        static RV read_reference(IReadContext& ctx, String& s)
        {
            c32 ch = ctx.next_char();
            lucheck(ch == '&');
            ctx.consume(ch);
            ch = ctx.next_char();
            if(ch == '#')
            {
                // character reference.
                ctx.consume(ch);
                ch = ctx.next_char();
                u32 read_ch;
                if(ch == 'x')
                {
                    // hex
                    ctx.consume(ch);
                    ch = ctx.next_char();
                    if(!is_hex(ch))
                    {
                        return set_error(BasicError::format_error(), "Unexpected character. (line %d pos %d).", ctx.get_line(), ctx.get_pos());
                    }
                    read_ch = atohex(ch);
                    ctx.consume(ch);
                    ch = ctx.next_char();
                    while(ch != ';')
                    {
                        if(!is_hex(ch))
                        {
                            return set_error(BasicError::format_error(), "Unexpected character. (line %d pos %d).", ctx.get_line(), ctx.get_pos());
                        }
                        read_ch = read_ch << 4 + atohex(ch);
                        ctx.consume(ch);
                        ch = ctx.next_char();
                    }
                }
                else
                {
                    // decimal
                    if((u32)ch < '0' || (u32)ch > '9')
                    {
                        return set_error(BasicError::format_error(), "Unexpected character. (line %d pos %d).", ctx.get_line(), ctx.get_pos());
                    }
                    read_ch = (u32)ch - '0';
                    ctx.consume(ch);
                    ch = ctx.next_char();
                    while(ch != ';')
                    {
                        if((u32)ch < '0' || (u32)ch > '9')
                        {
                            return set_error(BasicError::format_error(), "Unexpected character. (line %d pos %d).", ctx.get_line(), ctx.get_pos());
                        }
                        read_ch = read_ch * 10 + ((u32)ch - '0');
                        ctx.consume(ch);
                        ch = ctx.next_char();
                    }
                }
                ctx.consume(ch); // ;
                c8 buf[6];
                usize char_size = utf8_encode_char(buf, (c32)read_ch);
                s.append(buf, char_size);
            }
            else
            {
                // entity reference.
                c32 buf[5];
                buf[0] = ctx.next_char(0);
                buf[1] = ctx.next_char(1);
                buf[2] = ctx.next_char(2);
                buf[3] = ctx.next_char(3);
                buf[4] = ctx.next_char(4);
                if(buf[0] == 'a' && buf[1] == 'm' && buf[2] == 'p' && buf[3] == ';')
                {
                    s.push_back('&');
                    ctx.consume(buf[0]);
                    ctx.consume(buf[1]);
                    ctx.consume(buf[2]);
                    ctx.consume(buf[3]);
                }
                else if(buf[0] == 'l' && buf[1] == 't' && buf[2] == ';')
                {
                    s.push_back('<');
                    ctx.consume(buf[0]);
                    ctx.consume(buf[1]);
                    ctx.consume(buf[2]);
                }
                else if(buf[0] == 'g' && buf[1] == 't' && buf[2] == ';')
                {
                    s.push_back('>');
                    ctx.consume(buf[0]);
                    ctx.consume(buf[1]);
                    ctx.consume(buf[2]);
                }
                else if(buf[0] == 'a' && buf[1] == 'p' && buf[2] == 'o' && buf[3] == 's' && buf[4] == ';')
                {
                    s.push_back('\'');
                    ctx.consume(buf[0]);
                    ctx.consume(buf[1]);
                    ctx.consume(buf[2]);
                    ctx.consume(buf[3]);
                    ctx.consume(buf[4]);
                }
                else if(buf[0] == 'q' && buf[1] == 'u' && buf[2] == 'o' && buf[3] == 't' && buf[4] == ';')
                {
                    s.push_back('"');
                    ctx.consume(buf[0]);
                    ctx.consume(buf[1]);
                    ctx.consume(buf[2]);
                    ctx.consume(buf[3]);
                    ctx.consume(buf[4]);
                }
                else
                {
                    // does not match anything, proceed as normal strings.
                    s.push_back('&');
                }
            }
            return ok;
        }
        static R<String> read_xml_string_literal(IReadContext& ctx)
		{
			String s;
            lutry
            {
                c32 ch = ctx.next_char();
                lucheck(ch == '"' || ch == '\'');
                ctx.consume(ch);
                ch = ctx.next_char();
                while (ch)
                {
                    if(ch == '&')
                    {
                        luexp(read_reference(ctx, s));
                        ch = ctx.next_char();
                    }
                    if (ch == '"' || ch == '\'')
                    {
                        ctx.consume(ch);
                        break;
                    }
                    c8 buf[6];
                    usize buf_count = utf8_encode_char(buf, ch);
                    s.append(buf, buf_count);
                    ctx.consume(ch);
                    ch = ctx.next_char();
                }
            }
            lucatchret;
			return s;
		}
        static R<String> read_xml_character_data(IReadContext& ctx)
        {
            String s;
            lutry
            {
                c32 ch = ctx.next_char();
                while (ch)
                {
                    if(ch == '<')
                    {
                        skip_comments(ctx);
                        ch = ctx.next_char();
                        if(ch == '<')
                        {
                            break;
                        }
                    }
                    if(ch == '&')
                    {
                        luexp(read_reference(ctx, s));
                    }
                    else
                    {
                        c8 buf[6];
                        usize buf_count = utf8_encode_char(buf, ch);
                        s.append(buf, buf_count);
                        ctx.consume(ch);
                    }
                    ch = ctx.next_char();
                }
            }
            lucatchret;
            return s;
        }
        static R<Variant> read_xml_attribute(IReadContext& ctx, Name& attribute_name)
        {
            Variant ret;
            lutry
            {
                String name;
                read_xml_name(ctx, name);
                if(name.empty()) return set_error(BasicError::format_error(), "Valid name character expected. (line %d pos %d).", ctx.get_line(), ctx.get_pos());
                attribute_name = name;
                skip_whitespaces_and_comments(ctx);
                c32 ch = ctx.next_char();
                if(ch != '=') return set_error(BasicError::format_error(), "'=' expected. (line %d pos %d).", ctx.get_line(), ctx.get_pos());
                ctx.consume(ch);
                skip_whitespaces_and_comments(ctx);
                ch = ctx.next_char();
                if(ch != '"' && ch != '\'') return set_error(BasicError::format_error(), "'\"' expected. (line %d pos %d).", ctx.get_line(), ctx.get_pos());
                lulet(data, read_xml_string_literal(ctx));
                ret = Name(data);
            }
            lucatchret;
            return ret;
        }
        static RV read_xml_start_tag(IReadContext& ctx, Variant& element, Name& element_name, bool& empty_tag)
        {
            lutry
            {
                skip_whitespaces_and_comments(ctx);
                c32 ch = ctx.next_char();
                if(ch != '<')
                {
                    return set_error(BasicError::format_error(), "'<' expected at the beginning of one element (line %d pos %d).", ctx.get_line(), ctx.get_pos());
                }
                ctx.consume(ch);
                String name;
                read_xml_name(ctx, name);
                if(name.empty()) return set_error(BasicError::format_error(), "Valid name character expected. (line %d pos %d).", ctx.get_line(), ctx.get_pos());
                element_name = name;
                element = new_xml_element(element_name);
                skip_whitespaces_and_comments(ctx);
                ch = ctx.next_char();
                c32 ch2 = ctx.next_char(1);
                Variant& attributes = get_xml_attributes(element);
                while(ch != '>' && !(ch == '/' && ch2 == '>'))
                {
                    Name attribute_name;
                    lulet(attribute, read_xml_attribute(ctx, attribute_name));
                    attributes[attribute_name] = move(attribute);
                    skip_whitespaces_and_comments(ctx);
                    ch = ctx.next_char();
                    ch2 = ctx.next_char(1);
                }
                if(ch == '>')
                {
                    empty_tag = false;
                    ctx.consume(ch);
                }
                else
                {
                    empty_tag = true;
                    ctx.consume(ch);
                    ctx.consume(ch2);
                }
            }
            lucatchret;
            return ok;
        }
        static R<String> read_xml_cdata(IReadContext& ctx)
        {
            c32 ch[9];
            ch[0] = ctx.next_char(0);
            ch[1] = ctx.next_char(1);
            ch[2] = ctx.next_char(2);
            ch[3] = ctx.next_char(3);
            ch[4] = ctx.next_char(4);
            ch[5] = ctx.next_char(5);
            ch[6] = ctx.next_char(6);
            ch[7] = ctx.next_char(7);
            ch[8] = ctx.next_char(8);
            luassert(
                ch[0] == '<' &&
                ch[1] == '!' &&
                ch[2] == '[' &&
                ch[3] == 'C' &&
                ch[4] == 'D' &&
                ch[5] == 'A' &&
                ch[6] == 'T' &&
                ch[7] == 'A' &&
                ch[8] == '[');
            ctx.consume(ch[0]);
            ctx.consume(ch[1]);
            ctx.consume(ch[2]);
            ctx.consume(ch[3]);
            ctx.consume(ch[4]);
            ctx.consume(ch[5]);
            ctx.consume(ch[6]);
            ctx.consume(ch[7]);
            ctx.consume(ch[8]);
            ch[0] = ctx.next_char(0);
            String r;
            while (true)
            {
                if(!ch[0]) return set_error(BasicError::format_error(), "Unexpected EOF occurred at line %d, pos %d.", ctx.get_line(), ctx.get_pos());
                if (ch[0] == ']')
                {
                    ch[1] = ctx.next_char(1);
                    ch[2] = ctx.next_char(2);
                    if (ch[1] == ']' && ch[2] == '>')
                    {
                        ctx.consume(ch[0]);
                        ctx.consume(ch[1]);
                        ctx.consume(ch[2]);
                        break;
                    }
                }
                c8 buf[6];
                usize buf_sz = utf8_encode_char(buf, ch[0]);
                r.append(buf, buf_sz);
                ctx.consume(ch[0]);
                ch[0] = ctx.next_char(0);
            }
            return r;
        }
        static R<Variant> read_xml_element(IReadContext& ctx);
        static RV read_xml_content(IReadContext& ctx, Variant& element)
        {
            lutry
            {
                Variant& content = get_xml_content(element);
                c32 ch = ctx.next_char();
                while(true)
                {
                    if(!ch) return set_error(BasicError::format_error(), "Unexpected EOF occurred at line %d, pos %d.", ctx.get_line(), ctx.get_pos());
                    if(ch == '<')
                    {
                        skip_comments(ctx);
                        ch = ctx.next_char();
                    }
                    if(ch == '<')
                    {
                        c32 ch2 = ctx.next_char(1);
                        if(ch2 == '/')
                        {
                            // end tag.
                            break;
                        }
                        else if (ch2 == '!' && 
                            ctx.next_char(2) == '[' &&
                            ctx.next_char(3) == 'C'&&
                            ctx.next_char(4) == 'D'&&
                            ctx.next_char(5) == 'A'&&
                            ctx.next_char(6) == 'T'&&
                            ctx.next_char(7) == 'A'&&
                            ctx.next_char(8) == '[')
                        {
                            // read CDATA.
                            lulet(cdata, read_xml_cdata(ctx));
                            content.push_back(Variant(Name(cdata)));
                        }
                        else
                        {
                            lulet(child, read_xml_element(ctx));
                            content.push_back(move(child));
                        }
                    }
                    else
                    {
                        lulet(chardata, read_xml_character_data(ctx));
                        bool is_blank = false;
                        // Discard indent strings (strings begin with '\n' and contain whitespaces only).
                        if(!chardata.empty() && (chardata[0] == '\n' || chardata[0] == '\r'))
                        {
                            is_blank = true;
                            for(auto& c : chardata)
                            {
                                if(!is_whitespace(c))
                                {
                                    is_blank = false;
                                    break;
                                }
                            }
                        }
                        if(!is_blank)
                        {
                            content.push_back(Variant(Name(chardata)));
                        }
                    }
                    ch = ctx.next_char();
                }
            }
            lucatchret;
            return ok;
        }
        static RV read_xml_end_tag(IReadContext& ctx, const Name& element_name)
        {
            c32 ch = ctx.next_char();
            c32 ch2 = ctx.next_char(1);
            luassert(ch == '<' && ch2 == '/');
            ctx.consume(ch);
            ctx.consume(ch2);
            String name;
            read_xml_name(ctx, name);
            if(Name(name) != element_name) 
                return set_error(BasicError::format_error(), "The name of the end tag (%s) does not match the name of the start tag (%s). (line %d pos %d)", name.c_str(), element_name.c_str(), ctx.get_line(), ctx.get_pos());
            skip_whitespaces_and_comments(ctx);
            ch = ctx.next_char();
            if(ch != '>') return set_error(BasicError::format_error(), "'>' expected at the end of one one tag (line %d pos %d).", ctx.get_line(), ctx.get_pos());
            ctx.consume(ch);
            return ok;
        }
        static R<Variant> read_xml_element(IReadContext& ctx)
        {
            Variant ret;
            lutry
            {
                Name element_name;
                bool empty_tag;
                luexp(read_xml_start_tag(ctx, ret, element_name, empty_tag));
                if(!empty_tag)
                {
                    luexp(read_xml_content(ctx, ret));
                    luexp(read_xml_end_tag(ctx, element_name));
                }
            }
            lucatchret;
            return ret;
        }
        static R<Variant> read_xml_document(IReadContext& ctx)
        {
            lutry
            {
                luexp(skip_xml_header(ctx));
            }
            lucatchret;
            return read_xml_element(ctx);
        }
        LUNA_VARIANT_UTILS_API R<Variant> read_xml(const void* src, usize src_size)
        {
            lucheck(src);
			BufferReadContext ctx;
            ctx.src = src;
			ctx.cur = src;
			ctx.src_size = src_size;
			ctx.line = 1;
			ctx.pos = 1;
            ctx.encoding = Encoding::utf_8;
            ctx.skip_utf16_bom();
            return read_xml_document(ctx);
        }
		LUNA_VARIANT_UTILS_API R<Variant> read_xml(IStream* stream)
        {
            lucheck(stream);
			StreamReadContext ctx;
			ctx.stream = stream;
			ctx.line = 1;
			ctx.pos = 1;
            ctx.skip_utf16_bom();
			return read_xml_document(ctx);
        }
        inline void write_indents(String& s, u32 num_indents)
		{
			for (u32 i = 0; i < num_indents; ++i)
			{
				s.push_back('\t');
			}
		}
        void write_xml_string(String& dst, const Name& str)
        {
            const c8* cur = str.c_str();
			const c8* end = str.c_str() + str.size();
			while (cur < end)
			{
				c32 ch = utf8_decode_char(cur);
				switch (ch)
				{
				case '<':
                    dst.append("&lt;");
                    break;
                case '>':
                    dst.append("&gt;");
                    break;
                case '&':
                    dst.append("&amp;");
                    break;
                case '"':
                    dst.append("&quot;");
                    break;
                case '\'':
                    dst.append("&apos;");
                    break;
                case '\n':
                    dst.append("&#10;");
                    break;
                case '\r':
                    dst.append("&#13;");
                    break;
                case '\t':
                    dst.append("&#9;");
                    break;
				default:
					dst.append(cur, utf8_charspan(ch));
					break;
				}
				cur += utf8_charspan(ch);
			}
        }
        void write_xml_element(const Variant& v, String& s, bool indent, u32 base_indent)
        {
            auto name = get_xml_name(v);
            // start tag.
            s.push_back('<');
            s.append(name.c_str());
            auto& attributes = get_xml_attributes(v);
            for(auto& attr : attributes.key_values())
            {
                s.push_back(' ');
                s.append(attr.first.c_str());
                s.push_back('=');
                s.push_back('"');
                write_xml_string(s, attr.second.str());
                s.push_back('"');
            }
            s.push_back('>');
            auto& content = get_xml_content(v);
            if(!content.empty())
            {
                bool single_chardata_content = content.size() == 1 && content.at(0).type() == VariantType::string;
                if(indent && !single_chardata_content)
                {
                    ++base_indent;
                    s.push_back('\n');
                }
                for(auto& child : content.values())
                {
                    if (indent && !single_chardata_content)
					{
						write_indents(s, base_indent);
					}
                    if(child.type() == VariantType::object)
                    {
                        write_xml_element(child, s, indent, base_indent);
                    }
                    else if(child.type() == VariantType::string)
                    {
                        write_xml_string(s, child.c_str());
                    }
                    if (indent && !single_chardata_content)
					{
						s.push_back('\n');
					}
                }
                if (indent && !single_chardata_content)
				{
					--base_indent;
					write_indents(s, base_indent);
				}
            }
            // end tag.
            s.push_back('<');
            s.push_back('/');
            s.append(name.c_str());
            s.push_back('>');
        }
        LUNA_VARIANT_UTILS_API String write_xml(const Variant& v, bool indent)
        {
            String r("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
            if(indent) r.push_back('\n');
            write_xml_element(v, r, indent, 0);
            return r;
        }
		LUNA_VARIANT_UTILS_API RV write_xml(IStream* stream, const Variant& v, bool indent)
        {
            String data = write_xml(v, indent);
			return stream->write(data.data(), data.size());
        }
    }
}