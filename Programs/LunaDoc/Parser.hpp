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

R<Vector<Variant>> parse_groups(Span<const Variant> group_files);
RV write_group_to_file(const Variant& group, const c8* filepath);