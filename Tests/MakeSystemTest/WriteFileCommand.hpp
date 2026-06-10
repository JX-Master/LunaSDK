/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include <Luna/MakeSystem/MakeSystem.hpp>
#include <Luna/Runtime/String.hpp>
#include "WriteFileCommand.generated.hpp"

namespace Luna
{
    struct [[luna::struct("{d112916e-514a-4c23-92bc-9d9606e7b1c8}")]] WriteFileCommand : MakeSystem::IMakeCommand
    {
        luiimpl();

        Path output;
        String content;
        Path depfile;
        String depfile_content;
        Path side_output;
        String side_content;
        usize* counter = nullptr;
        bool write_output = true;
        bool fail = false;

        virtual RV execute(LogHandler&) override;
    };
}
