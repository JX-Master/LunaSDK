/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file PipelineState.hpp
* @author JXMaster
* @date 2019/8/15
*/
#pragma once
#ifdef LUNA_RHI_D3D12

#include "Device.hpp"
#include "ShaderInputLayout.hpp"
#include "../../PipelineState.hpp"

namespace Luna
{
	namespace RHI
	{
		struct PipelineState : IPipelineState
		{
			lustruct("RHI::D3D12::PipelineState", "{31F529FE-43C4-4DF1-842B-BAF52CCFCF3F}");
			luiimpl()

			Ref<Device> m_device;
			ComPtr<ID3D12PipelineState> m_pso;
			bool m_is_graphic;

			PipelineState(Device* dev) :
				m_device(dev) {}

			IDevice* get_device()
			{
				return m_device.as<IDevice>();
			}

			bool init_graphic(const GraphicPipelineStateDesc& desc);
			bool init_compute(const ComputePipelineStateDesc& desc);
		};
	}
}

#endif