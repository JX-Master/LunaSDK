/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ObjLoader.hpp
* @author JXMaster
* @date 2020/5/12
*/
#pragma once
#include <Luna/Runtime/Array.hpp>
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/Runtime/Result.hpp>

#ifndef LUNA_OBJ_LOADER_API
#define LUNA_OBJ_LOADER_API 
#endif

namespace Luna
{
    namespace ObjLoader
    {
        //! @addtogroup ObjLoader ObjLoader
        //! ObjLoader module provides functions to parse .obj file data.
        //! @{
        
        //! Specifies the index of vertex position, color, normal and texcoord data in `attributes` for one vertex.
        struct Index
        {
            //! The index of vertex position and color element to use for this vertex.
            i32 vertex_index;
            //! The index of normal to use for this vertex.
            //! -1 means not used.
            i32 normal_index;
            //! The index of texture coordinate element for this vertex.
            //! -1 means not used.
            i32 texcoord_index;

            bool operator==(const Index& rhs) const
            {
                return 
                    vertex_index == rhs.vertex_index && 
                    normal_index == rhs.normal_index && 
                    texcoord_index == rhs.texcoord_index;
            }
            bool operator!=(const Index& rhs) const
            {
                return !(*this == rhs);
            }
        };

        //! Describes the mesh data of one shape.
        struct Mesh
        {
            //! The indices of vertices of this mesh.
            Vector<Index> indices;
            //! The number of vertices per
            //! face. 3 = triangle, 4 = quad,
            //! ... Up to 255 vertices per face.
            Vector<u8> num_face_vertices;
            //! per-face material ID
            Vector<i32> material_ids;
            // per-face smoothing group
            // ID(0 = off. positive value
            // = group id)
            Vector<u32> smoothing_group_ids;
        };

        //! Describes the lines data of one shape.
        struct Lines
        {
            //! indices for vertices(polygon lines)
            Vector<Index> indices;
            //! The number of vertices per line.
            Vector<i32> num_line_vertices;
        };

        //! Describes the points data of one shape.
        struct Points
        {
            //! indices for points
            Vector<Index> indices;
        };

        //! Describes one shape in the obj file data.
        struct Shape
        {
            //! The name of the shape.
            Name name;
            //! The mesh part of the shape.
            Mesh mesh;
            //! The lines part of the shape.
            Lines lines;
            //! The points part of the shape.
            Points points;
        };

        //! Describes vertex attributes.
        struct Attributes
        {
            //! The vertex position array.
            Array<Float3U> vertices;
            //! The vertex normal array.
            Array<Float3U> normals;
            //! The vertex texture coordinates array.
            Array<Float2U> texcoords;
            //! The vertex color array.
            Array<Float3U> colors;
        };

        //! Describes one obj file data.
        struct ObjMesh
        {
            //! The vertex attributes.
            Attributes attributes;
            //! The shapes.
            Array<Shape> shapes;
        };

        //! Loads mesh from OBJ file data.
        //! @param[in] obj_file The object file (.obj) data.
        //! @param[in] mtl_file The material file (.mtl) data. This is optional.
        //! @return Returns the loaded mesh data.
        LUNA_OBJ_LOADER_API R<ObjMesh> load(Span<const byte_t> obj_file, Span<const byte_t> mtl_file = {});

        //! @}
    }

    struct Module;
    LUNA_OBJ_LOADER_API Module* module_obj_loader();
}