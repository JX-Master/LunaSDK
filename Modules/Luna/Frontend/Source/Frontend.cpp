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
#include "FrontendImpl.hpp"
#include "Frontend.meta.generated.hpp"
#include "../Frontend.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Memory.hpp>

namespace Luna
{
    namespace Frontend
    {
        RV Frontend::set_resource_function(const Name& url, FunctionHandler&& handler, bool overwrite)
        {
            auto res = m_registry.emplace(make_pair(url, ResourceEntry()));
            if(!res.second && !overwrite) return E_ALREADY_EXISTS;
            auto& iter = res.first;
            iter->second.type = ResourceType::function;
            iter->second.function = move(handler);
            return ok;
        }

        RV Frontend::set_resource_data(const Name& url, Variant&& data, bool overwrite)
        {
            auto res = m_registry.emplace(make_pair(url, ResourceEntry()));
            if(!res.second && !overwrite) return E_ALREADY_EXISTS;
            auto& iter = res.first;
            iter->second.type = ResourceType::data;
            iter->second.data = move(data);
            return ok;
        }

        RV Frontend::set_resource_userdata(
            const Name& url,
            void* data,
            void (*dtor)(void*),
            bool overwrite)
        {
            auto res = m_registry.emplace(make_pair(url, ResourceEntry()));
            if(!res.second && !overwrite) return E_ALREADY_EXISTS;
            auto& iter = res.first;
            iter->second.type = ResourceType::userdata;
            iter->second.userdata_ptr = data;
            iter->second.userdata_dtor = dtor;
            return ok;
        }

        ResourceType Frontend::get_resource_type(const Name& url)
        {
            auto iter = m_registry.find(url);
            if (iter == m_registry.end())
            {
                return ResourceType::null;
            }
            return iter->second.type;
        }

        R<Variant> Frontend::get_resource_data(const Name& url)
        {
            auto iter = m_registry.find(url);
            if (iter == m_registry.end())
            {
                return E_RESOURCE_NOT_FOUND;
            }
            if (iter->second.type != ResourceType::data)
            {
                return E_TYPE_MISMATCH;
            }
            return iter->second.data;
        }

        void Frontend::remove_resource(const Name& url)
        {
            m_registry.erase(url);
        }

        Variant Frontend::invoke(const Name& url, const Variant& params)
        {
            auto result = invoke_impl(url, params);

            if (failed(result))
            {
                ResultCode code = result.errcode();
                Error err;
                if(code == E_ERROR_OBJECT)
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
                return make_error_response(make_frontend_error(err.code, cat_name, code_name, message, err.info));
            }
            return make_response(result.get());
        }

        R<Variant> Frontend::invoke_impl(const Name& url, const Variant& params)
        {
            auto iter = m_registry.find(url);
            if (iter == m_registry.end() || iter->second.type != ResourceType::function)
            {
                return E_METHOD_NOT_FOUND;
            }
            return iter->second.function(this, params);
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

        LUNA_FRONTEND_API Variant make_response(const Variant& result)
        {
            Variant msg(VariantType::object);
            msg["result"] = move(result);
            msg["error"]  = Variant();
            return msg;
        }

        LUNA_FRONTEND_API Variant make_error_response(const Variant& error)
        {
            Variant msg(VariantType::object);
            msg["result"] = Variant();
            msg["error"]  = move(error);
            return msg;
        }

        LUNA_FRONTEND_API Variant make_frontend_error(
            ResultCode result_code,
            const Name& category,
            const Name& code,
            const Name& message,
            const Variant& data)
        {
            Variant err(VariantType::object);
            c8 result_code_string[17];
            snprintf(result_code_string, sizeof(result_code_string), "%016llX", static_cast<unsigned long long>(result_code.code));
            err["result_code"] = Variant(result_code_string);
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
            return Ref<IFrontend>(o);
        }

        // -----------------------------------------------------------------------
        // Error codes
        // -----------------------------------------------------------------------

        static RV register_error_codes()
        {
            if (!register_error_category(ERROR_CATEGORY, "Frontend") ||
                !register_error_code(E_RESOURCE_NOT_FOUND, "resource_not_found", "The requested frontend resource was not found.") ||
                !register_error_code(E_TYPE_MISMATCH, "type_mismatch", "The frontend resource type does not match the requested type.") ||
                !register_error_code(E_METHOD_NOT_FOUND, "method_not_found", "The requested frontend method was not found."))
            {
                return set_error(E_ALREADY_EXISTS, "Frontend error metadata conflicts with an existing registration.");
            }
            return ok;
        }

        // -----------------------------------------------------------------------
        // Module
        // -----------------------------------------------------------------------

        struct FrontendModule : public Module
        {
            virtual const c8* get_name() override { return "Frontend"; }

            virtual RV on_register() override
            {
                RV result = register_error_codes();
                if (failed(result.errcode())) return result;
                Meta::register_Frontend_types();
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
