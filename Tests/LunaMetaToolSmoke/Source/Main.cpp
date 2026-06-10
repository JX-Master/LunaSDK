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
    auto plain_base_type = Luna::typeof<Luna::MetaToolSmoke::MetaSmokeStructWithPlainBase>();
    if (!plain_base_type)
    {
        return 7;
    }
    if (Luna::get_base_type(plain_base_type))
    {
        return 8;
    }
    auto reflected_base_type = Luna::typeof<Luna::MetaToolSmoke::MetaSmokeReflectedBase>();
    if (!reflected_base_type)
    {
        return 9;
    }
    auto derived_type = Luna::typeof<Luna::MetaToolSmoke::MetaSmokeDerivedStruct>();
    if (!derived_type)
    {
        return 10;
    }
    if (Luna::get_base_type(derived_type) != reflected_base_type)
    {
        return 11;
    }
    auto boxed_type = Luna::typeof<Luna::MetaToolSmoke::MetaSmokeBoxed>();
    if (!boxed_type)
    {
        return 12;
    }
    if (!Luna::is_interface_implemented_by_type(boxed_type, Luna::Meta::InterfaceMetaData<Luna::MetaToolSmoke::IMetaSmokeInterface>::__guid))
    {
        return 13;
    }
    {
        auto boxed = Luna::new_object<Luna::MetaToolSmoke::MetaSmokeBoxed>();
        auto iface = Luna::query_interface<Luna::MetaToolSmoke::IMetaSmokeInterface>(boxed.get());
        if (!iface)
        {
            return 14;
        }
        if (iface->marker() != 42)
        {
            return 15;
        }
    }
    auto helper_boxed_type = Luna::typeof<Luna::MetaToolSmoke::MetaSmokeBoxedWithHelperBase>();
    if (!helper_boxed_type)
    {
        return 16;
    }
    if (!Luna::is_interface_implemented_by_type(helper_boxed_type, Luna::Meta::InterfaceMetaData<Luna::MetaToolSmoke::IMetaSmokeInterface>::__guid))
    {
        return 17;
    }
    if (!Luna::is_interface_implemented_by_type(helper_boxed_type, Luna::Meta::InterfaceMetaData<Luna::MetaToolSmoke::IMetaSmokeHelperInterface>::__guid))
    {
        return 18;
    }
    {
        auto boxed = Luna::new_object<Luna::MetaToolSmoke::MetaSmokeBoxedWithHelperBase>();
        auto iface = Luna::query_interface<Luna::MetaToolSmoke::IMetaSmokeInterface>(boxed.get());
        if (!iface)
        {
            return 19;
        }
        if (iface->marker() != 84)
        {
            return 20;
        }
        auto helper_iface = Luna::query_interface<Luna::MetaToolSmoke::IMetaSmokeHelperInterface>(boxed.get());
        if (!helper_iface)
        {
            return 21;
        }
        if (helper_iface->helper_marker() != 24)
        {
            return 22;
        }
    }
    (void)Luna::typeof<Luna::MetaToolSmoke::MetaSmokeEnum>();
    Luna::close();
    return static_cast<int>(value.value);
}
