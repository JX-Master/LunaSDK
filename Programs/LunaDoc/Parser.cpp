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
Name _innergroup;
Name _refid;
Name _templateparamlist;
Name _typedef;
Name _innerclass;
Name _basecompoundref;
Name _publicattrib;
Name _publicfunc;
Name _variable;
Name _ref;
Name _ulink;
Name _url;
Name _par;
Name _itemizedlist;
Name _listitem;

static void new_paragraph(String& out_text)
{
    // make sure only one blank is inserted per paragraph.
    if(out_text.size() >= 2 && out_text[out_text.size() - 2] == '\n' && out_text[out_text.size() - 1] == '\n') return;
    out_text.push_back('\n');
    out_text.push_back('\n');
}

void Parser::encode_md_parameter_list(const Variant& parameterlist, String& out_text)
{
    out_text.append("#### Parameters\n");
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
                encode_md_text(e, out_text);
            }
        }
    }
}

inline bool is_blank_string(const c8* str)
{
    while(*str)
    {
        c8 ch = *str;
        if(ch < 0 || ch > 127 || !isspace(ch))
        {
            return false;
        }
        ++str;
    }
    return true;
}

void Parser::encode_md_text(const Variant& element, String& out_text)
{
    auto& content = get_xml_content(element);
    for(auto& c : content.values())
    {
        if(c.type() == VariantType::string)
        {
            Name text = c.str();
            if(!is_blank_string(text.c_str())) out_text.append(text.c_str());
        }
        else
        {
            auto name = get_xml_name(c);
            if(name == _para)
            {
                encode_md_text(c, out_text);
                new_paragraph(out_text);
            }
            else if(name == _parameterlist)
            {
                new_paragraph(out_text);
                encode_md_parameter_list(c, out_text);
            }
            else if(name == _title)
            {
                out_text.append("#### ");
                encode_md_text(c, out_text);
                out_text.push_back('\n');
            }
            else if(name == _itemizedlist)
            {
                auto& list_items = get_xml_content(c);
                for(auto& item : list_items.values())
                {
                    if(get_xml_name(item) == _listitem)
                    {
                        out_text.append("* ");
                        encode_md_text(item, out_text);
                    }
                }
            }
            else if(name == _simplesect)
            {
                auto& attributes = get_xml_attributes(c);
                if(attributes[_kind].str() == _return)
                {
                    new_paragraph(out_text);
                    out_text.append("#### Return value\n");
                    encode_md_text(c, out_text);
                }
                else if(attributes[_kind].str() == _par)
                {
                    new_paragraph(out_text);
                    encode_md_text(c, out_text);
                }
            }
            else if(name == _computeroutput)
            {
                out_text.push_back('`');
                encode_md_text(c, out_text);
                out_text.push_back('`');
            }
            else if(name == _ref)
            {
                auto& attributes = get_xml_attributes(c);
                Name id = attributes[_refid].str();
                bool valid_ref = false;
                {
                    auto iter = class_files.find(id);
                    if(iter != class_files.end())
                    {
                        valid_ref = true;
                    }
                }
                if(valid_ref)
                {
                    out_text.push_back('[');
                    encode_md_text(c, out_text);
                    out_text.push_back(']');
                    out_text.push_back('(');
                    out_text.append(id.c_str(), id.size());
                    out_text.append(".md)");
                }
                else
                {
                    encode_md_text(c, out_text);
                }
            }
            else if(name == _ulink)
            {
                auto& attributes = get_xml_attributes(c);
                Name url = attributes[_url].str();
                out_text.push_back('[');
                encode_md_text(c, out_text);
                out_text.push_back(']');
                out_text.push_back('(');
                out_text.append(url.c_str(), url.size());
                out_text.push_back(')');
            }
        }
    }
}

