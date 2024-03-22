/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Diff.hpp
* @author JXMaster
* @date 2022/6/26
*/
#pragma once
#include <Luna/Runtime/Variant.hpp>

#ifndef LUNA_VARIANT_UTILS_API
#define LUNA_VARIANT_UTILS_API
#endif

namespace Luna
{
    namespace VariantUtils
    {
        //! @addtogroup VariantUtils
        //! @{

        //! Creates one delta variant that stores changes from `before` to `after`.
        //! @param[in] before The first variant to compare.
        //! @param[in] after The second variant to compare.
        //! @return Return one delta variant that stores changes from `before` to `after`.
        LUNA_VARIANT_UTILS_API Variant diff(const Variant& before, const Variant& after);

        //! Applys the difference to the variant, so that it contains the same data as `after` when the diff object
        //! is created.
        //! @param[in] before The variant to patch.
        //! @param[in] delta The delta variant returned by @ref diff.
        //! @return Returns the patched variant object.
        LUNA_VARIANT_UTILS_API void patch(Variant& before, const Variant& delta);

        //! Reverts the difference made in `after`, so that it contains the same data as `before` when the diff
        //! object is created.
        //! @param[in] after The variant to revert.
        //! @param[in] delta The delta variant returned by @ref diff.
        //! @return Returns the reverted variant object.
        LUNA_VARIANT_UTILS_API void revert(Variant& after, const Variant& delta);

        //! Adds prefix nodes to the delta object.
        //! @details This can be used if the delta object is computed from a child variant, but the user wants to patch the delta object to a 
        //! parent variant. See remarks for details.
        //! @param[in] delta The delta object to modify.
        //! @param[in] prefix_nodes The prefix nodes to add to the delta object.
        //! @remark For example, if we have the following variant:
        //! ```json
        //! {
        //!     "rootRegion":
        //!     {
        //!         "members":
        //!         [
        //!             {
        //!                 "row":2
        //!             }
        //!         ]
        //!     }
        //! }
        //! ```
        //! and we have the following delta:
        //! ```json
        //! {
        //!     "row": [2,3]
        //! }
        //! ```
        //! after adding prefix nodes `{ "rootRegion", "members", (u64)0 }` to the delta, the result delta object will be:
        //! ```json
        //! {
        //!     "rootRegion": {
        //!         "members": {
        //!             "_t": "a",
        //!             "0": {
        //!                 "row": [2,3]
        //!             }
        //!         }
        //!     }
        //! }
        //! ```
        //! which can be used to change the value of ["rootRegion"]["members"][0]["rows"] from 2 to 3. 
        LUNA_VARIANT_UTILS_API void add_diff_prefix(Variant& delta, Span<const Variant> prefix_nodes);

        //! @}
    }
}