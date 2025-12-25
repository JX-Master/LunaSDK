/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShapeBuffer.hpp
* @author JXMaster
* @date 2024/7/2
*/
#pragma once
#include <Luna/RHI/Buffer.hpp>
#include <Luna/Runtime/Ref.hpp>

#ifndef LUNA_VG_API
#define LUNA_VG_API
#endif

namespace Luna
{
    namespace VG
    {
        //! @addtogroup VG
        //! @{
        
        //! @interface IShapeBuffer
        //! Represents one buffer that stores shape points. The user can record command points
        //! into the shape buffer then call @ref IShapeBuffer::build to build RHI buffer that stores
        //! shape points.
        struct IShapeBuffer : virtual Interface
        {
            luiid("fc6439a7-ca8f-45fd-aaaa-b753adb94767");

            //! Gets shape points that are recorded in the shape buffer.
            //! @param[in] modify Whether the user will modify shape points in the returned shape points vector.
            //! If this is `true`, the shape buffer will be marked as dirty and will be rebuilt in the next 
            //! @ref build call.
            //! @return Returns one vector that contains the recorded shape points.
            //! The user may add or remove points to this vector manually.
            virtual Vector<f32>& get_shape_points(bool modify = true) = 0;

            //! Builds RHI buffer from shape points.
            //! @param[in] device The device used to create the RHI buffer.
            //! @return Returns the built RHI buffer that contains the shape points. The returned RHI buffer
            //! is valid until next call to `build` of this shape buffer object.
            //! 
            //! The returned RHI buffer may be `nullptr` if this function is called and the shape buffer contains 
            //! no shape points.
            //! @remark The shape buffer keeps a strong reference to the returned RHI buffer, and skips
            //! unnecessary build operations if the shape data is not changed. The RHI buffer is also reused
            //! to hold new data if the new number of shape points is not greater than the former build.
            virtual R<RHI::IBuffer*> build(RHI::IDevice* device) = 0;
        };

        LUNA_VG_API Ref<IShapeBuffer> new_shape_buffer();

        //! @}
    }
}