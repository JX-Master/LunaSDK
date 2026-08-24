/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Frontend.hpp
* @author JXMaster
* @date 2026/3/13
* @brief Frontend module public API: resource registry, invocation, error codes, and module entry points.
*/
#pragma once
#include <Luna/Runtime/Functional.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Variant.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Name.hpp>
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Ref.hpp>
#include "Frontend.generated.hpp"

#ifndef LUNA_FRONTEND_API
#define LUNA_FRONTEND_API
#endif

namespace Luna
{
    //! @addtogroup Frontend Frontend
    //! The Frontend module provides a protocol-independent resource registry and synchronous
    //! Variant-based function invocation kernel.
    //! @{
    //! @}

    namespace Frontend
    {
        //! @addtogroup Frontend
        //! @{

        //! Defines the type of a resource stored in the resource registry.
        enum class ResourceType : u32
        {
            //! The resource does not exist.
            null     = 0,
            //! The resource is a callable function.
            function = 1,
            //! The resource is a Variant data node.
            data     = 2,
            //! The resource is an opaque user-managed memory block.
            userdata = 3,
        };

        struct IFrontend;

        //! The function handler type used to register callable resources.
        //! @details The handler receives an application-defined Variant and returns either a
        //! Variant on success or a ResultCode on failure. IFrontend propagates this result without
        //! adding a protocol-specific message envelope.
        using FunctionHandler = Function<R<Variant>(IFrontend* frontend, const Variant& params)>;

        //! @interface IFrontend
        //! The main interface of the Frontend module.
        //! @details Every IFrontend instance owns one independent resource registry. It does not
        //! parse or construct JSON-RPC messages, match request identifiers, process message
        //! batches, perform serialization, or provide a transport. Such behavior belongs to a
        //! protocol shell built on top of this interface.
        //!
        //! IFrontend is not thread-safe. The caller must synchronize all access to one instance.
        //! Function handlers may invoke other function resources and modify the registry,
        //! including removing or overwriting the function resource being invoked.
        struct [[Luna::interface("{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}")]] IFrontend : virtual Interface
        {
            //! Calls one registered function in this frontend.
            //! @param[in] url The URL identifying the function.
            //! @param[in] params The application-defined parameter Variant passed to the function.
            //! @return Returns the handler result without modification, or
            //! @ref Frontend::E_METHOD_NOT_FOUND if the URL does not identify a function.
            //! @details The handler is stored in a reference-counted boxed object. This call retains
            //! the box before invoking the handler, so the handler is not copied and remains valid
            //! if it removes or overwrites its own registry entry while executing.
            virtual R<Variant> invoke(const Name& url, const Variant& params) = 0;

            //! Sets a function resource at the given URL.
            //! @param[in] url The URL identifying the resource.
            //! @param[in] handler The callable handler for this function resource.
            //! @param[in] overwrite If `false` (default), returns @ref E_ALREADY_EXISTS
            //!            if a resource already exists at the URL.
            //!            If `true`, the existing resource is replaced.
            //! @return Returns @ref E_BAD_ARGUMENTS if `url` is empty or `handler` is
            //! invalid.
            virtual RV set_resource_function(const Name& url, FunctionHandler&& handler, bool overwrite = false) = 0;

            //! Sets a Variant data resource at the given URL.
            //! @details This call serves as both register and modify: it creates the resource if it
            //! does not exist, or replaces it if `overwrite` is `true`.
            //! @param[in] url The URL identifying the resource.
            //! @param[in] data The Variant value to store.
            //! @param[in] overwrite If `false` (default), returns @ref E_ALREADY_EXISTS
            //!            if a resource already exists at the URL.
            //! @return Returns @ref E_BAD_ARGUMENTS if `url` is empty.
            virtual RV set_resource_data(const Name& url, Variant&& data, bool overwrite = false) = 0;

            //! Sets a userdata resource at the given URL.
            //! @param[in] url The URL identifying the resource.
            //! @param[in] data Pointer to the memory block.
            //! @param[in] dtor Optional destructor called when the resource is removed or overwritten.
            //! @param[in] overwrite If `false` (default), returns @ref E_ALREADY_EXISTS
            //!            if a resource already exists at the URL.
            //! @return Returns @ref E_BAD_ARGUMENTS if `url` is empty.
            //! @details This IFrontend takes ownership of `data` only if this call succeeds. If a
            //! destructor is provided, it is also called when the IFrontend is destroyed.
            virtual RV set_resource_userdata(
                const Name& url,
                void* data,
                void (*dtor)(void*) = nullptr,
                bool overwrite = false
            ) = 0;

            //! Returns the type of the resource at the given URL.
            //! @return @ref ResourceType::null if no resource exists at the URL.
            virtual ResourceType get_resource_type(const Name& url) = 0;

            //! Returns the Variant data of the resource at the given URL.
            //! @return Returns @ref Frontend::E_RESOURCE_NOT_FOUND if no resource exists,
            //!         or @ref Frontend::E_TYPE_MISMATCH if the resource is not a data resource.
            virtual R<Variant> get_resource_data(const Name& url) = 0;

            //! Returns the pointer stored by a userdata resource at the given URL.
            //! @return Returns @ref Frontend::E_RESOURCE_NOT_FOUND if no resource exists,
            //! or @ref Frontend::E_TYPE_MISMATCH if the resource is not a userdata resource.
            //! @details The pointer remains owned by this IFrontend and may be invalidated by a
            //! subsequent resource removal, overwrite, or destruction of the IFrontend.
            virtual R<void*> get_resource_userdata(const Name& url) = 0;

            //! Removes the resource at the given URL.
            //! @param[in] url The resource to remove.
            //! @return Returns @ref E_BAD_ARGUMENTS if `url` is empty. Removing a URL
            //! that does not exist succeeds without effect.
            virtual RV remove_resource(const Name& url) = 0;
        };

        
        //! Creates and returns a new independent Frontend instance.
        //! @details The returned instance has its own empty resource registry.
        LUNA_FRONTEND_API Ref<IFrontend> new_frontend();

        //! Returns the Frontend module pointer for use with @ref add_module.
        LUNA_FRONTEND_API Module* module_frontend();

        // -----------------------------------------------------------------------
        // Error codes
        // -----------------------------------------------------------------------

        //! @defgroup FrontendResultCodes Frontend Result Codes
        //! @{

        //! The Frontend error category identifier.
        inline constexpr errcat_t ERROR_CATEGORY = make_error_category(ErrorDomain::LUNA_SDK, LunaErrorCategory::FRONTEND);

        //! The requested resource URL was not found in the registry.
        inline constexpr ResultCode E_RESOURCE_NOT_FOUND = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::FRONTEND, -1);

        //! The resource exists but its type does not match the expected type.
        inline constexpr ResultCode E_TYPE_MISMATCH = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::FRONTEND, -2);

        //! The requested URL does not identify a function resource.
        inline constexpr ResultCode E_METHOD_NOT_FOUND = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::FRONTEND, -3);

        //! @}
        //! @}
    }
}
