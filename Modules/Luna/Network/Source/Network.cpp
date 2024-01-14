/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Network.cpp
* @author JXMaster
* @date 2024/1/11
*/
#include "../Network.hpp"
#include <Luna/Runtime/Module.hpp>
namespace Luna
{
    namespace Network
    {
        RV platform_init();
        void platform_close();
        struct NetworkModule : public Module
        {
            virtual const c8* get_name() override { return "Network"; }
			virtual RV on_init() override
			{
				return platform_init();
			}
			virtual void on_close() override
			{
				platform_close();
			}
        };
    }
    LUNA_NETWORK_API Module* module_network()
    {
        static Network::NetworkModule m;
        return &m;
    }
}