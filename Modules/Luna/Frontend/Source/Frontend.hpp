/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Frontend.hpp
* @author JXMaster
* @date 2026/3/13
* @brief Internal declaration of the Frontend class (IFrontend implementation).
*/
#pragma once
#include "../Frontend.hpp"
#include <Luna/Runtime/Ref.hpp>

namespace Luna
{
    namespace Frontend
    {
        //! Internal storage entry for a single resource.
        struct ResourceEntry
        {
            ResourceType type = ResourceType::null;

            // Valid when type == ResourceType::function
            FunctionHandler function;

            // Valid when type == ResourceType::data
            Variant data;

            // Valid when type == ResourceType::userdata
            void* userdata_ptr    = nullptr;
            void (*userdata_dtor)(void*) = nullptr;

            ResourceEntry() = default;
            ResourceEntry(const ResourceEntry&) = delete;
            ResourceEntry(ResourceEntry&& rhs) :
                type(rhs.type)
            {
                switch(type)
                {
                    case ResourceType::data:
                        data = move(rhs.data);
                    case ResourceType::function:
                        function = move(rhs.function);
                        break;
                    case ResourceType::userdata:
                        userdata_ptr = rhs.userdata_ptr;
                        userdata_dtor = rhs.userdata_dtor;
                        rhs.userdata_ptr = nullptr;
                        rhs.userdata_dtor = nullptr;
                        break;
                    default:
                        break;
                }
                rhs.type = ResourceType::null;
            }
            ResourceEntry& operator=(const ResourceEntry&) = delete;
            ResourceEntry& operator=(ResourceEntry&& rhs)
            {
                type = rhs.type;
                switch(type)
                {
                    case ResourceType::data:
                        data = move(rhs.data);
                    case ResourceType::function:
                        function = move(rhs.function);
                        break;
                    case ResourceType::userdata:
                        userdata_ptr = rhs.userdata_ptr;
                        userdata_dtor = rhs.userdata_dtor;
                        rhs.userdata_ptr = nullptr;
                        rhs.userdata_dtor = nullptr;
                        break;
                    default:
                        break;
                }
                rhs.type = ResourceType::null;
                return *this;
            }

            ~ResourceEntry()
            {
                switch(type)
                {
                    case ResourceType::data:
                    data = Variant();
                    break;
                    case ResourceType::function:
                    function.reset();
                    break;
                    case ResourceType::userdata:
                    if(userdata_ptr)
                    {
                        if (userdata_dtor)
                        {
                            userdata_dtor(userdata_ptr);
                        }
                        userdata_ptr = nullptr;
                    }
                    userdata_dtor = nullptr;
                    break;
                    default:
                    break;
                }
                type = ResourceType::null;
            }
        };

        //! Concrete implementation of IFrontend.
        struct Frontend : public IFrontend
        {
            lustruct("Luna::Frontend::Frontend", "{F0E1D2C3-B4A5-6789-0DEF-1A2B3C4D5E6F}");
            luiimpl();

            HashMap<Path, ResourceEntry> m_registry;

            void init();

            virtual Variant invoke(const Path& url, const Variant& params, const Variant& id) override;
            virtual RV set_function(const Path& url, FunctionHandler&& handler, bool overwrite = false) override;
            virtual RV set_data(const Path& url, Variant&& data, bool overwrite = false) override;
            virtual RV set_userdata(
                const Path& url,
                void*       data,
                void (*destructor)(void*) = nullptr,
                bool        overwrite    = false
            ) override;
            virtual ResourceType get_resource_type(const Path& url) override;
            virtual R<Variant> get_data(const Path& url) override;
            virtual void remove_resource(const Path& url) override;

        private:

            //! Registers the built-in resource functions into m_registry.
            void register_builtin_functions();

            R<Variant> invoke_impl(const Path& url, const Variant& params, const Variant& id);
        };
    }
}
