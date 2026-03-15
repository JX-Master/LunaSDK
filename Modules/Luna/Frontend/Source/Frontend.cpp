/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Frontend.cpp
* @author JXMaster
* @date 2026/3/13
* @brief Frontend implementation: IFrontend, built-in functions, message helpers,
*        error codes, and module registration.
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_FRONTEND_API LUNA_EXPORT
#include "Frontend.hpp"
#include "../Frontend.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Memory.hpp>

namespace Luna
{
    namespace Frontend
    {
        // -----------------------------------------------------------------------
        // Built-in function URLs
        // -----------------------------------------------------------------------

        static const Path k_builtin_get_type("/__builtin__/get_type");
        static const Path k_builtin_get     ("/__builtin__/get");
        static const Path k_builtin_set     ("/__builtin__/set");
        static const Path k_builtin_delete  ("/__builtin__/delete");

        // -----------------------------------------------------------------------
        // Frontend::init
        // -----------------------------------------------------------------------

        void Frontend::init()
        {
            register_builtin_functions();
        }

        // -----------------------------------------------------------------------
        // Built-in functions
        // -----------------------------------------------------------------------

        static R<Variant> builtin_get_type(IFrontend* frontend, const Variant& params, const Variant& id)
        {
            auto& url_v = params["url"];
            if (!url_v.valid() || url_v.type() != VariantType::string)
            {
                return make_frontend_error(
                    Name("FrontendError"), Name("invalid_params"),
                    Name("'url' parameter is required and must be a string"));
            }
            Path url(url_v.c_str());
            ResourceType t = frontend->get_resource_type(url);
            const c8* type_str = "null";
            switch (t)
            {
            case ResourceType::function: type_str = "function"; break;
            case ResourceType::data:     type_str = "data";     break;
            case ResourceType::userdata: type_str = "userdata"; break;
            default: break;
            }
            return Variant(Name(type_str));
        }

        static R<Variant> builtin_get(IFrontend* frontend, const Variant& params, const Variant& id)
        {
            auto& url_v = params["url"];
            if (!url_v.valid() || url_v.type() != VariantType::string)
            {
                return make_frontend_error(
                    Name("FrontendError"), Name("invalid_params"),
                    Name("'url' parameter is required and must be a string"));
            }
            Path url(url_v.c_str());
            return frontend->get_data(url);
        }

        static R<Variant> builtin_set(IFrontend* frontend, const Variant& params, const Variant& id)
        {
            auto& url_v  = params["url"];
            Variant data_v = params["data"];
            if (!url_v.valid() || url_v.type() != VariantType::string)
            {
                return make_frontend_error(
                    Name("FrontendError"), Name("invalid_params"),
                    Name("'url' parameter is required and must be a string"));
            }
            Path url(url_v.c_str());
            bool overwrite = false;
            auto& ow_v = params["overwrite"];
            if (ow_v.valid() && ow_v.type() == VariantType::boolean)
            {
                overwrite = ow_v.boolean();
            }
            lutry
            {
                luexp(frontend->set_data(url, move(data_v), overwrite));
            }
            lucatchret;
            return Variant(true);
        }

        static R<Variant> builtin_delete(IFrontend* frontend, const Variant& params, const Variant& id)
        {
            auto& url_v = params["url"];
            if (!url_v.valid() || url_v.type() != VariantType::string)
            {
                return make_frontend_error(
                    Name("FrontendError"), Name("invalid_params"),
                    Name("'url' parameter is required and must be a string"));
            }
            Path url(url_v.c_str());
            frontend->remove_resource(url);
            return Variant(true);
        }

        void Frontend::register_builtin_functions()
        {
            // /__builtin__/get_type
            // params: { "url": string }
            // returns: string ("function" | "data" | "userdata" | "null")
            lupanic_if_failed(set_function(k_builtin_get_type, builtin_get_type, true));

            // /__builtin__/get
            // params: { "url": string }
            // returns: Variant data stored at url
            lupanic_if_failed(set_function(k_builtin_get, builtin_get, true));

            // /__builtin__/set
            // params: { "url": string, "data": Variant, "overwrite": bool (optional, default false) }
            // returns: true on success
            lupanic_if_failed(set_function(k_builtin_set, builtin_set, true));

            // /__builtin__/delete
            // params: { "url": string }
            // returns: true on success
            lupanic_if_failed(set_function(k_builtin_delete, builtin_delete, true));
        }

        RV Frontend::set_function(const Path& url, FunctionHandler&& handler, bool overwrite)
        {
            auto res = m_registry.emplace(make_pair(url, ResourceEntry()));
            if(!res.second && !overwrite) return BasicError::already_exists();
            auto& iter = res.first;
            iter->second.type = ResourceType::function;
            iter->second.function = move(handler);
            return ok;
        }

        RV Frontend::set_data(const Path& url, Variant&& data, bool overwrite)
        {
            auto res = m_registry.emplace(make_pair(url, ResourceEntry()));
            if(!res.second && !overwrite) return BasicError::already_exists();
            auto& iter = res.first;
            iter->second.type = ResourceType::data;
            iter->second.data = move(data);
            return ok;
        }

