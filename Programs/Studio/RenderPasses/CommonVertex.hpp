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
    inline RHI::InputLayoutDesc get_vertex_input_layout_desc()
    {
        using namespace RHI;
        return RHI::InputLayoutDesc({
			InputAttributeDesc("POSITION", 0, Format::rgb32_float),
			InputAttributeDesc("NORMAL", 0, Format::rgb32_float),
			InputAttributeDesc("TANGENT", 0, Format::rgb32_float),
			InputAttributeDesc("TEXCOORD", 0, Format::rg32_float),
			InputAttributeDesc("COLOR", 0, Format::rgba32_float),
		});
    }
}