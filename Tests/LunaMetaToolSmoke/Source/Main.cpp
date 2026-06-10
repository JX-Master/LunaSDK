#include "MetaSmoke.hpp"
#include "LunaMetaToolSmoke.meta.generated.hpp"
#include <Luna/Runtime/Reflection.hpp>
#include <Luna/Runtime/Ref.hpp>

int main()
{
    Luna::init();
    Luna::MetaToolSmoke::MetaSmokeStruct value;
    (void)Luna::Meta::StructMetaData<Luna::MetaToolSmoke::MetaSmokeStruct>::__guid;
    static_assert(Luna::Meta::StructMetaData<Luna::MetaToolSmoke::MetaSmokeStruct>::__properties[0].offset == 0);
    (void)Luna::Meta::EnumMetadata<Luna::MetaToolSmoke::MetaSmokeEnum>::__guid;
    static_assert(Luna::Meta::EnumMetadata<Luna::MetaToolSmoke::MetaSmokeEnum>::__options[1].value == 1);
    Luna::Meta::register_LunaMetaToolSmoke_types();
    if (!Luna::typeof<Luna::MetaToolSmoke::MetaSmokeStruct>())
    {
        return 1;
    }
    auto noncopyable_type = Luna::typeof<Luna::MetaToolSmoke::MetaSmokeNonCopyable>();
    if (!noncopyable_type)
    {
        return 2;
    }
    if (Luna::is_type_copy_constructable(noncopyable_type))
    {
        return 3;
    }
    if (Luna::is_type_move_constructable(noncopyable_type))
    {
        return 4;
    }
    if (Luna::is_type_copy_assignable(noncopyable_type))
    {
        return 5;
    }
    if (Luna::is_type_move_assignable(noncopyable_type))
    {
        return 6;
    }
    auto boxed_type = Luna::typeof<Luna::MetaToolSmoke::MetaSmokeBoxed>();
    if (!boxed_type)
    {
        return 7;
    }
    if (!Luna::is_interface_implemented_by_type(boxed_type, Luna::Meta::InterfaceMetaData<Luna::MetaToolSmoke::IMetaSmokeInterface>::__guid))
    {
        return 8;
    }
    {
        auto boxed = Luna::new_object<Luna::MetaToolSmoke::MetaSmokeBoxed>();
        auto iface = Luna::query_interface<Luna::MetaToolSmoke::IMetaSmokeInterface>(boxed.get());
        if (!iface)
        {
            return 9;
        }
        if (iface->marker() != 42)
        {
            return 10;
        }
    }
    (void)Luna::typeof<Luna::MetaToolSmoke::MetaSmokeEnum>();
    Luna::close();
    return static_cast<int>(value.value);
}