void Parser::encode_md_attrib_section(const Variant& section, String& out_group_content)
{
    auto& content = get_xml_content(section);
    String attrib_section;
    for(auto& c : content.values())
    {
        if(get_xml_name(c) != _memberdef) continue;
        auto& attributes = get_xml_attributes(c);
        if(attributes[_kind].str() != _variable) continue;
        auto& members = get_xml_content(c);
        Name name;
        Name definition;
        String templateparamlist;
        String briefdescription;
        String detaileddescription;
        for(auto& m : members.values())
        {
            auto member_name = get_xml_name(m);
            if(member_name == _templateparamlist)
            {
                templateparamlist.append("template <");
                auto& params = get_xml_content(m);
                for(auto& p : params.values())
                {
                    auto param_name = get_xml_name(p);
                    if(param_name == _param)
                    {
                        auto& type = get_xml_content(p).at(0);
                        auto type_name = get_xml_name(type);
                        if(type_name == _type)
                        {
                            templateparamlist.append(get_xml_content(type).at(0).c_str());
                            templateparamlist.append(", ");
                        }
                    }
                }
                if(templateparamlist[templateparamlist.size() - 2] == ',' && templateparamlist[templateparamlist.size() - 1] == ' ')
                {
                    templateparamlist.pop_back();
                    templateparamlist.pop_back();
                }
                templateparamlist.append(">\n");
            }
            else if(member_name == _definition)
            {
                definition = get_xml_content(m).at(0).str();
            }
            else if(member_name == _name)
            {
                name = get_xml_content(m).at(0).str();
            }
            else if(member_name == _briefdescription)
            {
                encode_md_text(m, briefdescription);
            }
            else if(member_name == _detaileddescription)
            {
                encode_md_text(m, detaileddescription);
            }
        }
        // Skip undocumented entry.
        if(is_blank_string(briefdescription.c_str()) && is_blank_string(detaileddescription.c_str()))
        {
            continue;
        }
        attrib_section.append("### ");
        attrib_section.append(name.c_str(), name.size());
        attrib_section.append("\n\n");
        attrib_section.append("```c++\n");
        if(!templateparamlist.empty()) attrib_section.append(templateparamlist);
        attrib_section.append(definition.c_str(), definition.size());
        attrib_section.append("\n```\n\n");
        if(!briefdescription.empty())
        {
            attrib_section.append(briefdescription);
        }
        if(!detaileddescription.empty())
        {
            attrib_section.append(detaileddescription);
        }
        attrib_section.append("---\n");
    }
    if(!attrib_section.empty())
    {
        out_group_content.append("## Properties\n");
        out_group_content.append(attrib_section);
    }
}

void Parser::encode_md_func_section(const Variant& section, String& out_group_content)
{
    auto& content = get_xml_content(section);
    String function_section;
    for(auto& c : content.values())
    {
        if(get_xml_name(c) != _memberdef) continue;
        auto& attributes = get_xml_attributes(c);
        if(attributes[_kind].str() != _function) continue;
        auto& func_members = get_xml_content(c);
        Name name;
        Name definition;
        Name argsstring;
        String templateparamlist;
        String briefdescription;
        String detaileddescription;
        for(auto& m : func_members.values())
        {
            auto member_name = get_xml_name(m);
            if(member_name == _templateparamlist)
            {
                templateparamlist.append("template <");
                auto& params = get_xml_content(m);
                for(auto& p : params.values())
                {
                    auto param_name = get_xml_name(p);
                    if(param_name == _param)
                    {
                        auto& type = get_xml_content(p).at(0);
                        auto type_name = get_xml_name(type);
                        if(type_name == _type)
                        {
                            templateparamlist.append(get_xml_content(type).at(0).c_str());
                            templateparamlist.append(", ");
                        }
                    }
                }
                if(templateparamlist[templateparamlist.size() - 2] == ',' && templateparamlist[templateparamlist.size() - 1] == ' ')
                {
                    templateparamlist.pop_back();
                    templateparamlist.pop_back();
                }
                templateparamlist.append(">\n");
            }
            else if(member_name == _type)
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
                name = get_xml_content(m).at(0).str();
            }
            else if(member_name == _qualifiedname)
            {
                //qualifiedname = get_xml_content(m).at(0).str();
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
                encode_md_text(m, briefdescription);
            }
            else if(member_name == _detaileddescription)
            {
                encode_md_text(m, detaileddescription);
            }
        }
        // Skip undocumented entry.
        if(is_blank_string(briefdescription.c_str()) && is_blank_string(detaileddescription.c_str()))
        {
            continue;
        }
        function_section.append("### ");
        function_section.append(name.c_str(), name.size());
        function_section.append("\n\n");
        function_section.append("```c++\n");
        if(!templateparamlist.empty()) function_section.append(templateparamlist);
        function_section.append(definition.c_str(), definition.size());
        function_section.append(argsstring.c_str(), argsstring.size());
        function_section.append("\n```\n\n");
        if(!briefdescription.empty())
        {
            function_section.append(briefdescription);
        }
        if(!detaileddescription.empty())
        {
            function_section.append(detaileddescription);
        }
        function_section.append("---\n");
    }
    if(!function_section.empty())
    {
        out_group_content.append("## Functions\n");
        out_group_content.append(function_section);
    }
}

