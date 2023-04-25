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
        enum class QueryType : u8
        {
            occlusion,
            timestamp,
            pipeline_statistics,
        };

        enum class QueryPipelineStatisticFlag : u32
        {
            //! Number of vertices read by input assembler.
            input_vertices = 0x01,
            //! Number of primitives read by the input assembler.
            input_primitives = 0x02,
            //! Number of vertex shader invocations.
            vs_invocations = 0x04,
            //! Number of geometry shader invocations.
            gs_invocations = 0x08,
            //! Number of geometry shader output primitives.
            gs_output_primitives = 0x10,
            //! Number of primitives that were sent to the rasterizer.
            rasterizer_input_primitives = 0x20,
            //! Number of primitives that were rendered.
            rendered_primitives = 0x40,
            //! Number of pixel shader invocations.
            ps_invocations = 0x80,
            //! Number of hull shader invocations.
            hs_invocations = 0x100,
            //! Number of domain shader invocations.
            ds_invocations = 0x200,
            //! Number of compute shader invocations.
            cs_invocations = 0x400
        };

        struct QueryHeapDesc
        {
            //! The type of the query heap.
            QueryType type;
            //! Number of queries this heap contains.
            u32 count;
            //! If type is `QueryType::pipeline_statistics`, specify the pipeline statistic entry
            //! you want to query. Otherwise, this is ignored.
            QueryPipelineStatisticFlag pipeline_statistics;
        };

        struct IQueryHeap : virtual IDeviceChild
        {
            luiid("{11c98a1e-1fd4-48c7-828b-96c56239e6ca}");

            virtual QueryHeapDesc get_desc() = 0;

            //! Copies query results from query heap to the user-provided buffer.
            //! @param[in] index The index of the first query to copy.
            //! @param[in] count The number of queries to copy.
            //! @param[out] buffer The user-provided buffer used to store the results.
            //! @param[in] buffer_size The size of `buffer` in bytes.
            //! @param[in] stride The stride in bytes between results for individual queries within `buffer`
            //! @remark The user must ensure that all queries being copied are initialized, or the behavior is undefined.
            //! 
            //! For occlusion and timestamp queries, the result is represented by one `u64` value for each query. For pipeline 
            //! statistics querys, the result is represented by `N` number of `u64` values for each query, where `N` is the number
            //! of pipeline stages enabled by `QueryPipelineStatisticFlag` when creating the query heap. Pipeline statistics query
            //! stage results are arranged using the same order as they declared in `QueryPipelineStatisticFlag`.
            virtual RV get_query_results(u32 start_index, u32 count, void* buffer, usize buffer_size, usize stride) = 0;
        };
    }
}