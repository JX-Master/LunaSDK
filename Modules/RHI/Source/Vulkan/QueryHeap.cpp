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
					create_info.pipelineStatistics = 0;
					m_num_statistic_items = 0;
					if (test_flags(desc.pipeline_statistics, QueryPipelineStatisticFlag::input_vertices))
					{
						create_info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT;
						++m_num_statistic_items;
					}
					if (test_flags(desc.pipeline_statistics, QueryPipelineStatisticFlag::input_primitives))
					{
						create_info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT;
						++m_num_statistic_items;
					}
					if (test_flags(desc.pipeline_statistics, QueryPipelineStatisticFlag::vs_invocations))
					{
						create_info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT;
						++m_num_statistic_items;
					}
					if (test_flags(desc.pipeline_statistics, QueryPipelineStatisticFlag::rasterizer_input_primitives))
					{
						create_info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT;
						++m_num_statistic_items;
					}
					if (test_flags(desc.pipeline_statistics, QueryPipelineStatisticFlag::rendered_primitives))
					{
						create_info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT;
						++m_num_statistic_items;
					}
					if (test_flags(desc.pipeline_statistics, QueryPipelineStatisticFlag::ps_invocations))
					{
						create_info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;
						++m_num_statistic_items;
					}
					if (test_flags(desc.pipeline_statistics, QueryPipelineStatisticFlag::cs_invocations))
					{
						create_info.pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
						++m_num_statistic_items;
					}
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
		RV QueryHeap::get_query_results(u32 start_index, u32 count, void* buffer, usize buffer_size, usize stride)
		{
			return encode_vk_result(m_device->m_funcs.vkGetQueryPoolResults(
				m_device->m_device, m_query_pool, start_index, count, buffer_size, buffer,
				stride, VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));
		}
	}
}