void Parser::encode_md_typedef_section(const Variant& section, String& out_group_content)
{
    auto& content = get_xml_content(section);
    String typedef_section;
    for(auto& c : content.values())
    {
        if(get_xml_name(c) != _memberdef) continue;
        auto& attributes = get_xml_attributes(c);
        if(attributes[_kind].str() != _typedef) continue;
        auto& type_members = get_xml_content(c);
        Name qualifiedname;
        Name definition;
        String briefdescription;
        String detaileddescription;
        for(auto& m : type_members.values())
        {
            auto member_name = get_xml_name(m);
            if(member_name == _qualifiedname)
            {
                qualifiedname = get_xml_content(m).at(0).str();
            }
            else if(member_name == _definition)
            {
                definition = get_xml_content(m).at(0).str();
            }
            else if(member_name == _briefdescription)
            {
                encode_md_text(m, briefdescription);
            }
            else if(member_name == _detaileddescription)
            {
                encode_md_text(m, detaileddescription);
            }
        }
        // Skip undocumented entry.
        if(is_blank_string(briefdescription.c_str()) && is_blank_string(detaileddescription.c_str()))
        {
            continue;
        }
        typedef_section.append("### ");
        typedef_section.append(qualifiedname.c_str(), qualifiedname.size());
        typedef_section.append("\n\n");
        typedef_section.append("```c++\n");
        typedef_section.append(definition.c_str(), definition.size());
        typedef_section.append("\n```\n\n");
        if(!briefdescription.empty())
        {
            typedef_section.append(briefdescription);
        }
        if(!detaileddescription.empty())
        {
            typedef_section.append(detaileddescription);
        }
        typedef_section.append("---\n");
    }
    if(!typedef_section.empty())
    {
        out_group_content.append("## Aliasing types\n");
        out_group_content.append(typedef_section);
    }
}

struct BaseClassDesc
{
    Name name;
    Name prot;
    Name virt;
};

