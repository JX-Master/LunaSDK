/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file QueryHeap.hpp
* @author JXMaster
* @date 2023/3/8
*/
#pragma once
#include "DeviceChild.hpp"

namespace Luna
{
    namespace RHI
    {
        //! @addtogroup RHI
        //! @{
        
        //! Specifies query type.
        enum class QueryType : u8
        {
            //! Specifies occlusion query.
            occlusion,
            //! Specifies timestamp query that works for non-copy queue.
            timestamp,
            //! Specifies timestamp query that works for copy queue.
            timestamp_copy_queue,
            //! Specifies pipeline statistics query.
            pipeline_statistics,
        };

        //! Specifies one query in pipeline statistics query heap.
        struct PipelineStatistics
        {
            //! Number of vertex shader invocations.
            u64 vs_invocations;
            //! Number of primitives that were sent to the rasterizer.
            u64 rasterizer_input_primitives;
            //! Number of primitives that were rendered.
            u64 rendered_primitives;
            //! Number of pixel shader invocations.
            u64 ps_invocations;
            //! Number of compute shader invocations.
            u64 cs_invocations;
        };

        //! Describes one query heap.
        struct QueryHeapDesc
        {
            //! The type of the query heap.
            QueryType type;
            //! Number of queries this heap contains.
            u32 count;
            
            QueryHeapDesc() = default;
            QueryHeapDesc(QueryType type, u32 count) :
                type(type),
                count(count) {}
        };

        //! @interface IQueryHeap
        //! Contains one array of query elements that can be used to query pipeline execution information, such
        //! as pixel occlusion, execution time and pipeline statistics.
        struct IQueryHeap : virtual IDeviceChild
        {
            luiid("{11c98a1e-1fd4-48c7-828b-96c56239e6ca}");

            //! Gets the descriptor of the query heap.
            virtual QueryHeapDesc get_desc() = 0;

            //! Copies timestamp query results from query heap to the user-provided buffer.
            //! @param[in] index The index of the first query to copy.
            //! @param[in] count The number of queries to copy.
            //! @param[out] values The user-provided buffer used to store the results.
            //! @remark The user must ensure that all queries being copied are initialized, or the behavior is undefined.
            //! If this query heap is not `QueryHeapType::timestamp`, this function fails with `BasicError::not_supported`.
            virtual RV get_timestamp_values(u32 index, u32 count, u64* values) = 0;

            //! Copies occlusion query results from query heap to the user-provided buffer.
            //! @param[in] index The index of the first query to copy.
            //! @param[in] count The number of queries to copy.
            //! @param[out] values The user-provided buffer used to store the results.
            //! @remark The user must ensure that all queries being copied are initialized, or the behavior is undefined.
            //! If this query heap is not `QueryHeapType::occlusion`, this function fails with `BasicError::not_supported`.
            virtual RV get_occlusion_values(u32 index, u32 count, u64* values) = 0;

            //! Copies pipeline statistics query results from query heap to the user-provided buffer.
            //! @param[in] index The index of the first query to copy.
            //! @param[in] count The number of queries to copy.
            //! @param[out] values The user-provided buffer used to store the results.
            //! @remark The user must ensure that all queries being copied are initialized, or the behavior is undefined.
            //! If this query heap is not `QueryHeapType::pipeline_statistics`, this function fails with `BasicError::not_supported`.
            virtual RV get_pipeline_statistics_values(u32 index, u32 count, PipelineStatistics* values) = 0;
        };

        //! @}
    }
}
