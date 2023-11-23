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

void Parser::encode_md_text(const Variant& element, String& out_text)
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
                c8 ch = text.c_str()[i];
                if(ch < 0 || ch > 127 || !isspace(ch))
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
                encode_md_text(c, out_text);
                new_paragraph(out_text);
            }
            else if(name == _parameterlist)
            {
                new_paragraph(out_text);
                encode_md_parameter_list(c, out_text);
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
    out_group_content.append("## Properties\n");
    auto& content = get_xml_content(section);
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
        out_group_content.append("### ");
        out_group_content.append(name.c_str(), name.size());
        out_group_content.append("\n\n");
        out_group_content.append("```c++\n");
        if(!templateparamlist.empty()) out_group_content.append(templateparamlist);
        out_group_content.append(definition.c_str(), definition.size());
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

void Parser::encode_md_func_section(const Variant& section, String& out_group_content)
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
                encode_md_text(m, briefdescription);
            }
            else if(member_name == _detaileddescription)
            {
                encode_md_text(m, detaileddescription);
            }
        }
        out_group_content.append("### ");
        out_group_content.append(qualifiedname.c_str(), qualifiedname.size());
        out_group_content.append("\n\n");
        out_group_content.append("```c++\n");
        if(!templateparamlist.empty()) out_group_content.append(templateparamlist);
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

void Parser::encode_md_typedef_section(const Variant& section, String& out_group_content)
{
    out_group_content.append("## Aliasing types\n");
    auto& content = get_xml_content(section);
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
        out_group_content.append("### ");
        out_group_content.append(qualifiedname.c_str(), qualifiedname.size());
        out_group_content.append("\n\n");
        out_group_content.append("```c++\n");
        out_group_content.append(definition.c_str(), definition.size());
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

RV Parser::encode_md_class_file(const Variant& xml_data, String& out_content)
{
    lutry
    {
        auto& compounddef = find_first_xml_child_element(xml_data, _compounddef);
        auto& class_content = get_xml_content(compounddef);
        Name title;
        Name base_class;
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
                base_class = basecompoundref;
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
        out_content.append("\n\n");
        if(!briefdescription.empty())
        {
            out_content.append(briefdescription);
        }
        if(!detaileddescription.empty())
        {
            out_content.append(detaileddescription);
        }
        for(auto& s : sections)
        {
            out_content.append(s);
        }
    }
    lucatchret;
    return ok;
}

RV Parser::encode_md_group_file(const Variant& group, String& out_group_content)
{
    lutry
    {
        auto& compounddef = find_first_xml_child_element(group, _compounddef);
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
                auto refid = attr[_refid].str();
                String innerclass;
                innerclass.push_back('[');
                encode_md_text(c, innerclass);
                innerclass.append("](");
                innerclass.append(refid.c_str(), refid.size());
                innerclass.append(".md)");
                innerclasses.push_back(move(innerclass));
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
        if(!innergroups.empty())
        {
            out_group_content.append("## Topics\n");
            for(auto& innergroup : innergroups)
            {
                out_group_content.append("* ");
                out_group_content.append(innergroup);
                out_group_content.push_back('\n');
            }
        }
        if(!innerclasses.empty())
        {
            out_group_content.append("## Classes\n");
            for(auto& innerclass : innerclasses)
            {
                out_group_content.append("* ");
                out_group_content.append(innerclass);
                out_group_content.push_back('\n');
            }
        }
        for(auto& s : sections)
        {
            out_group_content.append(s);
        }
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
    class_files.insert(make_pair(class_id, move(file_data)));
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
        for(auto& g : group_files)
        {
            String group_md_text;
            luexp(encode_md_group_file(g.second, group_md_text));
            Path path = output_dir;
            path.push_back(g.first);
            path.append_extension("md");
            String path_str = path.encode();
            log_info("LunaDoc", "Write %s", path_str.c_str());
            lulet(f, open_file(path_str.c_str(), FileOpenFlag::write, FileCreationMode::create_always));
            luexp(f->write(group_md_text.c_str(), group_md_text.size()));
        }
        for(auto& c : class_files)
        {
            String class_md_text;
            luexp(encode_md_class_file(c.second, class_md_text));
            Path path = output_dir;
            path.push_back(c.first);
            path.append_extension("md");
            String path_str = path.encode();
            log_info("LunaDoc", "Write %s", path_str.c_str());
            lulet(f, open_file(path_str.c_str(), FileOpenFlag::write, FileCreationMode::create_always));
            luexp(f->write(class_md_text.c_str(), class_md_text.size()));
        }
    }
    lucatchret;
    return ok;
}