RV Parser::encode_md_class_file(const Name& xml_name, const Variant& xml_data, const Path& output_dir)
{
    lutry
    {
        String out_content;
        auto& compounddef = find_first_xml_child_element(xml_data, _compounddef);
        auto& class_content = get_xml_content(compounddef);
        auto& attributes = get_xml_attributes(compounddef);
        Name title;
        Vector<BaseClassDesc> base_classes;
        Name kind = attributes[_kind].str();
        String briefdescription;
        String detaileddescription;
        Vector<String> sections;
        for(auto& m : class_content.values())
        {
            if(m.type() != VariantType::object) continue;
            auto member_name = get_xml_name(m);
            if(member_name == _compoundname)
            {
                auto compoundname = get_xml_content(m).at(0).str();
                if(!compoundname) return set_error(BasicError::format_error(), "<compoundname> not found for group <compounddef>");
                title = compoundname;
            }
            else if(member_name == _basecompoundref)
            {
                auto basecompoundref = get_xml_content(m).at(0).str();
                auto& baseattrs = get_xml_attributes(m);
                BaseClassDesc base;
                base.name = basecompoundref;
                base.prot = baseattrs[_prot].str();
                base.virt = baseattrs[_virt].str();
                base_classes.push_back(move(base));
            }
            else if(member_name == _sectiondef)
            {
                auto& section_attributes = get_xml_attributes(m);
                Name section_kind = section_attributes[_kind].str();
                String section;
                if(section_kind == _publicattrib)
                {
                    encode_md_attrib_section(m, section);
                }
                else if(section_kind == _publicfunc)
                {
                    encode_md_func_section(m, section);
                }
                sections.push_back(move(section));
            }
            else if(member_name == _briefdescription)
            {
                encode_md_text(m, briefdescription);
            }
            else if(member_name == _detaileddescription)
            {
                encode_md_text(m, detaileddescription);
            }
        }
        out_content.append("# ");
        out_content.append(title.c_str(), title.size());
        out_content.append("\n");
        if(!is_blank_string(briefdescription.c_str()))
        {
            out_content.append(briefdescription);
        }
        out_content.append("```c++\n");
        out_content.append(kind.c_str(), kind.size());
        out_content.push_back(' ');
        out_content.append(title.c_str(), title.size());
        if(!base_classes.empty())
        {
            out_content.append(" : ");
            for(usize i = 0; i < base_classes.size(); ++i)
            {
                auto& base = base_classes[i];
                if(base.prot)
                {
                    out_content.append(base.prot.c_str(), base.prot.size());
                    out_content.push_back(' ');
                }
                if(base.virt)
                {
                    out_content.append(base.virt.c_str(), base.virt.size());
                    out_content.push_back(' ');
                }
                out_content.append(base.name.c_str(), base.name.size());
                if(i != base_classes.size() - 1)
                {
                    out_content.append(", ");
                }
            }
        }
        out_content.append("\n```\n\n");
        if(!is_blank_string(detaileddescription.c_str()))
        {
            out_content.append("## Overview\n");
            out_content.append(detaileddescription);
        }
        for(auto& s : sections)
        {
            out_content.append(s);
        }
        // Write file.
        Path path = output_dir;
        path.push_back(xml_name);
        path.append_extension("md");
        String path_str = path.encode();
        log_info("LunaDoc", "Write %s", path_str.c_str());
        lulet(f, open_file(path_str.c_str(), FileOpenFlag::write, FileCreationMode::create_always));
        luexp(f->write(out_content.c_str(), out_content.size()));
    }
    lucatchret;
    return ok;
}

RV Parser::encode_md_group_file(const Name& xml_name, const Variant& xml_data, const Path& output_dir)
{
    lutry
    {
        String out_content;
        auto& compounddef = find_first_xml_child_element(xml_data, _compounddef);
        auto& group_content = get_xml_content(compounddef);
        String title;
        String briefdescription;
        String detaileddescription;
        Vector<String> sections;
        Vector<String> innergroups;
        Vector<String> innerclasses;
        for(auto& c : group_content.values())
        {
            if(c.type() != VariantType::object) continue;
            auto name = get_xml_name(c);
            if(name == _compoundname)
            {
                // auto name = get_xml_content(c).at(0).str();
                // if(!name) return set_error(BasicError::format_error(), "<compoundname> not found for group <compounddef>");
                // out_group_filename = name;
            }
            else if(name == _title)
            {
                encode_md_text(c, title);
            }
            else if(name == _briefdescription)
            {
                encode_md_text(c, briefdescription);
            }
            else if(name == _detaileddescription)
            {
                encode_md_text(c, detaileddescription);
            }
            else if(name == _sectiondef)
            {
                auto kind = get_xml_attributes(c)[_kind].str();
                String section;
                if(kind == _func)
                {
                    encode_md_func_section(c, section);
                }
                else if(kind == _typedef)
                {
                    encode_md_typedef_section(c, section);
                }
                sections.push_back(move(section));
            }
            else if(name == _innergroup)
            {
                auto& attr = get_xml_attributes(c);
                auto refid = attr[_refid].str();
                luassert(refid.size() > 8);
                Name groupname = refid.c_str() + 8;
                String innergroup;
                innergroup.push_back('[');
                encode_md_text(c, innergroup);
                innergroup.append("](");
                innergroup.append(groupname.c_str(), groupname.size());
                innergroup.append(".md)");
                innergroups.push_back(move(innergroup));
            }
            else if(name == _innerclass)
            {
                auto& attr = get_xml_attributes(c);
                Name refid = attr[_refid].str();
                bool valid_ref = false;
                {
                    auto iter = class_files.find(refid);
                    if(iter != class_files.end())
                    {
                        valid_ref = true;
                    }
                }
                if(valid_ref)
                {
                    String innerclass;
                    innerclass.push_back('[');
                    encode_md_text(c, innerclass);
                    innerclass.append("](");
                    innerclass.append(refid.c_str(), refid.size());
                    innerclass.append(".md)");
                    innerclasses.push_back(move(innerclass));
                }
            }
        }
        if(!title.empty())
        {
            out_content.append("# ");
            out_content.append(title);
            out_content.push_back('\n');
        }
        out_content.append(briefdescription);
        out_content.append(detaileddescription);
        if(!innergroups.empty())
        {
            out_content.append("## Topics\n");
            for(auto& innergroup : innergroups)
            {
                out_content.append("* ");
                out_content.append(innergroup);
                out_content.push_back('\n');
            }
        }
        if(!innerclasses.empty())
        {
            out_content.append("## Classes\n");
            for(auto& innerclass : innerclasses)
            {
                out_content.append("* ");
                out_content.append(innerclass);
                out_content.push_back('\n');
            }
        }
        for(auto& s : sections)
        {
            out_content.append(s);
        }
        // Write file.
        Path path = output_dir;
        path.push_back(xml_name);
        path.append_extension("md");
        String path_str = path.encode();
        log_info("LunaDoc", "Write %s", path_str.c_str());
        lulet(f, open_file(path_str.c_str(), FileOpenFlag::write, FileCreationMode::create_always));
        luexp(f->write(out_content.c_str(), out_content.size()));
    }
    lucatchret;
    return ok;
}

