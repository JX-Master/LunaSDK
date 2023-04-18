// Copyright 2018-2022 JXMaster. All rights reserved.
/*
* @file CommandQueue.hpp
* @author JXMaster
* @date 2022/10/29
*/
#pragma once
#include "Common.hpp"
#include "Device.hpp"

namespace Luna
{
	namespace RHI
	{
		struct CommandQueue : ICommandQueue
		{
			lustruct("RHI::CommandQueue", "{47F84AC7-CD6D-44F0-9A75-E85EDFBF633A}");

			Ref<Device> m_device;
			Name m_name;
			VkQueue m_queue;
			CommandQueueDesc m_desc;
			RV init(const CommandQueueDesc& desc);

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const Name& name) override { m_name = name; }
			virtual CommandQueueDesc get_desc() override { return m_desc; }

		};
	}
}