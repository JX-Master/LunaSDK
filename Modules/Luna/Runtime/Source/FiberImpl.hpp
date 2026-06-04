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
#include "Platform/Fiber.hpp"
#include "FiberImpl.generated.hpp"
namespace Luna
{
    struct [[luna::struct("{c07ce059-34ec-4df8-9699-02c3110be31b}")]] Fiber : IFiber
    {
        luiimpl();

        Platform::Fiber m_fiber;
        bool m_should_delete = false;

        ~Fiber();
    };
}