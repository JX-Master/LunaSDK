/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file CookedDocument.cpp
* @author JXMaster
* @date 2026/8/29
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GAME_GUI_API LUNA_EXPORT
#include "Internal.hpp"
#include <Luna/Runtime/Algorithm.hpp>
#include <Luna/Runtime/Blob.hpp>
#include <Luna/Runtime/MemoryUtils.hpp>

namespace Luna
{
    namespace GameGUI
    {
        namespace
        {
            constexpr u8 COOKED_MAGIC[8] = {'L', 'U', 'N', 'A', 'G', 'G', 'U', 'I'};
            constexpr u32 MAX_COOKED_COLLECTION_SIZE = 16 * 1024 * 1024;
            constexpr u32 MAX_COOKED_VARIANT_DEPTH = 256;

            RV write_exact(IStream* stream, const void* data, usize size)
            {
                usize written = 0;
                lutry
                {
                    luexp(stream->write(data, size, &written));
                }
                lucatchret;
                return written == size ? ok : RV(E_IO_ERROR);
            }

            RV read_exact(IStream* stream, void* data, usize size)
            {
                usize read_bytes = 0;
                lutry
                {
                    luexp(stream->read(data, size, &read_bytes));
                }
                lucatchret;
                return read_bytes == size ? ok : RV(E_BAD_DATA);
            }

            RV write_u8(IStream* stream, u8 value)
            {
                return write_exact(stream, &value, sizeof(value));
            }

            RV write_u32(IStream* stream, u32 value)
            {
                u8 bytes[4];
                for(u32 i = 0; i < 4; ++i) bytes[i] = (u8)(value >> (i * 8));
                return write_exact(stream, bytes, sizeof(bytes));
            }

            RV write_u64(IStream* stream, u64 value)
            {
                u8 bytes[8];
                for(u32 i = 0; i < 8; ++i) bytes[i] = (u8)(value >> (i * 8));
                return write_exact(stream, bytes, sizeof(bytes));
            }

            R<u8> read_u8(IStream* stream)
            {
                u8 value;
                RV result = read_exact(stream, &value, sizeof(value));
                if(failed(result)) return result.errcode();
                return value;
            }

            R<u32> read_u32(IStream* stream)
            {
                u8 bytes[4];
                RV result = read_exact(stream, bytes, sizeof(bytes));
                if(failed(result)) return result.errcode();
                u32 value = 0;
                for(u32 i = 0; i < 4; ++i) value |= (u32)bytes[i] << (i * 8);
                return value;
            }

            R<u64> read_u64(IStream* stream)
            {
                u8 bytes[8];
                RV result = read_exact(stream, bytes, sizeof(bytes));
                if(failed(result)) return result.errcode();
                u64 value = 0;
                for(u32 i = 0; i < 8; ++i) value |= (u64)bytes[i] << (i * 8);
                return value;
            }

            RV write_guid(IStream* stream, const Guid& value)
            {
                lutry
                {
                    luexp(write_u64(stream, value.high));
                    luexp(write_u64(stream, value.low));
                }
                lucatchret;
                return ok;
            }

            R<Guid> read_guid(IStream* stream)
            {
                lutry
                {
                    lulet(high, read_u64(stream));
                    lulet(low, read_u64(stream));
                    return Guid(high, low);
                }
                lucatchret;
                return E_FAILURE;
            }

            RV write_string(IStream* stream, const c8* value)
            {
                usize size = value ? strlen(value) : 0;
                if(size > U32_MAX) return E_BAD_DATA;
                lutry
                {
                    luexp(write_u32(stream, (u32)size));
                    if(size) luexp(write_exact(stream, value, size));
                }
                lucatchret;
                return ok;
            }

            R<String> read_string(IStream* stream)
            {
                lutry
                {
                    lulet(size, read_u32(stream));
                    if(size > MAX_COOKED_COLLECTION_SIZE)
                        luthrow(set_error(E_BAD_DATA, "GameGUI cooked string is too large."));
                    String value(size, '\0');
                    if(size) luexp(read_exact(stream, value.data(), size));
                    return value;
                }
                lucatchret;
                return E_FAILURE;
            }

