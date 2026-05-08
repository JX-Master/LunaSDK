#include "VariantUtils.h"

#include <Luna/VariantUtils/VariantUtils.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/VariantUtils/XML.hpp>
#include <Luna/VariantUtils/Diff.hpp>
#include <Luna/Runtime/Blob.hpp>
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/Memory.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Stream.hpp>
#include <Luna/Runtime/Variant.hpp>
#include <Luna/Runtime/Interface.hpp>

#include <cstring>

namespace
{
luna_errcode_t from_errcode(Luna::ErrCode code)
{
    return static_cast<luna_errcode_t>(code.code);
}

luna_errcode_t from_result(const Luna::RV& result)
{
    return from_errcode(result.errcode());
}

LunaVariantUtilsVariantHandle from_variant_ptr(Luna::Variant* variant)
{
    return LunaVariantUtilsVariantHandle{variant};
}

Luna::Variant* to_variant_ptr(LunaVariantUtilsVariantHandle handle)
{
    return reinterpret_cast<Luna::Variant*>(handle.variant);
}

LunaVariantUtilsVariantHandle clone_variant_handle(const Luna::Variant& variant)
{
    return from_variant_ptr(Luna::memnew<Luna::Variant>(variant));
}

const char* duplicate_string(const char* source)
{
    if(!source)
    {
        return nullptr;
    }
    auto size = std::strlen(source);
    auto* buffer = static_cast<char*>(Luna::memalloc(size + 1));
    if(!buffer)
    {
        return nullptr;
    }
    std::memcpy(buffer, source, size + 1);
    return buffer;
}

Luna::IStream* get_stream(luna_handle_t stream_object)
{
    return stream_object ? Luna::query_interface<Luna::IStream>(stream_object) : nullptr;
}
}

