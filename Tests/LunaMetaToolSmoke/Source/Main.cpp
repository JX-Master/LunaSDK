#include "MetaSmoke.hpp"

int main()
{
    Luna::MetaToolSmoke::MetaSmokeStruct value;
    (void)Luna::Meta::StructMetaData<Luna::MetaToolSmoke::MetaSmokeStruct>::__guid;
    (void)Luna::Meta::EnumMetadata<Luna::MetaToolSmoke::MetaSmokeEnum>::__guid;
    (void)Luna::typeof<Luna::MetaToolSmoke::MetaSmokeEnum>();
    return static_cast<int>(value.value);
}
