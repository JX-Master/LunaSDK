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
		struct CommandQueue
		{
			lustruct("RHI::CommandQueue", "{47F84AC7-CD6D-44F0-9A75-E85EDFBF633A}");

			Ref<Device> m_device;
			VkQueue m_queue;

			RV init(CommandQueueType type);
		};
	}
}