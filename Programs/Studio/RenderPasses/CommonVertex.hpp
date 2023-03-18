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
    struct CommonVertex
    {
        lustruct("CommonVertex", "{8c672a8b-ed16-4bdc-a6e7-a42f01d92710}");

        Blob vs_blob;

        RV init();
    };

    inline RHI::InputLayoutDesc get_vertex_input_layout_desc()
    {
        using namespace RHI;
        return RHI::InputLayoutDesc({
			InputElementDesc("POSITION", 0, Format::rgb32_float),
			InputElementDesc("NORMAL", 0, Format::rgb32_float),
			InputElementDesc("TANGENT", 0, Format::rgb32_float),
			InputElementDesc("TEXCOORD", 0, Format::rg32_float),
			InputElementDesc("COLOR", 0, Format::rgba32_float),
		});
    }
}