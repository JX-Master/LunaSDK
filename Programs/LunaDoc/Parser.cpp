/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Parser.cpp
* @author JXMaster
* @date 2023/10/24
*/
#include "Parser.hpp"
#include <Luna/VariantUtils/XML.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Log.hpp>
using namespace Luna::VariantUtils;

Name _doxygen;
Name _compounddef;
Name _compoundname;
Name _name;
Name _title;
Name _sectiondef;
Name _briefdescription;
Name _detaileddescription;
Name _kind;
Name _func;
Name _memberdef;
Name _id;
Name _function;
Name _prot;
Name _static;
Name _constexpr;
Name _const;
Name _explicit;
Name _ninline;
Name _virt;
Name _no;
Name _yes;
Name _type;
Name _definition;
Name _argsstring;
Name _qualifiedname;
Name _param;
Name _declname;

RV encode_markdown_text(String& out_text, const Variant& content)
{
    Name text = content.at(0).str();
    // Skip blank string.
    {
        bool is_blank = true;
        for(usize i = 0; i < text.size(); ++i)
        {
            if(!isspace(text.c_str()[i]))
            {
                is_blank = false;
                break;
            }
        }
        if(is_blank) return ok;
    }
    out_text.append(text.c_str());
    return ok;
}

static R<Variant> parse_func_section(const Variant& section)
{
    Variant r(VariantType::array);
    lutry
    {
        auto& content = get_xml_content(section);
        for(auto& c : content.values())
        {
            if(get_xml_name(c) != _memberdef) continue;
            Variant member;
            auto& attributes = get_xml_attributes(c);
            if(attributes[_kind].str() != _function) continue;
            member[_id] = attributes[_id];
            member[_prot] = attributes[_prot].str() == _yes ? true : false;
            member[_static] = attributes[_static].str() == _yes ? true : false;
            member[_constexpr] = attributes[_constexpr].str() == _yes ? true : false;
            member[_const] = attributes[_const].str() == _yes ? true : false;
            member[_explicit] = attributes[_explicit].str() == _yes ? true : false;
            member[_ninline] = attributes[_ninline].str() == _yes ? true : false;
            member[_virt] = attributes[_virt].str() == _yes ? true : false;
            member[_prot] = attributes[_prot].str() == _yes ? true : false;
            auto& func_members = get_xml_content(c);
            for(auto& m : func_members.values())
            {
                auto member_name = get_xml_name(m);
                if(member_name == _type)
                {
                    member[_type] = get_xml_content(m).at(0).str();
                }
                else if(member_name == _definition)
                {
                    member[_definition] = get_xml_content(m).at(0).str();
                }
                else if(member_name == _argsstring)
                {
                    member[_argsstring] = get_xml_content(m).at(0).str();
                }
                else if(member_name == _name)
                {
                    member[_name] = get_xml_content(m).at(0).str();
                }
                else if(member_name == _qualifiedname)
                {
                    member[_qualifiedname] = get_xml_content(m).at(0).str();
                }
                else if(member_name == _param)
                {
                    Variant param(VariantType::object);
                    auto& param_content = get_xml_content(m);
                    for(auto& param_m : param_content.values())
                    {
                        auto param_m_name = get_xml_name(param_m);
                        if(param_m_name == _type)
                        {
                            param[_type] = get_xml_content(param_m).at(0).str();
                        }
                        else if(param_m_name == _declname)
                        {
                            param[_declname] = get_xml_content(param_m).at(0).str();
                        }
                    }
                    member[_param].push_back(move(param));
                }
                else if(member_name == _briefdescription)
                {
                    member[_briefdescription] = get_xml_content(m).at(0).str();
                }
            }
            r.push_back(move(member));
        }
    }
    lucatchret;
    return r;
}

static R<Variant> parse_group(const Variant& group_content)
{
    Variant r;
    lutry
    {
        for(auto& c : group_content.values())
        {
            if(c.type() != VariantType::object) continue;
            auto name = get_xml_name(c);
            if(name == _compoundname)
            {
                auto name = get_xml_content(c).at(0).str();
                if(!name) return set_error(BasicError::format_error(), "<compoundname> not found for group <compounddef>");
                r[_name] = name;
            }
            else if(name == _title)
            {
                String text;
                luexp(encode_markdown_text(text, get_xml_content(c)));
                if(!text.empty()) r[_title] = text;
            }
            else if(name == _briefdescription)
            {
                String text;
                luexp(encode_markdown_text(text, get_xml_content(c)));
                if(!text.empty()) r[_briefdescription] = text;
            }
            else if(name == _detaileddescription)
            {
                String text;
                luexp(encode_markdown_text(text, get_xml_content(c)));
                if(!text.empty()) r[_detaileddescription] = text;
            }
            else if(name == _sectiondef)
            {
                auto& kind = get_xml_attributes(c)[_kind].str();
                if(kind == _func)
                {
                    lulet(sec, parse_func_section(c));
                    r[_func] = move(sec);
                }
            }
        }
    }
    lucatchret;
    return r;
}

R<Vector<Variant>> parse_groups(Span<const Variant> group_files)
{
    Vector<Variant> ret;
    lutry
    {
        // Generate group objects.
        HashMap<Name, Variant> groups;
        for(auto& f : group_files)
        {
            if(get_xml_name(f) != _doxygen) return set_error(BasicError::format_error(), "One doxygen XML file must begin with <doxygen>, got %s", get_xml_name(f).c_str());
            auto& compounddef = find_first_xml_child_element(f, _compounddef);
            if(compounddef.type() != VariantType::object) return set_error(BasicError::format_error(), "<compounddef> not found");
            lulet(g, parse_group(get_xml_content(compounddef)));
            groups.insert(make_pair(g[_name].c_str(), g));
        }
        for(auto& g : groups)
        {
            ret.push_back(move(g.second));
        }
    }
    lucatchret;
    return ret;
}

RV write_group_to_file(const Variant& group, const c8* filepath)
{
    lutry
    {
        lulet(f, open_file(filepath, FileOpenFlag::write | FileOpenFlag::user_buffering, FileCreationMode::create_always));
        // Write title.
        {
            Name title = group[_title].str();
            if(title)
            {
                luexp(f->write("# ", 2));
                luexp(f->write(title.c_str(), title.size()));
                luexp(f->write("\n", 1));
            }
        }
        // Write brief description.
        {
            Name text = group[_briefdescription].str();
            if(text)
            {
                luexp(f->write(text.c_str(), text.size()));
                luexp(f->write("\n", 1));
            }
        }
        // Write detailed description.
        {
            Name text = group[_detaileddescription].str();
            if(text)
            {
                luexp(f->write(text.c_str(), text.size()));
                luexp(f->write("\n", 1));
            }
        }
        // Write function sections.
        {
            auto& funcs = group[_func];
            if(funcs.valid())
            {
                luexp(f->write("## Functions\n", 13));
                for(auto& fn : funcs.values())
                {
                    auto qualifiedname = fn[_qualifiedname].str();
                    luexp(f->write("### ", 4));
                    luexp(f->write(qualifiedname.c_str(), qualifiedname.size()));
                    luexp(f->write("\n", 1));
                    luexp(f->write("```\n", 4));
                    auto definition = fn[_definition].str();
                    luexp(f->write(definition.c_str(), definition.size()));
                    auto argsstring = fn[_argsstring].str();
                    luexp(f->write(argsstring.c_str(), argsstring.size()));
                    luexp(f->write("\n```\n", 5));
                }
            }
        }
    }
    lucatchret;
    return ok;
}