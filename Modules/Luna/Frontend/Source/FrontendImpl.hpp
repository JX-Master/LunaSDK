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
#include "FrontendImpl.generated.hpp"
namespace Luna
{
    namespace Frontend
    {
        //! Reference-counted storage for one function handler.
        struct [[luna::struct("{01F6C18D-6D67-4D34-ABB2-731841169B0E}")]] FunctionResource
        {
            FunctionHandler handler;

            FunctionResource() = default;
            FunctionResource(FunctionHandler&& handler) : handler(move(handler)) {}
        };

        //! Internal storage entry for a single resource.
        struct ResourceEntry
        {
            ResourceType type = ResourceType::null;

            // Valid when type == ResourceType::function
            Ref<FunctionResource> function;

            // Valid when type == ResourceType::data
            Variant data;

            // Valid when type == ResourceType::userdata
            void* userdata_ptr    = nullptr;
            void (*userdata_dtor)(void*) = nullptr;

            ResourceEntry() = default;
            ResourceEntry(const ResourceEntry&) = delete;
            ResourceEntry(ResourceEntry&& rhs)
            {
                move_from(rhs);
            }
            ResourceEntry& operator=(const ResourceEntry&) = delete;
            ResourceEntry& operator=(ResourceEntry&& rhs)
            {
                if(this != &rhs)
                {
                    reset();
                    move_from(rhs);
                }
                return *this;
            }

            ~ResourceEntry()
            {
                reset();
            }

        private:

            void reset()
            {
                if(type == ResourceType::userdata && userdata_ptr && userdata_dtor)
                {
                    userdata_dtor(userdata_ptr);
                }
                function.reset();
                data = Variant();
                userdata_ptr = nullptr;
                userdata_dtor = nullptr;
                type = ResourceType::null;
            }

            void move_from(ResourceEntry& rhs)
            {
                type = rhs.type;
                function = move(rhs.function);
                data = move(rhs.data);
                userdata_ptr = rhs.userdata_ptr;
                userdata_dtor = rhs.userdata_dtor;

                rhs.type = ResourceType::null;
                rhs.userdata_ptr = nullptr;
                rhs.userdata_dtor = nullptr;
            }
        };

        //! Concrete implementation of IFrontend.
        struct [[luna::struct("{F0E1D2C3-B4A5-6789-0DEF-1A2B3C4D5E6F}")]] Frontend : public IFrontend
        {
            luiimpl();

            HashMap<Name, ResourceEntry> m_registry;

            virtual R<Variant> invoke(const Name& url, const Variant& params) override;
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
            virtual R<void*> get_resource_userdata(const Name& url) override;
            virtual RV remove_resource(const Name& url) override;
        };
    }
}
