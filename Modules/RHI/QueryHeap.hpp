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
        enum class QueryHeapType : u8
        {
            occlusion,
            timestamp,
            pipeline_statistics,
        };

        struct QueryHeapDesc
        {
            //! The type of the query heap.
            QueryHeapType type;
            //! Number of queries this heap contains.
            u32 count;
        };

        struct PipelineStatistics
        {
            //! Number of vertices read by input assembler.
            u64 input_vertices;
            //! Number of primitives read by the input assembler.
            u64 input_primitives;
            //! Number of vertex shader invocations.
            u64 vs_invocations;
            //! Number of geometry shader invocations.
            u64 gs_invocations;
            //! Number of geometry shader output primitives.
            u64 gs_output_primitives;
            //! Number of primitives that were sent to the rasterizer.
            u64 rasterizer_input_primitives;
            //! Number of primitives that were rendered.
            u64 rendered_primitives;
            //! Number of pixel shader invocations.
            u64 ps_invocations;
            //! Number of hull shader invocations.
            u64 hs_invocations;
            //! Number of domain shader invocations.
            u64 ds_invocations;
            //! Number of compute shader invocations.
            u64 cs_invocations;
        };

        struct IQueryHeap : virtual IDeviceChild
        {
            luiid("{11c98a1e-1fd4-48c7-828b-96c56239e6ca}");

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
    }
}