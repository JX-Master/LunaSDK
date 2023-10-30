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
Name _para;
Name _parameterlist;
Name _simplesect;
Name _return;
Name _parameternamelist;
Name _parameterdescription;
Name _parametername;
Name _parameteritem;
Name _computeroutput;

static void encode_markdown_text(String& out_text, const Variant& element);

static void encode_parameter_list(String& out_text, const Variant& parameterlist)
{
    out_text.append("\n#### Parameters\n");
    auto& items = get_xml_content(parameterlist);
    for(auto& item : items.values())
    {
        if(get_xml_name(item) != _parameteritem) continue;
        auto& item_elements = get_xml_content(item);
        for(auto& e : item_elements.values())
        {
            auto element_name = get_xml_name(e);
            if(element_name == _parameternamelist)
            {
                auto& parameter_name = get_xml_content(e).at(0);
                if(get_xml_name(parameter_name) == _parametername)
                {
                    out_text.append("##### ");
                    out_text.append(get_xml_content(parameter_name).at(0).str().c_str());
                    out_text.push_back('\n');
                }
            }
            else if(element_name == _parameterdescription)
            {
                encode_markdown_text(out_text, e);
            }
        }
    }
}

static void encode_markdown_text(String& out_text, const Variant& element)
{
    auto& content = get_xml_content(element);
    for(auto& c : content.values())
    {
        if(c.type() == VariantType::string)
        {
            Name text = c.str();
            bool is_blank = true;
            for(usize i = 0; i < text.size(); ++i)
            {
                if(!isspace(text.c_str()[i]))
                {
                    is_blank = false;
                    break;
                }
            }
            if(!is_blank) out_text.append(text.c_str());
        }
        else
        {
            auto name = get_xml_name(c);
            if(name == _para)
            {
                encode_markdown_text(out_text, c);
                out_text.append("\n\n");
            }
            else if(name == _parameterlist)
            {
                encode_parameter_list(out_text, c);
            }
            else if(name == _simplesect)
            {
                auto& attributes = get_xml_attributes(c);
                if(attributes[_kind].str() == _return)
                {
                    out_text.append("#### Return value\n");
                    encode_markdown_text(out_text, c);
                }
            }
            else if(name == _computeroutput)
            {
                out_text.push_back('`');
                encode_markdown_text(out_text, c);
                out_text.push_back('`');
            }
        }
    }
}

static void parse_func_section(String& out_group_content, const Variant& section)
{
    out_group_content.append("## Functions\n");
    auto& content = get_xml_content(section);
    for(auto& c : content.values())
    {
        if(get_xml_name(c) != _memberdef) continue;
        auto& attributes = get_xml_attributes(c);
        if(attributes[_kind].str() != _function) continue;
        auto& func_members = get_xml_content(c);
        Name qualifiedname;
        Name definition;
        Name argsstring;
        String briefdescription;
        String detaileddescription;
        for(auto& m : func_members.values())
        {
            auto member_name = get_xml_name(m);
            if(member_name == _type)
            {
                //member[_type] = get_xml_content(m).at(0).str();
            }
            else if(member_name == _definition)
            {
                definition = get_xml_content(m).at(0).str();
            }
            else if(member_name == _argsstring)
            {
                argsstring = get_xml_content(m).at(0).str();
            }
            else if(member_name == _name)
            {
                //member[_name] = get_xml_content(m).at(0).str();
            }
            else if(member_name == _qualifiedname)
            {
                qualifiedname = get_xml_content(m).at(0).str();
            }
            else if(member_name == _param)
            {
                // Variant param(VariantType::object);
                // auto& param_content = get_xml_content(m);
                // for(auto& param_m : param_content.values())
                // {
                //     auto param_m_name = get_xml_name(param_m);
                //     if(param_m_name == _type)
                //     {
                //         param[_type] = get_xml_content(param_m).at(0).str();
                //     }
                //     else if(param_m_name == _declname)
                //     {
                //         param[_declname] = get_xml_content(param_m).at(0).str();
                //     }
                // }
                // member[_param].push_back(move(param));
            }
            else if(member_name == _briefdescription)
            {
                encode_markdown_text(briefdescription, m);
            }
            else if(member_name == _detaileddescription)
            {
                String bd;
                encode_markdown_text(detaileddescription, m);
            }
        }
        out_group_content.append("### ");
        out_group_content.append(qualifiedname.c_str(), qualifiedname.size());
        out_group_content.append("\n\n");
        out_group_content.append("```c++\n");
        out_group_content.append(definition.c_str(), definition.size());
        out_group_content.append(argsstring.c_str(), argsstring.size());
        out_group_content.append("\n```\n\n");
        if(!briefdescription.empty())
        {
            out_group_content.append(briefdescription);
        }
        if(!detaileddescription.empty())
        {
            out_group_content.append(detaileddescription);
        }
    }
}

RV parse_group(const Variant& group, String& out_group_content, Name& out_group_filename)
{
    lutry
    {
        out_group_filename = Name();
        if(get_xml_name(group) != _doxygen) return set_error(BasicError::format_error(), "One doxygen XML file must begin with <doxygen>, got %s", get_xml_name(group).c_str());
        auto& compounddef = find_first_xml_child_element(group, _compounddef);
        if(compounddef.type() != VariantType::object) return set_error(BasicError::format_error(), "<compounddef> not found");
        auto& group_content = get_xml_content(compounddef);
        String title;
        String briefdescription;
        String detaileddescription;
        Vector<String> sections;
        for(auto& c : group_content.values())
        {
            if(c.type() != VariantType::object) continue;
            auto name = get_xml_name(c);
            if(name == _compoundname)
            {
                auto name = get_xml_content(c).at(0).str();
                if(!name) return set_error(BasicError::format_error(), "<compoundname> not found for group <compounddef>");
                out_group_filename = name;
            }
            else if(name == _title)
            {
                encode_markdown_text(title, c);
            }
            else if(name == _briefdescription)
            {
                encode_markdown_text(briefdescription, c);
            }
            else if(name == _detaileddescription)
            {
                encode_markdown_text(detaileddescription, c);
            }
            else if(name == _sectiondef)
            {
                auto kind = get_xml_attributes(c)[_kind].str();
                if(kind == _func)
                {
                    String section;
                    parse_func_section(section, c);
                    sections.push_back(move(section));
                }
            }
        }
        if(!title.empty())
        {
            out_group_content.append("# ");
            out_group_content.append(title);
            out_group_content.push_back('\n');
        }
        out_group_content.append(briefdescription);
        out_group_content.append(detaileddescription);
        for(auto& s : sections)
        {
            out_group_content.append(s);
        }
    }
    lucatchret;
    return ok;
}