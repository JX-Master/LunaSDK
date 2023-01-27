/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SemanticAnalyzer.hpp
* @author JXMaster
* @date 2020/12/2
*/
#pragma once
#include "Variant.hpp"
#include "Result.hpp"
namespace Luna
{
	LUNA_RUNTIME_API RV semantic_analyze(Vector<Variant>& asts, const Variant& assembly_reference);
}