            RV write_variant(IStream* stream, const Variant& value, u32 depth = 0)
            {
                if(depth >= MAX_COOKED_VARIANT_DEPTH) return E_BAD_DATA;
                lutry
                {
                    luexp(write_u8(stream, (u8)value.type()));
                    switch(value.type())
                    {
                    case VariantType::null: break;
                    case VariantType::object:
                        if(value.size() > U32_MAX) luthrow(E_BAD_DATA);
                        luexp(write_u32(stream, (u32)value.size()));
                        {
                        Vector<Name> keys;
                        keys.reserve(value.size());
                        for(const auto& pair : value.key_values()) keys.push_back(pair.first);
                        sort(keys.begin(), keys.end(), [](const Name& left, const Name& right)
                        {
                            return strcmp(left.c_str(), right.c_str()) < 0;
                        });
                        for(const Name& key : keys)
                        {
                            luexp(write_string(stream, key.c_str()));
                            luexp(write_variant(stream, value[key], depth + 1));
                        }
                        }
                        break;
                    case VariantType::array:
                        if(value.size() > U32_MAX) luthrow(E_BAD_DATA);
                        luexp(write_u32(stream, (u32)value.size()));
                        for(const Variant& child : value.values())
                            luexp(write_variant(stream, child, depth + 1));
                        break;
                    case VariantType::number:
                        luexp(write_u8(stream, (u8)value.number_type()));
                        if(value.number_type() == VariantNumberType::number_i64)
                        {
                            luexp(write_u64(stream, (u64)value.inum()));
                        }
                        else if(value.number_type() == VariantNumberType::number_u64)
                        {
                            luexp(write_u64(stream, value.unum()));
                        }
                        else
                        {
                            f64 number = value.fnum();
                            u64 bits;
                            memcpy(&bits, &number, sizeof(bits));
                            luexp(write_u64(stream, bits));
                        }
                        break;
                    case VariantType::string:
                        luexp(write_string(stream, value.c_str()));
                        break;
                    case VariantType::boolean:
                        luexp(write_u8(stream, value.boolean() ? 1 : 0));
                        break;
                    case VariantType::blob:
                        if(value.blob_size() > U32_MAX || value.blob_alignment() > U32_MAX)
                            luthrow(E_BAD_DATA);
                        luexp(write_u32(stream, (u32)value.blob_alignment()));
                        luexp(write_u32(stream, (u32)value.blob_size()));
                        if(value.blob_size())
                            luexp(write_exact(stream, value.blob_data(), value.blob_size()));
                        break;
                    default: luthrow(E_BAD_DATA);
                    }
                }
                lucatchret;
                return ok;
            }

            R<Variant> read_variant(IStream* stream, u32 depth = 0)
            {
                if(depth >= MAX_COOKED_VARIANT_DEPTH) return E_BAD_DATA;
                lutry
                {
                    lulet(type_value, read_u8(stream));
                    VariantType type = (VariantType)type_value;
                    switch(type)
                    {
                    case VariantType::null: return Variant();
                    case VariantType::object:
                    {
                        lulet(count, read_u32(stream));
                        if(count > MAX_COOKED_COLLECTION_SIZE) luthrow(E_BAD_DATA);
                        Variant value(VariantType::object);
                        for(u32 i = 0; i < count; ++i)
                        {
                            lulet(key, read_string(stream));
                            lulet(child, read_variant(stream, depth + 1));
                            value[Name(key.c_str())] = move(child);
                        }
                        return value;
                    }
                    case VariantType::array:
                    {
                        lulet(count, read_u32(stream));
                        if(count > MAX_COOKED_COLLECTION_SIZE) luthrow(E_BAD_DATA);
                        Variant value(VariantType::array);
                        for(u32 i = 0; i < count; ++i)
                        {
                            lulet(child, read_variant(stream, depth + 1));
                            value.push_back(move(child));
                        }
                        return value;
                    }
                    case VariantType::number:
                    {
                        lulet(number_type_value, read_u8(stream));
                        lulet(bits, read_u64(stream));
                        VariantNumberType number_type = (VariantNumberType)number_type_value;
                        if(number_type == VariantNumberType::number_i64) return Variant((i64)bits);
                        if(number_type == VariantNumberType::number_u64) return Variant(bits);
                        if(number_type != VariantNumberType::number_f64) luthrow(E_BAD_DATA);
                        f64 number;
                        memcpy(&number, &bits, sizeof(number));
                        return Variant(number);
                    }
                    case VariantType::string:
                    {
                        lulet(value, read_string(stream));
                        return Variant(value.c_str());
                    }
                    case VariantType::boolean:
                    {
                        lulet(value, read_u8(stream));
                        if(value > 1) luthrow(E_BAD_DATA);
                        return Variant(value != 0);
                    }
                    case VariantType::blob:
                    {
                        lulet(alignment, read_u32(stream));
                        lulet(size, read_u32(stream));
                        if(size > MAX_COOKED_COLLECTION_SIZE) luthrow(E_BAD_DATA);
                        Blob blob(size, alignment);
                        if(size) luexp(read_exact(stream, blob.data(), size));
                        return Variant(move(blob));
                    }
                    default: luthrow(E_BAD_DATA);
                    }
                }
                lucatchret;
                return E_FAILURE;
            }
        }