RV Parser::add_group_xml_file(Variant&& file_data)
{
    if(get_xml_name(file_data) != _doxygen) return set_error(BasicError::format_error(), "One doxygen XML file must begin with <doxygen>, got %s", get_xml_name(file_data).c_str());
    auto& compounddef = find_first_xml_child_element(file_data, _compounddef);
    if(compounddef.type() != VariantType::object) return set_error(BasicError::format_error(), "<compounddef> not found");
    auto& compounddef_attribtues = get_xml_attributes(compounddef);
    Name group_id = compounddef_attribtues[_id].str();
    luassert(group_id.size() > 8);
    Name group_filename = group_id.c_str() + 8;
    group_files.insert(make_pair(group_filename, move(file_data)));
    return ok;
}

RV Parser::add_class_xml_file(Variant&& file_data)
{
    if(get_xml_name(file_data) != _doxygen) return set_error(BasicError::format_error(), "One doxygen XML file must begin with <doxygen>, got %s", get_xml_name(file_data).c_str());
    auto& compounddef = find_first_xml_child_element(file_data, _compounddef);
    if(compounddef.type() != VariantType::object) return set_error(BasicError::format_error(), "<compounddef> not found");
    auto& compounddef_attribtues = get_xml_attributes(compounddef);
    Name class_id = compounddef_attribtues[_id].str();
    // Skip classes without docs.
    bool doc_class = true;
    {
        String briefdescription;
        String detaileddescription;
        auto& bd = find_first_xml_child_element(compounddef, _briefdescription);
        if(bd.valid())
        {
            encode_md_text(bd, briefdescription);
        }
        auto& dd = find_first_xml_child_element(compounddef, _detaileddescription);
        if(dd.valid())
        {
            encode_md_text(dd, briefdescription);
        }
        if(is_blank_string(briefdescription.c_str()) && is_blank_string(detaileddescription.c_str()))
        {
            doc_class = false;
        }
    }
    if(doc_class)
    {
        class_files.insert(make_pair(class_id, move(file_data)));
    }
    return ok;
}

RV Parser::encode_md_files(const Path& output_dir)
{
    lutry
    {
        // Create directory if not exist
        {
            auto dir = get_file_attribute(output_dir.encode().c_str());
            if (failed(dir))
            {
                luexp(create_dir(output_dir.encode().c_str()));
            }
        }
        // Encode group files.
        for(auto& g : group_files)
        {
            luexp(encode_md_group_file(g.first, g.second, output_dir));
        }
        for(auto& c : class_files)
        {
            luexp(encode_md_class_file(c.first, c.second, output_dir));
        }
    }
    lucatchret;
    return ok;
}