/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Frontend.cpp
* @author JXMaster
* @date 2026/3/13
* @brief Frontend implementation: resource registry, invocation, error codes, and module registration.
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_FRONTEND_API LUNA_EXPORT
#include "FrontendImpl.hpp"
#include "Frontend.meta.generated.hpp"
#include "../Frontend.hpp"
#include <Luna/Runtime/Module.hpp>

namespace Luna
{
    namespace Frontend
    {
        RV Frontend::set_resource_function(const Name& url, FunctionHandler&& handler, bool overwrite)
        {
            if(!url || !handler) return BasicError::bad_arguments();
            auto iter = m_registry.find(url);
            if(iter != m_registry.end() && !overwrite) return BasicError::already_exists();
            ResourceEntry entry;
            entry.type = ResourceType::function;
            entry.function = new_object<FunctionResource>(move(handler));
            m_registry.insert_or_assign(url, move(entry));
            return ok;
        }

        RV Frontend::set_resource_data(const Name& url, Variant&& data, bool overwrite)
        {
            if(!url) return BasicError::bad_arguments();
            auto iter = m_registry.find(url);
            if(iter != m_registry.end() && !overwrite) return BasicError::already_exists();
            ResourceEntry entry;
            entry.type = ResourceType::data;
            entry.data = move(data);
            m_registry.insert_or_assign(url, move(entry));
            return ok;
        }

        RV Frontend::set_resource_userdata(
            const Name& url,
            void* data,
            void (*dtor)(void*),
            bool overwrite)
        {
            if(!url) return BasicError::bad_arguments();
            auto iter = m_registry.find(url);
            if(iter != m_registry.end() && !overwrite) return BasicError::already_exists();
            ResourceEntry entry;
            entry.type = ResourceType::userdata;
            entry.userdata_ptr = data;
            entry.userdata_dtor = dtor;
            m_registry.insert_or_assign(url, move(entry));
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
                return FrontendError::resource_not_found();
            }
            if (iter->second.type != ResourceType::data)
            {
                return FrontendError::type_mismatch();
            }
            return iter->second.data;
        }

        R<void*> Frontend::get_resource_userdata(const Name& url)
        {
            auto iter = m_registry.find(url);
            if(iter == m_registry.end())
            {
                return FrontendError::resource_not_found();
            }
            if(iter->second.type != ResourceType::userdata)
            {
                return FrontendError::type_mismatch();
            }
            return iter->second.userdata_ptr;
        }

        RV Frontend::remove_resource(const Name& url)
        {
            if(!url) return BasicError::bad_arguments();
            m_registry.erase(url);
            return ok;
        }

        R<Variant> Frontend::invoke(const Name& url, const Variant& params)
        {
            auto iter = m_registry.find(url);
            if (iter == m_registry.end() || iter->second.type != ResourceType::function)
            {
                return FrontendError::method_not_found();
            }
            Ref<FunctionResource> function = iter->second.function;
            return function->handler(this, params);
        }

        LUNA_FRONTEND_API Ref<IFrontend> new_frontend()
        {
            Ref<Frontend> o = new_object<Frontend>();
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
