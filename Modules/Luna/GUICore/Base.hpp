/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Base.hpp
* @author JXMaster
* @date 2026/6/17
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Span.hpp>
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Runtime/Name.hpp>
#include <Luna/Runtime/Hash.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/Runtime/Math/Math.hpp>
#include <Luna/Font/Font.hpp>

#ifndef LUNA_GUICORE_API
#define LUNA_GUICORE_API
#endif

namespace Luna
{
    //! @addtogroup GUICore GUI Core
    //! @{
    namespace GUICore
    {
        //! Stable identifier type used by GUI Core elements, layers and state objects.
        using id_t = u64;

        //! The invalid element index value.
        constexpr u32 INVALID_ELEMENT = U32_MAX;

        //! The invalid layer index value.
        constexpr u32 INVALID_LAYER = U32_MAX;

        //! Default data scope used when no explicit scope has been pushed.
        constexpr id_t DEFAULT_DATA_SCOPE = 14695981039346656037ull;

        //! Mixes a numeric local ID into a data scope.
        //! @param[in] scope The parent data scope. Passing zero uses @ref DEFAULT_DATA_SCOPE.
        //! @param[in] local_id The local ID inside the scope.
        //! @return Returns the stable ID for the scoped item.
        inline id_t make_scoped_id(id_t scope, id_t local_id)
        {
            id_t seed = scope ? scope : DEFAULT_DATA_SCOPE;
            id_t ret = memhash64(&local_id, sizeof(local_id), seed);
            return ret ? ret : DEFAULT_DATA_SCOPE;
        }

        //! Mixes a string local ID into a data scope.
        //! @param[in] scope The parent data scope. Passing zero uses @ref DEFAULT_DATA_SCOPE.
        //! @param[in] local_name The local string ID inside the scope.
        //! @return Returns the stable ID for the scoped item.
        inline id_t make_scoped_id(id_t scope, const c8* local_name)
        {
            id_t seed = scope ? scope : DEFAULT_DATA_SCOPE;
            id_t ret = strhash<id_t>(local_name ? local_name : "", seed);
            return ret ? ret : DEFAULT_DATA_SCOPE;
        }

        //! Describes one GUI Core frame.
        struct FrameDesc
        {
            //! The logical GUI surface size used by layout and input positions.
            //! @remark A window host normally treats this as screen size. A world-space host treats it as the
            //! logical dimensions of one projected GUI surface.
            Float2U screen_size = Float2U(0.0f);
            //! The render target size in physical pixels.
            UInt2U framebuffer_size = UInt2U(0, 0);
            //! The host DPI scale for this frame.
            f32 dpi_scale = 1.0f;
            //! The elapsed time since the previous frame, in seconds.
            f32 delta_time = 1.0f / 60.0f;
        };

        //! Describes one font registered in a GUI Core context.
        struct FontDesc
        {
            //! The font file object. The context keeps a reference to registered font files.
            Font::IFontFile* font = nullptr;
            //! The font face index inside @ref font.
            u32 font_index = 0;
        };

        //! Controls automatic state cleanup in a GUI Core context.
        enum class StateLifetime : u8
        {
            //! Clears the state at the next @ref IContext::begin_frame call.
            current_frame,
            //! Clears the state if it is not refreshed for the next frame.
            next_frame,
            //! Keeps the state until @ref IContext::clear_state is called or the owning context is destroyed.
            context,
            //! Reserves persistent storage semantics for future implementation.
            persistent
        };

        //! Hash functor for GUI Core stable IDs.
        struct IdHash
        {
            //! Hashes one GUI Core ID.
            //! @param[in] value The value to hash.
            //! @return Returns the hash value.
            usize operator()(id_t value) const
            {
                return hash<u64>()(value);
            }
        };

        //! Builds a stable state identifier from an owner ID and a state object type GUID.
        //! @param[in] owner_id The element, layer or subsystem ID that owns the state.
        //! @param[in] state_type The GUID of the state object type.
        //! @return Returns the generated state identifier.
        LUNA_GUICORE_API id_t make_state_id(id_t owner_id, const Guid& state_type);

        //! Builds a stable state identifier from an owner ID and a boxed state object type.
        //! @param[in] owner_id The element, layer or subsystem ID that owns the state.
        //! @return Returns the generated state identifier.
        template <typename T>
        id_t make_state_id(id_t owner_id)
        {
            return make_state_id(owner_id, Meta::StructMetaData<T>::__guid);
        }
    }
    //! @}
}
