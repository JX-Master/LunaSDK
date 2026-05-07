#pragma once

namespace cppsl
{
    template <typename T>
    struct ConstantBuffer
    {
        const T* operator->() const;
    };

    template <typename T>
    struct StructuredBuffer
    {
        T operator[](unsigned int index) const;
    };

    template <typename T>
    struct RWStructuredBuffer
    {
        T operator[](unsigned int index) const;
    };
}
