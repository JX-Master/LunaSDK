/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Style.hpp
* @author JXMaster
* @date 2026/6/17
*/
#pragma once
#include "Base.hpp"

namespace Luna
{
    namespace GUICore
    {
        //! Identifies the payload type of one style entry value.
        enum class StyleValueType : u8
        {
            //! No value.
            none,
            //! One 32-bit floating point value.
            f32,
            //! Two 32-bit floating point values.
            f32x2,
            //! Three 32-bit floating point values.
            f32x3,
            //! Four 32-bit floating point values.
            f32x4,
            //! One name value.
            name
        };

        //! Stores one resolved style value.
        struct StyleValue
        {
            //! The stored value type.
            StyleValueType type = StyleValueType::none;
            //! Numeric value storage. The first N components are used by f32/f32x2/f32x3/f32x4 entries.
            Float4U number = Float4U(0.0f);
            //! Name value storage for @ref StyleValueType::name.
            Name name;
        };

        //! Identifies how a local style entry participates in inheritance.
        enum class StyleEntryMode : u8
        {
            //! This entry inherits from its parent.
            inherit,
            //! This entry provides a local value.
            set,
            //! This entry explicitly hides an inherited value.
            unset
        };

        //! One style entry record.
        struct StyleEntry
        {
            //! Entry inheritance mode.
            StyleEntryMode mode = StyleEntryMode::inherit;
            //! Entry value used when @ref mode is @ref StyleEntryMode::set.
            StyleValue value;
        };

        //! One named style record.
        struct Style
        {
            //! Optional parent style.
            Name parent;
            //! Local style entries.
            HashMap<Name, StyleEntry> entries;
        };

        //! Describes one style entry that may be consumed by a high-level immediate API package.
        //! @remark GUI Core stores this metadata for tools and debug views only. It does not interpret the entry
        //! name as widget behavior.
        struct StyleEntrySchema
        {
            //! Package or style family that declares this entry, such as `gui.editor`.
            Name owner;
            //! Style entry name.
            Name entry;
            //! Expected style value type.
            StyleValueType type = StyleValueType::none;
            //! Fallback value used by the declaring package when no style value is found.
            StyleValue default_value;
            //! Optional inspector/debug category.
            String category;
            //! Optional human-readable description.
            String description;
        };

        //! Creates one f32 style value.
        //! @param[in] value The value.
        //! @return Returns the style value.
        inline StyleValue style_f32(f32 value)
        {
            StyleValue r;
            r.type = StyleValueType::f32;
            r.number.x = value;
            return r;
        }

        //! Creates one f32x2 style value.
        //! @param[in] value The value.
        //! @return Returns the style value.
        inline StyleValue style_f32x2(const Float2U& value)
        {
            StyleValue r;
            r.type = StyleValueType::f32x2;
            r.number.x = value.x;
            r.number.y = value.y;
            return r;
        }

        //! Creates one f32x3 style value.
        //! @param[in] value The value.
        //! @return Returns the style value.
        inline StyleValue style_f32x3(const Float3U& value)
        {
            StyleValue r;
            r.type = StyleValueType::f32x3;
            r.number.x = value.x;
            r.number.y = value.y;
            r.number.z = value.z;
            return r;
        }

        //! Creates one f32x4 style value.
        //! @param[in] value The value.
        //! @return Returns the style value.
        inline StyleValue style_f32x4(const Float4U& value)
        {
            StyleValue r;
            r.type = StyleValueType::f32x4;
            r.number = value;
            return r;
        }

        //! Creates one name style value.
        //! @param[in] value The value.
        //! @return Returns the style value.
        inline StyleValue style_name(const Name& value)
        {
            StyleValue r;
            r.type = StyleValueType::name;
            r.name = value;
            return r;
        }
    }
}
