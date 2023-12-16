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
Name _orderedlist;
Name _listitem;
Name _remark;
Name _define;
Name _defname;
Name _direction;
Name _emphasis;
Name _bold;
Name _var;
Name _initializer;

static void new_paragraph(String& out_text)
{
    // make sure only one blank is inserted per paragraph.
    if(out_text.size() >= 2 && out_text[out_text.size() - 2] == '\n' && out_text[out_text.size() - 1] == '\n') return;
    out_text.push_back('\n');
    out_text.push_back('\n');
}

void Parser::encode_md_parameter_list(const Variant& parameterlist, String& out_text)
{
    out_text.append("## Parameters\n");
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
                auto& direction = get_xml_attributes(parameter_name)[_direction].str();
                if(get_xml_name(parameter_name) == _parametername)
                {
                    out_text.append("* ");
                    if(direction)
                    {
                        out_text.push_back('*');
                        out_text.append(direction.c_str(), direction.size());
                        out_text.append("* ");
                    }
                    out_text.append("**");
                    out_text.append(get_xml_content(parameter_name).at(0).str().c_str());
                    out_text.append("**\n\n    ");
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

void Parser::encode_md_text(const Variant& element, String& out_text, bool raw)
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
                out_text.append("## ");
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
            else if(name == _orderedlist)
            {
                auto& list_items = get_xml_content(c);
                for(auto& item : list_items.values())
                {
                    if(get_xml_name(item) == _listitem)
                    {
                        out_text.append("1. ");
                        encode_md_text(item, out_text);
                    }
                }
            }
            else if(name == _simplesect)
            {
                auto& attributes = get_xml_attributes(c);
                Name kind = attributes[_kind].str();
                if(kind == _return)
                {
                    new_paragraph(out_text);
                    out_text.append("## Return value\n");
                    encode_md_text(c, out_text);
                }
                else if(kind == _par)
                {
                    new_paragraph(out_text);
                    encode_md_text(c, out_text);
                }
                else if(kind == _remark)
                {
                    new_paragraph(out_text);
                    out_text.append("## Remark\n");
                    encode_md_text(c, out_text);
                }
            }
            else if(name == _emphasis)
            {
                out_text.push_back('*');
                encode_md_text(c, out_text);
                out_text.push_back('*');
            }
            else if(name == _bold)
            {
                out_text.append("**");
                encode_md_text(c, out_text);
                out_text.append("**");
            }
            else if(name == _computeroutput)
            {
                out_text.push_back('`');
                encode_md_text(c, out_text, true);
                out_text.push_back('`');
            }
            else if(name == _ref)
            {
                auto& attributes = get_xml_attributes(c);
                Name id = attributes[_refid].str();
                bool valid_ref = false;
                {
                    auto iter = ids.find(id);
                    if(iter != ids.end())
                    {
                        valid_ref = true;
                    }
                }
                if(valid_ref && !raw)
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

RV Parser::encode_md_attrib_section(const c8* section_name, const Variant& section, String& out_parent_content, const Path& output_dir)
{
    lutry
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
            Name qualifiedname;
            Name id = attributes[_id].str();
            Name is_constexpr = attributes[_constexpr].str();
            Name definition;
            Name initializer;
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
                else if(member_name == _qualifiedname)
                {
                    qualifiedname = get_xml_content(m).at(0).str();
                }
                else if(member_name == _initializer)
                {
                    initializer = get_xml_content(m).at(0).str();
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
            // Set file content.
            String file_content;
            file_content.append("# ");
            file_content.append(qualifiedname.c_str(), qualifiedname.size());
            file_content.append("\n\n");
            file_content.append("```c++\n");
            if(!templateparamlist.empty()) file_content.append(templateparamlist);
            file_content.append(definition.c_str(), definition.size());
            if(is_constexpr == _yes)
            {
                file_content.push_back(' ');
                file_content.append(initializer.c_str(), initializer.size());
            }
            file_content.append("\n```\n\n");
            if(!is_blank_string(briefdescription.c_str()))
            {
                file_content.append(briefdescription);
            }
            if(!is_blank_string(detaileddescription.c_str()))
            {
                file_content.append(detaileddescription);
            }
            // Write file.
            Path path = output_dir;
            path.push_back(id);
            path.append_extension("md");
            String path_str = path.encode();
            log_info("LunaDoc", "Write %s", path_str.c_str());
            lulet(f, open_file(path_str.c_str(), FileOpenFlag::write, FileCreationMode::create_always));
            luexp(f->write(file_content.c_str(), file_content.size()));
            // Write function section.
            attrib_section.append("* [");
            attrib_section.append(definition.c_str(), definition.size());
            attrib_section.append("](");
            attrib_section.append(id.c_str(), id.size());
            attrib_section.append(".md)\n");
            if(!is_blank_string(briefdescription.c_str()))
            {
                attrib_section.append("\n    ");
                attrib_section.append(briefdescription);
            }
        }
        if(!attrib_section.empty())
        {
            out_parent_content.append("## ");
            out_parent_content.append(section_name);
            out_parent_content.push_back('\n');
            out_parent_content.append(attrib_section);
        }
    }
    lucatchret;
    return ok;
}

RV Parser::encode_md_func_section(const c8* section_name, const Variant& section, String& out_parent_content, const Path& output_dir)
{
    lutry
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
            Name qualifiedname;
            Name id = attributes[_id].str();
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
                    qualifiedname = get_xml_content(m).at(0).str();
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
            // Set file content.
            String file_content;
            file_content.append("# ");
            file_content.append(qualifiedname.c_str(), qualifiedname.size());
            file_content.append("\n\n");
            file_content.append("```c++\n");
            if(!templateparamlist.empty()) file_content.append(templateparamlist);
            file_content.append(definition.c_str(), definition.size());
            file_content.append(argsstring.c_str(), argsstring.size());
            file_content.append("\n```\n\n");
            if(!is_blank_string(briefdescription.c_str()))
            {
                file_content.append(briefdescription);
            }
            if(!is_blank_string(detaileddescription.c_str()))
            {
                file_content.append(detaileddescription);
            }
            // Write file.
            Path path = output_dir;
            path.push_back(id);
            path.append_extension("md");
            String path_str = path.encode();
            log_info("LunaDoc", "Write %s", path_str.c_str());
            lulet(f, open_file(path_str.c_str(), FileOpenFlag::write, FileCreationMode::create_always));
            luexp(f->write(file_content.c_str(), file_content.size()));
            // Write function section.
            function_section.append("* [");
            function_section.append(definition.c_str(), definition.size());
            function_section.append(argsstring.c_str(), argsstring.size());
            function_section.append("](");
            function_section.append(id.c_str(), id.size());
            function_section.append(".md)\n");
            if(!is_blank_string(briefdescription.c_str()))
            {
                function_section.append("\n    ");
                function_section.append(briefdescription);
            }
        }
        if(!function_section.empty())
        {
            out_parent_content.append("## ");
            out_parent_content.append(section_name);
            out_parent_content.push_back('\n');
            out_parent_content.append(function_section);
        }
    }
    lucatchret;
    return ok;
}

