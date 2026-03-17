/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Frontend.hpp
* @author JXMaster
* @date 2026/3/13
* @brief Frontend module public API: message helpers, error codes, and module entry points.
*/
#pragma once
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Name.hpp>
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Ref.hpp>

namespace Luna
{
    //! @addtogroup Frontend Frontend
    //! The Frontend module provides a stream-based API for message passing, remote procedure calls,
    //! Model Context Protocol (MCP), and command-based user actions (undo/redo).
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
        //! @details The handler receives the `params` field of the request message and returns
        //! either a result Variant on success, or an ErrCode on failure.
        //! The ErrCode will be converted to a Frontend error object by IFrontend.
        using FunctionHandler = Function<R<Variant>(IFrontend* frontend, const Variant& params)>;

        //! @interface IFrontend
        //! The main interface of the Frontend module.
        //! @details IFrontend owns one IResourceRegistry and exposes a synchronous message
        //! dispatch interface. Asynchronous use cases are handled by the user by building an
        //! external message queue and calling handle_message at the appropriate time.
        struct IFrontend : virtual Interface
        {
            luiid("{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}");

            //! Calls one registered function in this frontend.
            //! @param[in] url The URL identifying the function.
            //! @param[in] params The parameter to pass to the function.
            //! @return Returns the function calling result. If the call is a notification, returns 
            //! an empty object.
            virtual Variant invoke(const Name& url, const Variant& params) = 0;

            //! Sets a function resource at the given URL.
            //! @param[in] url The URL identifying the resource.
            //! @param[in] handler The callable handler for this function resource.
            //! @param[in] overwrite If `false` (default), returns @ref BasicError::already_exists
            //!            if a resource already exists at the URL.
            //!            If `true`, the existing resource is replaced.
            virtual RV set_resource_function(const Name& url, FunctionHandler&& handler, bool overwrite = false) = 0;

            //! Sets a Variant data resource at the given URL.
            //! @details This call serves as both register and modify: it creates the resource if it
            //! does not exist, or replaces it if `overwrite` is `true`.
            //! @param[in] url The URL identifying the resource.
            //! @param[in] data The Variant value to store.
            //! @param[in] overwrite If `false` (default), returns @ref BasicError::already_exists
            //!            if a resource already exists at the URL.
            virtual RV set_resource_data(const Name& url, Variant&& data, bool overwrite = false) = 0;

            //! Sets a userdata resource at the given URL.
            //! @param[in] url The URL identifying the resource.
            //! @param[in] data Pointer to the memory block.
            //! @param[in] dtor Optional destructor called when the resource is removed or overwritten.
            //! @param[in] overwrite If `false` (default), returns @ref BasicError::already_exists
            //!            if a resource already exists at the URL.
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
            //! @return Returns @ref FrontendError::resource_not_found if no resource exists,
            //!         or @ref FrontendError::type_mismatch if the resource is not a data resource.
            virtual R<Variant> get_resource_data(const Name& url) = 0;

            //! Removes the resource at the given URL.
            //! @param[in] url The resource to remove.
            virtual void remove_resource(const Name& url) = 0;
        };

        
        //! Creates and returns a new independent Frontend instance.
        //! @details The returned instance has its own resource registry and built-in functions,
        //! and is independent of the global instance.
        LUNA_FRONTEND_API Ref<IFrontend> new_frontend();

        // -----------------------------------------------------------------------
        // Message helpers
        // -----------------------------------------------------------------------

        //! Constructs a request message Variant.
        //! @param[in] method The URL of the function resource to call.
        //! @param[in] params The parameters for the function call. Must be an array, object, or null Variant.
        //! @return A Variant object representing the request message.
        LUNA_FRONTEND_API Variant make_request(const Name& method, const Variant& params);

        //! Constructs a notification message Variant (a request with a null id).
        //! @details Notification messages do not generate a response from the server.
        //! @param[in] method The URL of the function resource to call.
        //! @param[in] params The parameters for the function call.
        //! @return A Variant object representing the notification message.
        LUNA_FRONTEND_API Variant make_notification(const Name& method, const Variant& params);

        //! Constructs a successful response message Variant.
        //! @param[in] result The return value of the function call. Must not be a null Variant.
        //! @return A Variant object representing the response message.
        LUNA_FRONTEND_API Variant make_response(const Variant& result);

        //! Constructs an error response message Variant.
        //! @param[in] error An error object Variant, typically created with @ref make_frontend_error.
        //! @return A Variant object representing the error response message.
        LUNA_FRONTEND_API Variant make_error_response(const Variant& error);

        //! Constructs a Frontend error object Variant.
        //! @param[in] category The error category name (e.g., "FrontendError").
        //! @param[in] code The error code name (e.g., "method_not_found").
        //! @param[in] message Optional human-readable error description.
        //! @param[in] data Optional additional error data.
        //! @return A Variant object representing the error object.
        LUNA_FRONTEND_API Variant make_frontend_error(
            const Name& category,
            const Name& code,
            const Name& message = Name(),
            const Variant& data = Variant()
        );

        //! Returns the Frontend module pointer for use with @ref add_module.
        LUNA_FRONTEND_API Module* module_frontend();

        // -----------------------------------------------------------------------
        // Error codes
        // -----------------------------------------------------------------------

        //! @defgroup FrontendError Frontend Errors
        //! @{

        namespace FrontendError
        {
            //! Returns the Frontend error category.
            LUNA_FRONTEND_API errcat_t errtype();

            //! The requested resource URL was not found in the registry.
            LUNA_FRONTEND_API ErrCode resource_not_found();

            //! The resource exists but its type does not match the expected type.
            LUNA_FRONTEND_API ErrCode type_mismatch();

            //! The request message's `method` field refers to a resource that is not a function.
            LUNA_FRONTEND_API ErrCode method_not_found();
        }

        //! @}
        //! @}
    }
}
