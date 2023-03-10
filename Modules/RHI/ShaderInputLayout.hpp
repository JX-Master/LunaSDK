/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShaderInputLayout.hpp
* @author JXMaster
* @date 2019/7/20
*/
#pragma once
#include <Runtime/Vector.hpp>
#include "DeviceChild.hpp"
#include "DescriptorSetLayout.hpp"

namespace Luna
{
	namespace RHI
	{
		enum class ShaderInputLayoutFlag : u32
		{
			none = 0,
			//! Input assembler and input layout is used.
			allow_input_assembler_input_layout = 0x01,
			//! Vertex shader cannot access the shader input.
			deny_vertex_shader_access = 0x02,
			//! Hull shader cannot access the shader input.
			deny_hull_shader_access = 0x04,
			//! Domain shader cannot access the shader input.
			deny_domain_shader_access = 0x08,
			//! Geometry shader cannot access the shader input.
			deny_geometry_shader_access = 0x10,
			//! Pixel shader cannot access the shader input.
			deny_pixel_shader_access = 0x20,
			//! Stream output is used.
			allow_stream_output = 0x40,
		};

		//! Describes the shader input layout.
		struct ShaderInputLayoutDesc
		{
			Vector<IDescriptorSetLayout*> descriptor_set_layouts;
			ShaderInputLayoutFlag flags;

			ShaderInputLayoutDesc() = default;
			ShaderInputLayoutDesc(
				InitializerList<IDescriptorSetLayout*> descriptor_set_layouts,
				ShaderInputLayoutFlag flags = ShaderInputLayoutFlag::none) :
				descriptor_set_layouts(descriptor_set_layouts),
				flags(flags) {}
		};

		//! @interface IShaderInputLayout
		//! Describes how shader inputs are accessed by every shader
		//! in the pipeline.
		struct IShaderInputLayout : virtual IDeviceChild
		{
			luiid("{347097dc-04e2-44e8-a9a0-3f89e77b4425}");

			//virtual ShaderInputLayoutDesc get_desc() = 0;
		};
	}
}