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

    struct [[Luna::interface("{6B0FC0D5-B5AF-4CC1-AC94-8B51A3BC4DDF}")]] IMetaSmokeHelperInterface : virtual Interface
    {
        virtual u32 helper_marker() = 0;
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

    struct MetaSmokePlainBase
    {
        i32 plain_base_value = 0;
    };

    struct [[luna::struct("{BF234AF8-5C4C-42F6-9231-752103CE1203}")]] MetaSmokeStructWithPlainBase : MetaSmokePlainBase
    {
        [[Luna::property]] i32 value = 0;
    };

    struct [[luna::struct("{53746E16-A04F-4203-BDE9-B22EDEB2C3D4}")]] MetaSmokeReflectedBase
    {
        [[Luna::property]] i32 reflected_base_value = 0;
    };

    struct [[luna::struct("{80A50329-D966-46E4-830C-651DA1DDA97F}")]] MetaSmokeDerivedStruct : MetaSmokeReflectedBase
    {
        [[Luna::property]] i32 derived_value = 0;
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

    struct MetaSmokeHelperBase : virtual IMetaSmokeHelperInterface
    {
        virtual u32 helper_marker() override
        {
            return 24;
        }
    };

    struct [[luna::struct("{8D3EE497-A6A6-4ACF-846C-534742793240}")]] MetaSmokeBoxedWithHelperBase : MetaSmokeHelperBase, IMetaSmokeInterface
    {
        virtual object_t get_object() override
        {
            return this;
        }
        virtual u32 marker() override
        {
            return 84;
        }
    };

    enum class [[luna::enum("{3B894695-E906-40D7-85EC-74E209541438}")]] MetaSmokeEnum : u32
    {
        A [[Luna::option]] = 0,
        B [[Luna::option]] = 1,
    };
}
