#pragma once
#include <cppsl/core.hxx>

namespace cppsl
{
    void GroupMemoryBarrierWithGroupSync();
    void InterlockedAdd(uint& dest, uint value);
}
