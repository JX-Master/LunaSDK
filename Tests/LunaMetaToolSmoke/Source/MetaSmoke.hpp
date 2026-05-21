#pragma once
#include <Luna/Runtime/Runtime.hpp>
#include "MetaSmoke.generated.hpp"

namespace Luna::MetaToolSmoke
{
    struct [[luna::struct("{C7D43C71-9895-47DA-9A78-F5502609BE30}")]] MetaSmokeStruct
    {
        i32 value = 0;
    };

    enum class [[luna::enum("{3B894695-E906-40D7-85EC-74E209541438}")]] MetaSmokeEnum : u32
    {
        A = 0,
        B = 1,
    };
}
