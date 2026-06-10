#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include "MetaSmoke.generated.hpp"

namespace Luna::MetaToolSmoke
{
    struct [[Luna::interface("{6BF6C9B0-0541-42BD-B96B-FEF52C9E4D40}")]] IMetaSmokeInterface : virtual Interface
    {
        virtual u32 marker() = 0;
    };

    struct [[luna::struct("{C7D43C71-9895-47DA-9A78-F5502609BE30}")]] MetaSmokeStruct
    {
        [[Luna::property]] i32 value = 0;
    };

    struct [[luna::struct("{D153C7A4-E834-43FD-8C34-0E6A96110764}")]] MetaSmokeNonCopyable
    {
        MetaSmokeNonCopyable() = default;
        MetaSmokeNonCopyable(const MetaSmokeNonCopyable&) = delete;
        MetaSmokeNonCopyable(MetaSmokeNonCopyable&&) = delete;
        MetaSmokeNonCopyable& operator=(const MetaSmokeNonCopyable&) = delete;
        MetaSmokeNonCopyable& operator=(MetaSmokeNonCopyable&&) = delete;
    };

    struct [[luna::struct("{7B993692-2949-4E6F-9B73-3CA09C23B7BA}")]] MetaSmokeBoxed : IMetaSmokeInterface
    {
        virtual object_t get_object() override
        {
            return this;
        }
        virtual u32 marker() override
        {
            return 42;
        }
    };

    enum class [[luna::enum("{3B894695-E906-40D7-85EC-74E209541438}")]] MetaSmokeEnum : u32
    {
        A [[Luna::option]] = 0,
        B [[Luna::option]] = 1,
    };
}
