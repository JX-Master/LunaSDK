/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file PipelineLayout.hpp
* @author JXMaster
* @date 2019/7/20
*/
#pragma once
#include <Luna/Runtime/Vector.hpp>
#include "DeviceChild.hpp"
#include "DescriptorSetLayout.hpp"

namespace Luna
{
	namespace RHI
	{
		enum class PipelineLayoutFlag : u32
		{
			none = 0,
			//! Input assembler and input layout is used.
			allow_input_assembler_input_layout = 0x01,
			//! Vertex shader cannot access the shader input.
			deny_vertex_shader_access = 0x02,
			//! Pixel shader cannot access the shader input.
			deny_pixel_shader_access = 0x04,
		};

		//! Describes the pipeline layout.
		struct PipelineLayoutDesc
		{
			Span<IDescriptorSetLayout*> descriptor_set_layouts;
			PipelineLayoutFlag flags;

			PipelineLayoutDesc() = default;
			PipelineLayoutDesc(
				Span<IDescriptorSetLayout*> descriptor_set_layouts,
				PipelineLayoutFlag flags = PipelineLayoutFlag::none) :
				descriptor_set_layouts(descriptor_set_layouts),
				flags(flags) {}
		};

		//! @interface IPipelineLayout
		//! Describes how shader parameters are accessed by every shader
		//! in the pipeline.
		struct IPipelineLayout : virtual IDeviceChild
		{
			luiid("{347097dc-04e2-44e8-a9a0-3f89e77b4425}");

		};
	}
}