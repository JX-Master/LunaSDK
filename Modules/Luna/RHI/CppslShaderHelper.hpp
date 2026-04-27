/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file CppslShaderHelper.hpp
* @author JXMaster
* @date 2026/4/27
*/
#pragma once
#include "RHI.hpp"

namespace Luna
{
    namespace RHI
    {
        //! @addtogroup RHI
        //! @{

        //! Gets one @ref ShaderData structure that refers to generated CPPSL shader data.
        //! @param[in] data The generated shader data.
        //! @param[in] data_size The generated shader data size.
        //! @param[in] entry_point The shader entry point.
        //! @param[in] format The shader data format.
        //! @return Returns one @ref ShaderData structure that refers to the specified generated shader data.
        inline ShaderData get_cppsl_shader_data(const byte_t* data, usize data_size, const c8* entry_point, ShaderDataFormat format)
        {
            return ShaderData(Span<const byte_t>(data, data_size), Name(entry_point), format);
        }

        //! @}
    }
}

#define LUNA_CPPSL_GET_SHADER_DATA(_shader) Luna::RHI::get_cppsl_shader_data((const Luna::byte_t*)SHADER_DATA_##_shader, SHADER_DATA_SIZE_##_shader, SHADER_ENTRY_POINT_##_shader, SHADER_DATA_FORMAT_##_shader)
#define LUNA_CPPSL_FILL_COMPUTE_SHADER_DATA(_desc, _shader) {_desc.cs = LUNA_CPPSL_GET_SHADER_DATA(_shader); \
_desc.metal_numthreads_x = SHADER_METAL_NUMTHREADS_X_##_shader; \
_desc.metal_numthreads_y = SHADER_METAL_NUMTHREADS_Y_##_shader; \
_desc.metal_numthreads_z = SHADER_METAL_NUMTHREADS_Z_##_shader; }
