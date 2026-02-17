/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Fiber.hpp
* @author JXMaster
* @date 2026/2/17
*/
#pragma once
#include "../Fiber.hpp"
#include "OS.hpp"
namespace Luna
{
    struct Fiber : IFiber
    {
        lustruct("Fiber", "{c07ce059-34ec-4df8-9699-02c3110be31b}");
        luiimpl();

        OS::FiberContext m_context;
        bool m_should_delete = false;

        ~Fiber();
    };
}