extern "C"
{
LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_init_module(void)
{
    Luna::Module* module = Luna::module_variant_utils();
    auto result = Luna::add_module(module);
    if(!result.valid())
    {
        return from_result(result);
    }
    result = Luna::init_module(module);
    return from_result(result);
}

LUNA_VARIANT_UTILS_C_API void luna_variant_utils_free_string(const char* text)
{
    if(text)
    {
        Luna::memfree(const_cast<char*>(text));
    }
}

LUNA_VARIANT_UTILS_C_API void luna_variant_utils_free_buffer(void* data)
{
    if(data)
    {
        Luna::memfree(data);
    }
}

LUNA_VARIANT_UTILS_C_API LunaVariantUtilsVariantHandle luna_variant_utils_variant_create(uint8_t type)
{
    return from_variant_ptr(Luna::memnew<Luna::Variant>(static_cast<Luna::VariantType>(type)));
}

LUNA_VARIANT_UTILS_C_API LunaVariantUtilsVariantHandle luna_variant_utils_variant_create_i64(int64_t value)
{
    return from_variant_ptr(Luna::memnew<Luna::Variant>(static_cast<Luna::i64>(value)));
}

LUNA_VARIANT_UTILS_C_API LunaVariantUtilsVariantHandle luna_variant_utils_variant_create_u64(uint64_t value)
{
    return from_variant_ptr(Luna::memnew<Luna::Variant>(static_cast<Luna::u64>(value)));
}

LUNA_VARIANT_UTILS_C_API LunaVariantUtilsVariantHandle luna_variant_utils_variant_create_f64(double value)
{
    return from_variant_ptr(Luna::memnew<Luna::Variant>(static_cast<Luna::f64>(value)));
}

LUNA_VARIANT_UTILS_C_API LunaVariantUtilsVariantHandle luna_variant_utils_variant_create_string(const char* value)
{
    return from_variant_ptr(Luna::memnew<Luna::Variant>(value ? value : ""));
}

LUNA_VARIANT_UTILS_C_API LunaVariantUtilsVariantHandle luna_variant_utils_variant_create_boolean(int32_t value)
{
    return from_variant_ptr(Luna::memnew<Luna::Variant>(value != 0));
}

LUNA_VARIANT_UTILS_C_API LunaVariantUtilsVariantHandle luna_variant_utils_variant_create_blob(const void* data, uint64_t size, uint64_t alignment)
{
    Luna::Blob blob;
    if(data && size)
    {
        blob = Luna::Blob(static_cast<const Luna::byte_t*>(data), static_cast<Luna::usize>(size), static_cast<Luna::usize>(alignment));
    }
    return from_variant_ptr(Luna::memnew<Luna::Variant>(Luna::move(blob)));
}

LUNA_VARIANT_UTILS_C_API void luna_variant_utils_variant_destroy(LunaVariantUtilsVariantHandle variant)
{
    auto* ptr = to_variant_ptr(variant);
    if(ptr)
    {
        Luna::memdelete(ptr);
    }
}

LUNA_VARIANT_UTILS_C_API LunaVariantUtilsVariantHandle luna_variant_utils_variant_clone(LunaVariantUtilsVariantHandle variant)
{
    auto* ptr = to_variant_ptr(variant);
    return ptr ? clone_variant_handle(*ptr) : LunaVariantUtilsVariantHandle{nullptr};
}

LUNA_VARIANT_UTILS_C_API int32_t luna_variant_utils_variant_equals(LunaVariantUtilsVariantHandle lhs, LunaVariantUtilsVariantHandle rhs)
{
    auto* lhs_ptr = to_variant_ptr(lhs);
    auto* rhs_ptr = to_variant_ptr(rhs);
    if(!lhs_ptr || !rhs_ptr)
    {
        return 0;
    }
    return (*lhs_ptr == *rhs_ptr) ? 1 : 0;
}

LUNA_VARIANT_UTILS_C_API uint8_t luna_variant_utils_variant_get_type(LunaVariantUtilsVariantHandle variant)
{
    auto* ptr = to_variant_ptr(variant);
    return ptr ? static_cast<uint8_t>(ptr->type()) : static_cast<uint8_t>(Luna::VariantType::null);
}

LUNA_VARIANT_UTILS_C_API uint8_t luna_variant_utils_variant_get_number_type(LunaVariantUtilsVariantHandle variant)
{
    auto* ptr = to_variant_ptr(variant);
    return ptr ? static_cast<uint8_t>(ptr->number_type()) : static_cast<uint8_t>(Luna::VariantNumberType::not_number);
}

LUNA_VARIANT_UTILS_C_API int32_t luna_variant_utils_variant_valid(LunaVariantUtilsVariantHandle variant)
{
    auto* ptr = to_variant_ptr(variant);
    return ptr && ptr->valid() ? 1 : 0;
}

LUNA_VARIANT_UTILS_C_API uint64_t luna_variant_utils_variant_get_size(LunaVariantUtilsVariantHandle variant)
{
    auto* ptr = to_variant_ptr(variant);
    return ptr ? static_cast<uint64_t>(ptr->size()) : 0;
}

LUNA_VARIANT_UTILS_C_API int32_t luna_variant_utils_variant_contains(LunaVariantUtilsVariantHandle variant, const char* key)
{
    auto* ptr = to_variant_ptr(variant);
    return ptr && key && ptr->contains(key) ? 1 : 0;
}

LUNA_VARIANT_UTILS_C_API LunaVariantUtilsVariantHandle luna_variant_utils_variant_get_array_item(LunaVariantUtilsVariantHandle variant, uint64_t index)
{
    auto* ptr = to_variant_ptr(variant);
    if(!ptr)
    {
        return LunaVariantUtilsVariantHandle{nullptr};
    }
    return clone_variant_handle(ptr->at(static_cast<Luna::usize>(index)));
}

LUNA_VARIANT_UTILS_C_API LunaVariantUtilsVariantHandle luna_variant_utils_variant_get_object_item(LunaVariantUtilsVariantHandle variant, const char* key)
{
    auto* ptr = to_variant_ptr(variant);
    if(!ptr || !key)
    {
        return LunaVariantUtilsVariantHandle{nullptr};
    }
    return clone_variant_handle((*ptr)[key]);
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_variant_get_object_key(LunaVariantUtilsVariantHandle variant, uint64_t index, const char** out_key)
{
    auto* ptr = to_variant_ptr(variant);
    if(!ptr || !out_key)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_key = nullptr;
    Luna::usize current = 0;
    for(auto& pair : ptr->key_values())
    {
        if(current == static_cast<Luna::usize>(index))
        {
            *out_key = duplicate_string(pair.first.c_str());
            return *out_key ? 0 : from_errcode(Luna::BasicError::out_of_memory());
        }
        ++current;
    }
    return from_errcode(Luna::BasicError::out_of_range());
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_variant_set_array_item(LunaVariantUtilsVariantHandle variant, uint64_t index, LunaVariantUtilsVariantHandle value)
{
    auto* ptr = to_variant_ptr(variant);
    auto* value_ptr = to_variant_ptr(value);
    if(!ptr || !value_ptr)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    ptr->at(static_cast<Luna::usize>(index)) = *value_ptr;
    return 0;
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_variant_insert_array_item(LunaVariantUtilsVariantHandle variant, uint64_t index, LunaVariantUtilsVariantHandle value)
{
    auto* ptr = to_variant_ptr(variant);
    auto* value_ptr = to_variant_ptr(value);
    if(!ptr || !value_ptr)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    ptr->insert(static_cast<Luna::usize>(index), *value_ptr);
    return 0;
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_variant_push_back(LunaVariantUtilsVariantHandle variant, LunaVariantUtilsVariantHandle value)
{
    auto* ptr = to_variant_ptr(variant);
    auto* value_ptr = to_variant_ptr(value);
    if(!ptr || !value_ptr)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    ptr->push_back(*value_ptr);
    return 0;
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_variant_erase_array_item(LunaVariantUtilsVariantHandle variant, uint64_t index)
{
    auto* ptr = to_variant_ptr(variant);
    if(!ptr)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    ptr->erase(static_cast<Luna::usize>(index));
    return 0;
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_variant_pop_back(LunaVariantUtilsVariantHandle variant)
{
    auto* ptr = to_variant_ptr(variant);
    if(!ptr)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    ptr->pop_back();
    return 0;
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_variant_set_object_item(LunaVariantUtilsVariantHandle variant, const char* key, LunaVariantUtilsVariantHandle value)
{
    auto* ptr = to_variant_ptr(variant);
    auto* value_ptr = to_variant_ptr(value);
    if(!ptr || !key || !value_ptr)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    ptr->find_or_insert(key) = *value_ptr;
    return 0;
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_variant_erase_object_item(LunaVariantUtilsVariantHandle variant, const char* key)
{
    auto* ptr = to_variant_ptr(variant);
    if(!ptr || !key)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    ptr->erase(key);
    return 0;
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_variant_get_string(LunaVariantUtilsVariantHandle variant, const char** out_string)
{
    auto* ptr = to_variant_ptr(variant);
    if(!ptr || !out_string)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_string = duplicate_string(ptr->c_str());
    return *out_string ? 0 : from_errcode(Luna::BasicError::out_of_memory());
}

LUNA_VARIANT_UTILS_C_API int64_t luna_variant_utils_variant_get_i64(LunaVariantUtilsVariantHandle variant)
{
    auto* ptr = to_variant_ptr(variant);
    return ptr ? static_cast<int64_t>(ptr->inum()) : 0;
}

LUNA_VARIANT_UTILS_C_API uint64_t luna_variant_utils_variant_get_u64(LunaVariantUtilsVariantHandle variant)
{
    auto* ptr = to_variant_ptr(variant);
    return ptr ? static_cast<uint64_t>(ptr->unum()) : 0;
}

LUNA_VARIANT_UTILS_C_API double luna_variant_utils_variant_get_f64(LunaVariantUtilsVariantHandle variant)
{
    auto* ptr = to_variant_ptr(variant);
    return ptr ? static_cast<double>(ptr->fnum()) : 0.0;
}

LUNA_VARIANT_UTILS_C_API int32_t luna_variant_utils_variant_get_boolean(LunaVariantUtilsVariantHandle variant)
{
    auto* ptr = to_variant_ptr(variant);
    return ptr && ptr->boolean() ? 1 : 0;
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_variant_get_blob(LunaVariantUtilsVariantHandle variant, void** out_data, uint64_t* out_size, uint64_t* out_alignment)
{
    auto* ptr = to_variant_ptr(variant);
    if(!ptr || !out_data || !out_size || !out_alignment)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_data = nullptr;
    *out_size = 0;
    *out_alignment = 0;
    auto size = ptr->blob_size();
    auto alignment = ptr->blob_alignment();
    auto data = ptr->blob_data();
    if(data && size)
    {
        auto* buffer = Luna::memalloc(size, alignment);
        if(!buffer)
        {
            return from_errcode(Luna::BasicError::out_of_memory());
        }
        std::memcpy(buffer, data, size);
        *out_data = buffer;
    }
    *out_size = static_cast<uint64_t>(size);
    *out_alignment = static_cast<uint64_t>(alignment);
    return 0;
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_read_json(const char* src, uint64_t src_size, LunaVariantUtilsVariantHandle* out_variant)
{
    if(!src || !out_variant)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto result = Luna::VariantUtils::read_json(src, static_cast<Luna::usize>(src_size));
    if(!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_variant = from_variant_ptr(Luna::memnew<Luna::Variant>(Luna::move(result.get())));
    return 0;
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_read_json_stream(luna_handle_t stream_object, LunaVariantUtilsVariantHandle* out_variant)
{
    auto* stream = get_stream(stream_object);
    if(!stream || !out_variant)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto result = Luna::VariantUtils::read_json(stream);
    if(!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_variant = from_variant_ptr(Luna::memnew<Luna::Variant>(Luna::move(result.get())));
    return 0;
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_write_json(LunaVariantUtilsVariantHandle variant, int32_t indent, const char** out_string)
{
    auto* ptr = to_variant_ptr(variant);
    if(!ptr || !out_string)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_string = duplicate_string(Luna::VariantUtils::write_json(*ptr, indent != 0).c_str());
    return *out_string ? 0 : from_errcode(Luna::BasicError::out_of_memory());
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_write_json_stream(luna_handle_t stream_object, LunaVariantUtilsVariantHandle variant, int32_t indent)
{
    auto* stream = get_stream(stream_object);
    auto* ptr = to_variant_ptr(variant);
    if(!stream || !ptr)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::VariantUtils::write_json(stream, *ptr, indent != 0));
}

LUNA_VARIANT_UTILS_C_API LunaVariantUtilsVariantHandle luna_variant_utils_new_xml_element(const char* name)
{
    return from_variant_ptr(Luna::memnew<Luna::Variant>(Luna::VariantUtils::new_xml_element(name ? name : "")));
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_get_xml_name(LunaVariantUtilsVariantHandle xml_element, const char** out_name)
{
    auto* ptr = to_variant_ptr(xml_element);
    if(!ptr || !out_name)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_name = duplicate_string(Luna::VariantUtils::get_xml_name(*ptr).c_str());
    return *out_name ? 0 : from_errcode(Luna::BasicError::out_of_memory());
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_set_xml_name(LunaVariantUtilsVariantHandle xml_element, const char* name)
{
    auto* ptr = to_variant_ptr(xml_element);
    if(!ptr || !name)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::VariantUtils::set_xml_name(*ptr, name);
    return 0;
}

LUNA_VARIANT_UTILS_C_API LunaVariantUtilsVariantHandle luna_variant_utils_get_xml_attributes(LunaVariantUtilsVariantHandle xml_element)
{
    auto* ptr = to_variant_ptr(xml_element);
    return ptr ? clone_variant_handle(Luna::VariantUtils::get_xml_attributes(*ptr)) : LunaVariantUtilsVariantHandle{nullptr};
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_set_xml_attributes(LunaVariantUtilsVariantHandle xml_element, LunaVariantUtilsVariantHandle attributes)
{
    auto* xml_ptr = to_variant_ptr(xml_element);
    auto* attributes_ptr = to_variant_ptr(attributes);
    if(!xml_ptr || !attributes_ptr)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::VariantUtils::get_xml_attributes(*xml_ptr) = *attributes_ptr;
    return 0;
}

LUNA_VARIANT_UTILS_C_API LunaVariantUtilsVariantHandle luna_variant_utils_get_xml_content(LunaVariantUtilsVariantHandle xml_element)
{
    auto* ptr = to_variant_ptr(xml_element);
    return ptr ? clone_variant_handle(Luna::VariantUtils::get_xml_content(*ptr)) : LunaVariantUtilsVariantHandle{nullptr};
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_set_xml_content(LunaVariantUtilsVariantHandle xml_element, LunaVariantUtilsVariantHandle content)
{
    auto* xml_ptr = to_variant_ptr(xml_element);
    auto* content_ptr = to_variant_ptr(content);
    if(!xml_ptr || !content_ptr)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::VariantUtils::get_xml_content(*xml_ptr) = *content_ptr;
    return 0;
}

LUNA_VARIANT_UTILS_C_API LunaVariantUtilsVariantHandle luna_variant_utils_find_first_xml_child_element(LunaVariantUtilsVariantHandle xml_element, const char* name, uint64_t start_index, uint64_t* out_index)
{
    auto* ptr = to_variant_ptr(xml_element);
    if(!ptr || !name)
    {
        return LunaVariantUtilsVariantHandle{nullptr};
    }
    Luna::usize native_index = 0;
    const auto& child = Luna::VariantUtils::find_first_xml_child_element(*ptr, name, static_cast<Luna::usize>(start_index), out_index ? &native_index : nullptr);
    if(out_index)
    {
        *out_index = static_cast<uint64_t>(native_index);
    }
    return clone_variant_handle(child);
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_read_xml(const void* src, uint64_t src_size, LunaVariantUtilsVariantHandle* out_variant)
{
    if(!src || !out_variant)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto result = Luna::VariantUtils::read_xml(src, static_cast<Luna::usize>(src_size));
    if(!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_variant = from_variant_ptr(Luna::memnew<Luna::Variant>(Luna::move(result.get())));
    return 0;
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_read_xml_stream(luna_handle_t stream_object, LunaVariantUtilsVariantHandle* out_variant)
{
    auto* stream = get_stream(stream_object);
    if(!stream || !out_variant)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    auto result = Luna::VariantUtils::read_xml(stream);
    if(!result.valid())
    {
        return from_errcode(result.errcode());
    }
    *out_variant = from_variant_ptr(Luna::memnew<Luna::Variant>(Luna::move(result.get())));
    return 0;
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_write_xml(LunaVariantUtilsVariantHandle variant, int32_t indent, const char** out_string)
{
    auto* ptr = to_variant_ptr(variant);
    if(!ptr || !out_string)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    *out_string = duplicate_string(Luna::VariantUtils::write_xml(*ptr, indent != 0).c_str());
    return *out_string ? 0 : from_errcode(Luna::BasicError::out_of_memory());
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_write_xml_stream(luna_handle_t stream_object, LunaVariantUtilsVariantHandle variant, int32_t indent)
{
    auto* stream = get_stream(stream_object);
    auto* ptr = to_variant_ptr(variant);
    if(!stream || !ptr)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    return from_result(Luna::VariantUtils::write_xml(stream, *ptr, indent != 0));
}

LUNA_VARIANT_UTILS_C_API LunaVariantUtilsVariantHandle luna_variant_utils_diff(LunaVariantUtilsVariantHandle before, LunaVariantUtilsVariantHandle after)
{
    auto* before_ptr = to_variant_ptr(before);
    auto* after_ptr = to_variant_ptr(after);
    if(!before_ptr || !after_ptr)
    {
        return LunaVariantUtilsVariantHandle{nullptr};
    }
    return from_variant_ptr(Luna::memnew<Luna::Variant>(Luna::VariantUtils::diff(*before_ptr, *after_ptr)));
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_patch(LunaVariantUtilsVariantHandle before, LunaVariantUtilsVariantHandle delta)
{
    auto* before_ptr = to_variant_ptr(before);
    auto* delta_ptr = to_variant_ptr(delta);
    if(!before_ptr || !delta_ptr)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::VariantUtils::patch(*before_ptr, *delta_ptr);
    return 0;
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_revert(LunaVariantUtilsVariantHandle after, LunaVariantUtilsVariantHandle delta)
{
    auto* after_ptr = to_variant_ptr(after);
    auto* delta_ptr = to_variant_ptr(delta);
    if(!after_ptr || !delta_ptr)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::VariantUtils::revert(*after_ptr, *delta_ptr);
    return 0;
}

LUNA_VARIANT_UTILS_C_API luna_errcode_t luna_variant_utils_add_diff_prefix(LunaVariantUtilsVariantHandle delta, const LunaVariantUtilsVariantHandle* prefix_nodes, uint64_t count)
{
    auto* delta_ptr = to_variant_ptr(delta);
    if(!delta_ptr || (!prefix_nodes && count))
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }
    Luna::Vector<Luna::Variant> native_nodes;
    native_nodes.reserve(static_cast<Luna::usize>(count));
    for(Luna::usize i = 0; i < static_cast<Luna::usize>(count); ++i)
    {
        auto* node_ptr = to_variant_ptr(prefix_nodes[i]);
        if(!node_ptr)
        {
            return from_errcode(Luna::BasicError::bad_arguments());
        }
        native_nodes.push_back(*node_ptr);
    }
    Luna::VariantUtils::add_diff_prefix(*delta_ptr, {native_nodes.data(), native_nodes.size()});
    return 0;
}
}