RV Parser::encode_md_def_section(const c8* section_name, const Variant& section, String& out_parent_content, const Path& output_dir)
{
    lutry
    {
        auto& content = get_xml_content(section);
        String function_section;
        for(auto& c : content.values())
        {
            if(get_xml_name(c) != _memberdef) continue;
            auto& attributes = get_xml_attributes(c);
            if(attributes[_kind].str() != _define) continue;
            auto& func_members = get_xml_content(c);
            Name name;
            Name id = attributes[_id].str();
            String briefdescription;
            String detaileddescription;
            Vector<Name> param_list;
            for(auto& m : func_members.values())
            {
                auto member_name = get_xml_name(m);
                if(member_name == _name)
                {
                    name = get_xml_content(m).at(0).str();
                }
                else if(member_name == _param)
                {
                    auto& defname = find_first_xml_child_element(m, _defname);
                    param_list.push_back(get_xml_content(defname).at(0).str());
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
            String signature(name.c_str(), name.size());
            if(!param_list.empty())
            {
                signature.push_back('(');
                for(usize i = 0; i < param_list.size(); ++i)
                {
                    signature.append(param_list[i].c_str(), param_list[i].size());
                    if(i != param_list.size() - 1)
                    {
                        signature.append(", ");
                    }
                }
                signature.push_back(')');
            }
            // Set file content.
            String file_content;
            file_content.append("# ");
            file_content.append(name.c_str(), name.size());
            file_content.append("\n\n");
            file_content.append("```c++\n");
            file_content.append("#define ");
            file_content.append(signature);
            file_content.append("\n```\n\n");
            if(!is_blank_string(briefdescription.c_str()))
            {
                file_content.append(briefdescription);
            }
            if(!is_blank_string(detaileddescription.c_str()))
            {
                file_content.append(detaileddescription);
            }
            // Write file.
            Path path = output_dir;
            path.push_back(id);
            path.append_extension("md");
            String path_str = path.encode();
            log_info("LunaDoc", "Write %s", path_str.c_str());
            lulet(f, open_file(path_str.c_str(), FileOpenFlag::write, FileCreationMode::create_always));
            luexp(f->write(file_content.c_str(), file_content.size()));
            // Write function section.
            function_section.append("* [");
            function_section.append(signature);
            function_section.append("](");
            function_section.append(id.c_str(), id.size());
            function_section.append(".md)\n");
            if(!is_blank_string(briefdescription.c_str()))
            {
                function_section.append("\n    ");
                function_section.append(briefdescription);
            }
        }
        if(!function_section.empty())
        {
            out_parent_content.append("## ");
            out_parent_content.append(section_name);
            out_parent_content.push_back('\n');
            out_parent_content.append(function_section);
        }
    }
    lucatchret;
    return ok;
}

RV Parser::encode_md_typedef_section(const c8* section_name, const Variant& section, String& out_parent_content, const Path& output_dir)
{
    lutry
    {
        auto& content = get_xml_content(section);
        String typedef_section;
        for(auto& c : content.values())
        {
            if(get_xml_name(c) != _memberdef) continue;
            auto& attributes = get_xml_attributes(c);
            if(attributes[_kind].str() != _typedef) continue;
            auto& type_members = get_xml_content(c);
            Name name;
            Name qualifiedname;
            Name id = attributes[_id].str();
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
                else if(member_name == _name)
                {
                    name = get_xml_content(m).at(0).str();
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
            // Set file content.
            String file_content;
            file_content.append("# ");
            file_content.append(qualifiedname.c_str(), qualifiedname.size());
            file_content.append("\n\n");
            file_content.append("```c++\n");
            file_content.append(definition.c_str(), definition.size());
            file_content.append("\n```\n\n");
            if(!is_blank_string(briefdescription.c_str()))
            {
                file_content.append(briefdescription);
            }
            if(!is_blank_string(detaileddescription.c_str()))
            {
                file_content.append(detaileddescription);
            }
            // Write file.
            Path path = output_dir;
            path.push_back(id);
            path.append_extension("md");
            String path_str = path.encode();
            log_info("LunaDoc", "Write %s", path_str.c_str());
            lulet(f, open_file(path_str.c_str(), FileOpenFlag::write, FileCreationMode::create_always));
            luexp(f->write(file_content.c_str(), file_content.size()));
            // Write function section.
            typedef_section.append("* [");
            typedef_section.append(definition.c_str(), definition.size());
            typedef_section.append("](");
            typedef_section.append(id.c_str(), id.size());
            typedef_section.append(".md)\n");
            if(!is_blank_string(briefdescription.c_str()))
            {
                typedef_section.append("\n    ");
                typedef_section.append(briefdescription);
            }
        }
        if(!typedef_section.empty())
        {
            out_parent_content.append("## ");
            out_parent_content.append(section_name);
            out_parent_content.push_back('\n');
            out_parent_content.append(typedef_section);
        }
    }
    lucatchret;
    return ok;
}

struct BaseClassDesc
{
    Name name;
    Name prot;
    Name virt;
    Name id;
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
                base.id = baseattrs[_refid].str();
                base_classes.push_back(move(base));
            }
            else if(member_name == _sectiondef)
            {
                auto& section_attributes = get_xml_attributes(m);
                Name section_kind = section_attributes[_kind].str();
                String section;
                if(section_kind == _publicattrib)
                {
                    luexp(encode_md_attrib_section("Member objects", m, section, output_dir));
                }
                else if(section_kind == _publicfunc)
                {
                    luexp(encode_md_func_section("Member functions", m, section, output_dir));
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
            out_content.append(detaileddescription);
        }
        if(!base_classes.empty())
        {
            if(base_classes.size() == 1)
            {
                out_content.append("## Base type\n");
            }
            else
            {
                out_content.append("## Base types\n");
            }
            for(auto& base : base_classes)
            {
                out_content.append("* ");
                auto iter = ids.find(base.id);
                if(iter == ids.end())
                {
                    out_content.append(base.name.c_str(), base.name.size());
                }
                else
                {
                    out_content.push_back('[');
                    out_content.append(base.name.c_str(), base.name.size());
                    out_content.append("](");
                    out_content.append(base.id.c_str(), base.id.size());
                    out_content.append(".md)");
                }
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
                    luexp(encode_md_func_section("Functions", c, section, output_dir));
                }
                else if(kind == _typedef)
                {
                    luexp(encode_md_typedef_section("Alias types", c, section, output_dir));
                }
                else if(kind == _define)
                {
                    luexp(encode_md_def_section("Macros", c, section, output_dir));
                }
                else if(kind == _var)
                {
                    luexp(encode_md_attrib_section("Constants", c, section, output_dir));
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

void Parser::add_section_ids(const Variant& section)
{
    auto& content = get_xml_content(section);
    for(auto& c : content.values())
    {
        if(get_xml_name(c) != _memberdef) continue;
        auto& attributes = get_xml_attributes(c);
        auto& members = get_xml_content(c);
        Name id = attributes[_id].str();
        String briefdescription;
        String detaileddescription;
        for(auto& m : members.values())
        {
            auto member_name = get_xml_name(m);
            if(member_name == _briefdescription)
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
        // Add entry.
        ids.insert(id);
    }
}

void Parser::add_group_member_ids(const Variant& group_data)
{
    auto& compounddef = find_first_xml_child_element(group_data, _compounddef);
    auto& content = get_xml_content(compounddef);
    for(auto& c : content.values())
    {
        if(c.type() != VariantType::object) continue;
        auto name = get_xml_name(c);
        if(name == _sectiondef)
        {
            auto kind = get_xml_attributes(c)[_kind].str();
            if(kind == _func || kind == _typedef || kind == _define || kind == _var)
            {
                add_section_ids(c);
            }
        }
    }
}

void Parser::add_class_member_ids(const Variant& class_data)
{
    String out_content;
    auto& compounddef = find_first_xml_child_element(class_data, _compounddef);
    auto& class_content = get_xml_content(compounddef);
    for(auto& m : class_content.values())
    {
        if(m.type() != VariantType::object) continue;
        auto member_name = get_xml_name(m);
        if(member_name == _sectiondef)
        {
            Name kind = get_xml_attributes(m)[_kind].str();
            String section;
            if(kind == _publicattrib || kind == _publicfunc)
            {
                add_section_ids(m);
            }
        }
    }
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
    auto iter = group_files.insert(make_pair(group_filename, move(file_data)));
    add_group_member_ids(iter.first->second);
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
        auto iter = class_files.insert(make_pair(class_id, move(file_data)));
        ids.insert(class_id);
        add_class_member_ids(iter.first->second);
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