        RV Frontend::set_userdata(
            const Path& url,
            void* data,
            void (*dtor)(void*),
            bool overwrite)
        {
            auto res = m_registry.emplace(make_pair(url, ResourceEntry()));
            if(!res.second && !overwrite) return BasicError::already_exists();
            auto& iter = res.first;
            iter->second.type = ResourceType::userdata;
            iter->second.userdata_ptr = data;
            iter->second.userdata_dtor = dtor;
            return ok;
        }

        ResourceType Frontend::get_resource_type(const Path& url)
        {
            auto iter = m_registry.find(url);
            if (iter == m_registry.end())
            {
                return ResourceType::null;
            }
            return iter->second.type;
        }

        R<Variant> Frontend::get_data(const Path& url)
        {
            auto iter = m_registry.find(url);
            if (iter == m_registry.end())
            {
                return FrontendError::resource_not_found();
            }
            if (iter->second.type != ResourceType::data)
            {
                return FrontendError::type_mismatch();
            }
            return iter->second.data;
        }

        void Frontend::remove_resource(const Path& url)
        {
            m_registry.erase(url);
        }

        Variant Frontend::invoke(const Path& url, const Variant& params, const Variant& id)
        {
            auto result = invoke_impl(url, params, id);

            // Null or absent id -> notification, discard response.
            if (!id.valid())
            {
                return Variant();
            }

            if (failed(result))
            {
                ErrCode code = result.errcode();
                Error err;
                if(code == BasicError::error_object())
                {
                    err = get_error();
                }
                else
                {
                    err.code = code;
                }
                Name cat_name  = get_error_category_name(get_error_code_category(err.code));
                Name code_name = get_error_code_name(err.code);
                Name message = err.message.c_str();
                return make_error_response(id,
                    make_frontend_error(cat_name, code_name, message, err.info));
            }
            return make_response(id, result.get());
        }

        R<Variant> Frontend::invoke_impl(const Path& url, const Variant& params, const Variant& id)
        {
            auto iter = m_registry.find(url);
            if (iter == m_registry.end() || iter->second.type != ResourceType::function)
            {
                return FrontendError::method_not_found();
            }
            return iter->second.function(this, params, id);
        }

        // -----------------------------------------------------------------------
        // Message helpers
        // -----------------------------------------------------------------------

        LUNA_FRONTEND_API Variant make_request(const Name& method, const Variant& params, Variant id)
        {
            Variant msg(VariantType::object);
            msg["method"] = Variant(method);
            msg["params"] = move(params);
            msg["id"]     = move(id);
            return msg;
        }

        LUNA_FRONTEND_API Variant make_notification(const Name& method, const Variant& params)
        {
            Variant msg(VariantType::object);
            msg["method"] = Variant(method);
            msg["params"] = move(params);
            msg["id"]     = Variant();
            return msg;
        }

        LUNA_FRONTEND_API Variant make_response(const Variant& id, const Variant& result)
        {
            Variant msg(VariantType::object);
            msg["id"]     = move(id);
            msg["result"] = move(result);
            msg["error"]  = Variant();
            return msg;
        }

        LUNA_FRONTEND_API Variant make_error_response(const Variant& id, const Variant& error)
        {
            Variant msg(VariantType::object);
            msg["id"]     = move(id);
            msg["result"] = Variant();
            msg["error"]  = move(error);
            return msg;
        }

        LUNA_FRONTEND_API Variant make_frontend_error(
            const Name& category,
            const Name& code,
            const Name& message,
            const Variant& data)
        {
            Variant err(VariantType::object);
            err["category"] = Variant(category);
            err["code"]     = Variant(code);
            if (message.size() > 0)
            {
                err["message"] = Variant(message);
            }
            if (data.type() != VariantType::null)
            {
                err["data"] = move(data);
            }
            return err;
        }

        LUNA_FRONTEND_API Ref<IFrontend> new_frontend()
        {
            Ref<Frontend> o = new_object<Frontend>();
            o->init();
            return Ref<IFrontend>(o);
        }

        // -----------------------------------------------------------------------
        // Error codes
        // -----------------------------------------------------------------------

        namespace FrontendError
        {
            LUNA_FRONTEND_API errcat_t errtype()
            {
                static errcat_t e = get_error_category_by_name("FrontendError");
                return e;
            }

            LUNA_FRONTEND_API ErrCode resource_not_found()
            {
                static ErrCode e = get_error_code_by_name("FrontendError", "resource_not_found");
                return e;
            }

            LUNA_FRONTEND_API ErrCode type_mismatch()
            {
                static ErrCode e = get_error_code_by_name("FrontendError", "type_mismatch");
                return e;
            }

            LUNA_FRONTEND_API ErrCode method_not_found()
            {
                static ErrCode e = get_error_code_by_name("FrontendError", "method_not_found");
                return e;
            }
        }

        // -----------------------------------------------------------------------
        // Module
        // -----------------------------------------------------------------------

        struct FrontendModule : public Module
        {
            virtual const c8* get_name() override { return "Frontend"; }

            virtual RV on_register() override
            {
                register_boxed_type<Frontend>();
                impl_interface_for_type<Frontend, IFrontend>();
                return ok;
            }
        };

        LUNA_FRONTEND_API Module* module_frontend()
        {
            static FrontendModule m;
            return &m;
        }
    }
}
