/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file QueryHeap.hpp
* @author JXMaster
* @date 2022/4/25
*/
#pragma once
#include "Device.hpp"

namespace Luna
{
	namespace RHI
	{
		struct QueryHeap : IQueryHeap
		{
			lustruct("RHI::QueryHeap", "{B744014B-48D0-417B-B7CC-F240CCBA59EE}");
			luiimpl();

			Ref<Device> m_device;
			Name m_name;
			VkQueryPool m_query_pool = VK_NULL_HANDLE;
			QueryHeapDesc m_desc;
			u32 m_num_statistic_items = 0;

			RV init(const QueryHeapDesc& desc);
			~QueryHeap();

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const c8* name) override { m_name = name; }
			virtual QueryHeapDesc get_desc() override { return m_desc; }
			virtual RV get_timestamp_values(u32 index, u32 count, u64* values) override;
			virtual RV get_occlusion_values(u32 index, u32 count, u64* values) override;
			virtual RV get_pipeline_statistics_values(u32 index, u32 count, PipelineStatistics* values) override;
		};
	}
}