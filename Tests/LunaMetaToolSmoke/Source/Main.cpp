#include "MetaSmoke.hpp"

int main()
{
    Luna::MetaToolSmoke::MetaSmokeStruct value;
    (void)Luna::Meta::StructMetaData<Luna::MetaToolSmoke::MetaSmokeStruct>::__guid;
    static_assert(Luna::Meta::StructMetaData<Luna::MetaToolSmoke::MetaSmokeStruct>::__properties[0].offset == 0);
    (void)Luna::Meta::EnumMetadata<Luna::MetaToolSmoke::MetaSmokeEnum>::__guid;
    static_assert(Luna::Meta::EnumMetadata<Luna::MetaToolSmoke::MetaSmokeEnum>::__options[1].value == 1);
    (void)Luna::typeof<Luna::MetaToolSmoke::MetaSmokeEnum>();
    return static_cast<int>(value.value);
}
