/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ShaderInputLayout.hpp
* @author JXMaster
* @date 2023/4/23
*/
#pragma once
#include "Device.hpp"
namespace Luna
{
	namespace RHI
	{
		struct ShaderInputLayout : IShaderInputLayout
		{
			lustruct("RHI::ShaderInputLayout", "{5B246B1A-354C-446A-924C-1E0F6040A82A}");
			luiimpl();

			Ref<Device> m_device;
			Name m_name;
			VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;

			RV init(const ShaderInputLayoutDesc& desc);
			~ShaderInputLayout();

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const Name& name) override { m_name = name; }
		};
	}
}