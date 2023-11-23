/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Parser.hpp
* @author JXMaster
* @date 2023/10/23
*/
#pragma once
#include <Luna/Runtime/Name.hpp>
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Runtime/Variant.hpp>
#include <Luna/Runtime/Span.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Path.hpp>
using namespace Luna;

extern Name _doxygen;
extern Name _compounddef;
extern Name _compoundname;
extern Name _name;
extern Name _title;
extern Name _sectiondef;
extern Name _briefdescription;
extern Name _detaileddescription;
extern Name _kind;
extern Name _func;
extern Name _memberdef;
extern Name _id;
extern Name _function;
extern Name _prot;
extern Name _static;
extern Name _constexpr;
extern Name _const;
extern Name _explicit;
extern Name _ninline;
extern Name _virt;
extern Name _no;
extern Name _yes;
extern Name _type;
extern Name _definition;
extern Name _argsstring;
extern Name _qualifiedname;
extern Name _param;
extern Name _declname;
extern Name _para;
extern Name _parameterlist;
extern Name _simplesect;
extern Name _return;
extern Name _parameternamelist;
extern Name _parameterdescription;
extern Name _parametername;
extern Name _parameteritem;
extern Name _computeroutput;
extern Name _innergroup;
extern Name _refid;
extern Name _templateparamlist;
extern Name _typedef;
extern Name _innerclass;
extern Name _basecompoundref;
extern Name _publicattrib;
extern Name _publicfunc;
extern Name _variable;
extern Name _ref;
extern Name _ulink;
extern Name _url;

struct Parser
{
    // All read group files, identified by their refid.
    HashMap<Name, Variant> group_files;
    // All read class files, identified by their refid.
    HashMap<Name, Variant> class_files;

    RV add_group_xml_file(Variant&& file_data);
    RV add_class_xml_file(Variant&& file_data);
    RV encode_md_files(const Path& output_dir);

    private:
    void encode_md_parameter_list(const Variant& parameterlist, String& out_text);
    void encode_md_text(const Variant& element, String& out_text);
    void encode_md_attrib_section(const Variant& section, String& out_group_content);
    void encode_md_func_section(const Variant& section, String& out_group_content);
    void encode_md_typedef_section(const Variant& section, String& out_group_content);
    RV encode_md_class_file(const Variant& xml_data, String& out_content);
    RV encode_md_group_file(const Variant& xml_data, String& out_content);
};