        R<Ref<Document>> read_cooked_document(IStream* stream)
        {
            if(!stream) return E_BAD_ARGUMENTS;
            lutry
            {
                u8 magic[sizeof(COOKED_MAGIC)];
                luexp(read_exact(stream, magic, sizeof(magic)));
                if(memcmp(magic, COOKED_MAGIC, sizeof(magic)))
                    luthrow(set_error(E_BAD_DATA, "Invalid GameGUI cooked document header."));
                Ref<Document> document = new_object<Document>();
                luset(document->root, read_guid(stream));
                lulet(node_count, read_u32(stream));
                if(node_count > MAX_COOKED_COLLECTION_SIZE) luthrow(E_BAD_DATA);
                document->nodes.reserve(node_count);
                for(u32 i = 0; i < node_count; ++i)
                {
                    NodeRecord node;
                    luset(node.id, read_guid(stream));
                    luset(node.type, read_guid(stream));
                    lulet(name, read_string(stream));
                    node.name = name.c_str();
                    luset(node.properties, read_variant(stream));
                    lulet(child_count, read_u32(stream));
                    if(child_count > MAX_COOKED_COLLECTION_SIZE) luthrow(E_BAD_DATA);
                    node.children.reserve(child_count);
                    for(u32 child_index = 0; child_index < child_count; ++child_index)
                    {
                        ChildLink link;
                        luset(link.child, read_guid(stream));
                        lulet(slot, read_string(stream));
                        link.slot = slot.c_str();
                        lulet(has_attachment, read_u8(stream));
                        if(has_attachment > 1) luthrow(E_BAD_DATA);
                        if(has_attachment) luset(link.attachment, read_variant(stream));
                        node.children.push_back(move(link));
                    }
                    document->nodes.push_back(move(node));
                }
                luexp(validate_document(*document));
                return document;
            }
            lucatchret;
            return E_FAILURE;
        }

        RV write_cooked_document(IStream* stream, const Document& document)
        {
            if(!stream) return E_BAD_ARGUMENTS;
            lutry
            {
                luexp(validate_document(document));
                luexp(write_exact(stream, COOKED_MAGIC, sizeof(COOKED_MAGIC)));
                luexp(write_guid(stream, document.root));
                if(document.nodes.size() > U32_MAX) luthrow(E_BAD_DATA);
                luexp(write_u32(stream, (u32)document.nodes.size()));
                for(const NodeRecord& node : document.nodes)
                {
                    luexp(write_guid(stream, node.id));
                    luexp(write_guid(stream, node.type));
                    luexp(write_string(stream, node.name.c_str()));
                    luexp(write_variant(stream, node.properties));
                    if(node.children.size() > U32_MAX) luthrow(E_BAD_DATA);
                    luexp(write_u32(stream, (u32)node.children.size()));
                    for(const ChildLink& link : node.children)
                    {
                        luexp(write_guid(stream, link.child));
                        luexp(write_string(stream, link.slot.c_str()));
                        luexp(write_u8(stream, link.attachment.valid() ? 1 : 0));
                        if(link.attachment.valid()) luexp(write_variant(stream, link.attachment));
                    }
                }
            }
            lucatchret;
            return ok;
        }
    }
}
