/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file CommonVertex.hpp
* @author JXMaster
* @date 2023/3/7
*/
#pragma once
#include <RHI/RHI.hpp>

namespace Luna
{
    inline void get_vertex_input_layout_desc(Vector<RHI::InputAttributeDesc>& attributes)
    {
        using namespace RHI;
		attributes.clear();
		attributes.push_back(InputAttributeDesc("POSITION", 0, 0, 0, 0, Format::rgb32_float));
		attributes.push_back(InputAttributeDesc("NORMAL", 0, 1, 0, 12, Format::rgb32_float));
		attributes.push_back(InputAttributeDesc("TANGENT", 0, 2, 0, 24, Format::rgb32_float));
		attributes.push_back(InputAttributeDesc("TEXCOORD", 0, 3, 0, 36, Format::rg32_float));
		attributes.push_back(InputAttributeDesc("COLOR", 0, 4, 0, 44, Format::rgba32_float));
    }
}