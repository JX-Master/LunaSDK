/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Coroutine.hpp
* @author JXMaster
* @date 2026/2/14
*/
#include "../Coroutine.hpp"

namespace Luna
{
    struct Coroutine : ICoroutine
    {
        lustruct("Coroutine", "{5d8c09eb-817c-411c-b49e-9335ec55a102}");
        luiimpl();

        Ref<IFiber> m_fiber;
        IFiber* m_parent = nullptr;
        void (*m_entry_func)(void* param) = nullptr;
        void* m_param = nullptr;

        virtual IFiber* get_fiber() override
        {
            return m_fiber;
        }
    };

    void coroutine_init();
    void coroutine_close();
}