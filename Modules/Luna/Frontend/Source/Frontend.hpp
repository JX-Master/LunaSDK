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
#include "Frontend.generated.hpp"
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
        struct [[luna::struct("{F0E1D2C3-B4A5-6789-0DEF-1A2B3C4D5E6F}")]] Frontend : public IFrontend
        {
            luiimpl();

            HashMap<Name, ResourceEntry> m_registry;

            virtual Variant invoke(const Name& url, const Variant& params) override;
            virtual RV set_resource_function(const Name& url, FunctionHandler&& handler, bool overwrite = false) override;
            virtual RV set_resource_data(const Name& url, Variant&& data, bool overwrite = false) override;
            virtual RV set_resource_userdata(
                const Name& url,
                void*       data,
                void (*destructor)(void*) = nullptr,
                bool        overwrite    = false
            ) override;
            virtual ResourceType get_resource_type(const Name& url) override;
            virtual R<Variant> get_resource_data(const Name& url) override;
            virtual void remove_resource(const Name& url) override;

        private:

            R<Variant> invoke_impl(const Name& url, const Variant& params);
        };
    }
}
