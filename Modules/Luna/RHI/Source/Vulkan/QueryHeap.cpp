/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file QueryHeap.cpp
* @author JXMaster
* @date 2022/4/25
*/
#include "QueryHeap.hpp"

namespace Luna
{
	namespace RHI
	{
		RV QueryHeap::init(const QueryHeapDesc& desc)
		{
			lutry
			{
				m_desc = desc;
				VkQueryPoolCreateInfo create_info{};
				create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
				switch (desc.type)
				{
				case QueryType::occlusion: create_info.queryType = VK_QUERY_TYPE_OCCLUSION; break;
				case QueryType::timestamp: create_info.queryType = VK_QUERY_TYPE_TIMESTAMP; break;
				case QueryType::pipeline_statistics: create_info.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS; break;
				}
				create_info.queryCount = desc.count;
				if (desc.type == QueryType::pipeline_statistics)
				{
					create_info.pipelineStatistics =
						VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
						VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
						VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
						VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
						VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
						VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
						VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
					m_num_statistic_items = 7;
				}
				luexp(encode_vk_result(m_device->m_funcs.vkCreateQueryPool(m_device->m_device, &create_info, nullptr, &m_query_pool)));
			}
			lucatchret;
			return ok;
		}
		QueryHeap::~QueryHeap()
		{
			if (m_query_pool != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkDestroyQueryPool(m_device->m_device, m_query_pool, nullptr);
				m_query_pool = VK_NULL_HANDLE;
			}
		}
		RV QueryHeap::get_timestamp_values(u32 index, u32 count, u64* values)
		{
			if (m_desc.type != QueryType::timestamp) return BasicError::not_supported();
			return encode_vk_result(m_device->m_funcs.vkGetQueryPoolResults(
				m_device->m_device, m_query_pool, index, count, sizeof(u64) * count, values,
				sizeof(u64), VK_QUERY_RESULT_64_BIT));
		}
		RV QueryHeap::get_occlusion_values(u32 index, u32 count, u64* values)
		{
			if (m_desc.type != QueryType::occlusion) return BasicError::not_supported();
			return encode_vk_result(m_device->m_funcs.vkGetQueryPoolResults(
				m_device->m_device, m_query_pool, index, count, sizeof(u64) * count, values,
				sizeof(u64), VK_QUERY_RESULT_64_BIT));
		}
		RV QueryHeap::get_pipeline_statistics_values(u32 index, u32 count, PipelineStatistics* values)
		{
			if (m_desc.type != QueryType::pipeline_statistics) return BasicError::not_supported();
			return encode_vk_result(m_device->m_funcs.vkGetQueryPoolResults(
				m_device->m_device, m_query_pool, index, count, sizeof(PipelineStatistics) * count, values,
				sizeof(PipelineStatistics), VK_QUERY_RESULT_64_BIT));
		}
	}
}