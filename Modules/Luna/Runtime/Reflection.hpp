/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TypeInfo.hpp
* @author JXMaster
* @date 2022/5/14
* @brief APIs to fetch type informations from `typeinfo_t`.
*/
#pragma once
#include "TypeInfo.hpp"
#include "Variant.hpp"

namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeType Type reflection
	//! @}

	//! @addtogroup RuntimeType
	//! @{

	//! @brief Checks whether one type is a primitive type.
	//! @param[in] type The type object to check.
	//! @return Returns `true` if the specified type is a primitive type.
	//! Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API bool is_primitive_type(typeinfo_t type);
	//! @brief Checks whether one type is a structure type.
	//! @param[in] type The type object to check.
	//! @return Returns `true` if the specified type is a structure type.
	//! Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API bool is_struct_type(typeinfo_t type);
	//! @brief Checks whether one type is an enumeration type.
	//! @param[in] type The type object to check.
	//! @return Returns `true` if the specified type is an enumeration type.
	//! Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API bool is_enum_type(typeinfo_t type);
	//! @brief Checks whether one type is a generic structure type.
	//! @param[in] type The type object to check.
	//! @return Returns `true` if the specified type is a generic structure type.
	//! Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API bool is_generic_struct_type(typeinfo_t type);
	//! @brief Checks whether one type is a generic instanced structure type.
	//! @param[in] type The type object to check.
	//! @return Returns `true` if the specified type is a generic instanced structure type.
	//! Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API bool is_generic_struct_instanced_type(typeinfo_t type);

	//! @brief Gets one type by its name.
	//! @param[in] name The name of the type.
	//! @param[in] alias The alias name of the type.
	//! @return Returns the type object that matches the name.
	//! Returns `nullptr` if no such type is found.
	LUNA_RUNTIME_API typeinfo_t get_type_by_name(const Name& name, const Name& alias = Name());
	//! @brief Gets the name of the specified type.
	//! @param[in] type The type object.
	//! @param[out] alias If not `nullptr`, returns the alias of the type.
	//! @return Returns the name of the specified type.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API Name get_type_name(typeinfo_t type, Name* alias = nullptr);
	//! @brief Gets the GUID of the specified type.
	//! @param[in] type The type object.
	//! @return Returns the GUID of the specified type.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API Guid get_type_guid(typeinfo_t type);
	//! @brief Gets the size of the specified type.
	//! @param[in] type The type object.
	//! @return Returns the size of the specified type in bytes.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API usize get_type_size(typeinfo_t type);
	//! @brief Gets the alignment requirement of the specified type.
	//! @param[in] type The type object.
	//! @return Returns the alignment requirement of the specified type in bytes.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API usize get_type_alignment(typeinfo_t type);
	//! @brief Gets user defined private data for the specified type.
	//! @param[in] type The type object.
	//! @param[in] data_guid The GUID of the private data to check.
	//! @return Returns one pointer to the private data. Returns `nullptr` if such data does not exist.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API void* get_type_private_data(typeinfo_t type, const Guid& data_guid);
	//! @brief Sets user defined private data for the specified type.
	//! @param[in] type The type object.
	//! @param[in] data_guid The GUID of the private data. If one data with this GUID already exists, the original data
	//! will be deleted and replaced by one new data.
	//! @param[in] data_size The size of the data in bytes.
	//! @param[in] data_alignment The alignment requirement of the data in bytes. Specify `0` to use the default alignment, 
	//! which is @ref MAX_ALIGN.
	//! @param[in] data_dtor One optional callback function that will be called when the data is going to be freed if specified.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API void* set_type_private_data(typeinfo_t type, const Guid& data_guid, usize data_size, usize data_alignment = 0, void(*data_dtor)(void*) = nullptr);

	//! @brief Checks whether one type is a trivially constructable type.
	//! @details One type is trivially constructable if:
	//! 1. It is a primitive or enumeration type, or
	//! 2. It is a structure or generic structure instanced type without user-provided constructor function, and 
	//! all properties of the type are trivially constructable.
	//! @param[in] type The type object.
	//! @return Returns `true` if the specified type is a trivially constructable type. 
	//! Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API bool is_type_trivially_constructable(typeinfo_t type);
	//! @brief Checks whether one type is a trivially destructable type.
	//! @details One type is trivially destructable if:
	//! 1. It is a primitive or enumeration type, or
	//! 2. It is a structure or generic structure instanced type without user-provided destructor function, and 
	//! all properties of the type are trivially destructable.
	//! @param[in] type The type object.
	//! @return Returns `true` if the specified type is a trivially destructable type. 
	//! Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API bool is_type_trivially_destructable(typeinfo_t type);
	//! @brief Checks whether one type is a trivially copy constructable type.
	//! @details One type is trivially copy constructable if:
	//! 1. It is a primitive or enumeration type, or
	//! 2. It is a structure or generic structure instanced type without user-provided copy constructor function, and 
	//! all properties of the type are trivially copy constructable.
	//! @param[in] type The type object.
	//! @return Returns `true` if the specified type is a trivially copy constructable type. 
	//! Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API bool is_type_trivially_copy_constructable(typeinfo_t type);
	//! @brief Checks whether one type is a trivially move constructable type.
	//! @details One type is trivially move constructable if:
	//! 1. It is a primitive or enumeration type, or
	//! 2. It is a structure or generic structure instanced type without user-provided move constructor function, and 
	//! all properties of the type are trivially move constructable.
	//! @param[in] type The type object.
	//! @return Returns `true` if the specified type is a trivially move constructable type. 
	//! Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API bool is_type_trivially_move_constructable(typeinfo_t type);
	//! @brief Checks whether one type is a trivially copy assignable type.
	//! @details One type is trivially copy assignable if:
	//! 1. It is a primitive or enumeration type, or
	//! 2. It is a structure or generic structure instanced type without user-provided copy assignment function, and 
	//! all properties of the type are trivially copy assignable.
	//! @param[in] type The type object.
	//! @return Returns `true` if the specified type is a trivially copy assignable type. 
	//! Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API bool is_type_trivially_copy_assignable(typeinfo_t type);
	//! @brief Checks whether one type is a trivially move assignable type.
	//! @details One type is trivially move assignable if:
	//! 1. It is a primitive or enumeration type, or
	//! 2. It is a structure or generic structure instanced type without user-provided move assignment function, and 
	//! all properties of the type are trivially move assignable.
	//! @param[in] type The type object.
	//! @return Returns `true` if the specified type is a trivially move assignable type. 
	//! Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API bool is_type_trivially_move_assignable(typeinfo_t type);
	//! @brief Checks whether one type is a trivially relocatable type.
	//! @details One type is trivially relocatable if:
	//! 1. It is a primitive or enumeration type, or
	//! 2. It is a structure or generic structure instanced type with `trivially_relocatable` set to `true`.
	//! @param[in] type The type object.
	//! @return Returns `true` if the specified type is a trivially relocatable type. 
	//! Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API bool is_type_trivially_relocatable(typeinfo_t type);

	//! @brief Constructs one instance of the specified type.
	//! @details The construction is performed as follows:
	//! 1. If `type` is a trivially constructable type, fills the memory with zeros.
	//! 2. If `type` is a non-trivially-constructable type with user defined constructor function, calls the function.
	//! 3. If `type` is a non-trivially-constructable type without user defined constructor function, constructs every 
	//! property of the structure recursively.
	//! @param[in] type The type object.
	//! @param[in] data The pointer to the instance.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `data` must specify one valid memory address.
 	LUNA_RUNTIME_API void construct_type(typeinfo_t type, void* data);
	//! @brief Constructs one array of instances of the specified type.
	//! @details The construction is performed as follows:
	//! 1. If `type` is a trivially constructable type, fills the memory with zeros.
	//! 2. If `type` is a non-trivially-constructable type with user defined constructor function, calls the function.
	//! 3. If `type` is a non-trivially-constructable type without user defined constructor function, constructs every 
	//! property of the structure recursively.
	//! @param[in] type The type object.
	//! @param[in] data The pointer to the instance array.
	//! @param[in] count The number of instances to construct.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `data` must specify one valid memory address.
	LUNA_RUNTIME_API void construct_type_range(typeinfo_t type, void* data, usize count);
	//! @brief Destructs one instance of the specified type.
	//! @details The destruction is performed as follows:
	//! 1. If `type` is a trivially destructable type, does nothing.
	//! 2. If `type` is a non-trivially-destructable type with user defined destructor function, calls the function.
	//! 3. If `type` is a non-trivially-destructable type without user defined destructor function, destructs every 
	//! property of the structure recursively.
	//! @param[in] type The type object.
	//! @param[in] data The pointer to the instance.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `data` must specify one valid memory address.
	LUNA_RUNTIME_API void destruct_type(typeinfo_t type, void* data);
	//! @brief Destructs one array of instances of the specified type.
	//! @details The destruction is performed as follows:
	//! 1. If `type` is a trivially destructable type, does nothing.
	//! 2. If `type` is a non-trivially-destructable type with user defined destructor function, calls the function.
	//! 3. If `type` is a non-trivially-destructable type without user defined destructor function, destructs every 
	//! property of the structure recursively.
	//! @param[in] type The type object.
	//! @param[in] data The pointer to the instance array.
	//! @param[in] count The number of instances to destruct.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `data` must specify one valid memory address.
	LUNA_RUNTIME_API void destruct_type_range(typeinfo_t type, void* data, usize count);
	//! @brief Copy constructs one instance of the specified type.
	//! @details The construction is performed as follows:
	//! 1. If `type` is a trivially copy constructable type, use @ref memcpy to copy instance data.
	//! 2. If `type` is a non-trivially-copy-constructable type with user defined copy constructor function, calls the function.
	//! 3. If `type` is a non-trivially-copy-constructable type without user defined copy constructor function, copy constructs every 
	//! property of the structure recursively.
	//! @param[in] type The type object.
	//! @param[in] dst The pointer to the instance to be copy constructed.
	//! @param[in] src The pointer to the instance to copy data from.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `dst` and `src` must specify one valid memory address.
	LUNA_RUNTIME_API void copy_construct_type(typeinfo_t type, void* dst, void* src);
	//! @brief Copy constructs one array of instances of the specified type.
	//! @details The construction is performed as follows:
	//! 1. If `type` is a trivially copy constructable type, use @ref memcpy to copy instance data.
	//! 2. If `type` is a non-trivially-copy-constructable type with user defined copy constructor function, calls the function.
	//! 3. If `type` is a non-trivially-copy-constructable type without user defined copy constructor function, copy constructs every 
	//! property of the structure recursively.
	//! @param[in] type The type object.
	//! @param[in] dst The pointer to the instance array to be copy constructed.
	//! @param[in] src The pointer to the instance array to copy data from.
	//! @param[in] count The number of instances to construct.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `dst` and `src` must specify one valid memory address.
	LUNA_RUNTIME_API void copy_construct_type_range(typeinfo_t type, void* dst, void* src, usize count);
	//! @brief Move constructs one instance of the specified type.
	//! @details The construction is performed as follows:
	//! 1. If `type` is a trivially move constructable type, use @ref memcpy to move instance data.
	//! 2. If `type` is a non-trivially-move-constructable type with user defined move constructor function, calls the function.
	//! 3. If `type` is a non-trivially-move-constructable type without user defined move constructor function, move constructs every 
	//! property of the structure recursively.
	//! @param[in] type The type object.
	//! @param[in] dst The pointer to the instance to be move constructed.
	//! @param[in] src The pointer to the instance to move data from.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `dst` and `src` must specify one valid memory address.
	LUNA_RUNTIME_API void move_construct_type(typeinfo_t type, void* dst, void* src);
	//! @brief Move constructs one array of instances of the specified type.
	//! @details The construction is performed as follows:
	//! 1. If `type` is a trivially move constructable type, use @ref memcpy to move instance data.
	//! 2. If `type` is a non-trivially-move-constructable type with user defined move constructor function, calls the function.
	//! 3. If `type` is a non-trivially-move-constructable type without user defined move constructor function, move constructs every 
	//! property of the structure recursively.
	//! @param[in] type The type object.
	//! @param[in] dst The pointer to the instance array to be move constructed.
	//! @param[in] src The pointer to the instance array to move data from.
	//! @param[in] count The number of instances to construct.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `dst` and `src` must specify one valid memory address.
	LUNA_RUNTIME_API void move_construct_type_range(typeinfo_t type, void* dst, void* src, usize count);
	//! @brief Copy assigns one instance of the specified type.
	//! @details The assignment is performed as follows:
	//! 1. If `type` is a trivially copy assignable type, use @ref memcpy to copy instance data.
	//! 2. If `type` is a non-trivially-copy-assignable type with user defined copy assignment function, calls the function.
	//! 3. If `type` is a non-trivially-copy-assignable type without user defined copy assignment function, copy assigns every 
	//! property of the structure recursively.
	//! @param[in] type The type object.
	//! @param[in] dst The pointer to the instance to be copy assigned.
	//! @param[in] src The pointer to the instance to copy data from.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `dst` and `src` must specify one valid memory address.
	LUNA_RUNTIME_API void copy_assign_type(typeinfo_t type, void* dst, void* src);
	//! @brief Copy assigns one array of instances of the specified type.
	//! @details The assignment is performed as follows:
	//! 1. If `type` is a trivially copy assignable type, use @ref memcpy to copy instance data.
	//! 2. If `type` is a non-trivially-copy-assignable type with user defined copy assignment function, calls the function.
	//! 3. If `type` is a non-trivially-copy-assignable type without user defined copy assignment function, copy assigns every 
	//! property of the structure recursively.
	//! @param[in] type The type object.
	//! @param[in] dst The pointer to the instance array to be copy assigned.
	//! @param[in] src The pointer to the instance array to copy data from.
	//! @param[in] count The number of instances to assign.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `dst` and `src` must specify one valid memory address.
	LUNA_RUNTIME_API void copy_assign_type_range(typeinfo_t type, void* dst, void* src, usize count);
	//! @brief Move assigns one instance of the specified type.
	//! @details The assignment is performed as follows:
	//! 1. If `type` is a trivially move assignable type, use @ref memcpy to move instance data.
	//! 2. If `type` is a non-trivially-move-assignable type with user defined move assignment function, calls the function.
	//! 3. If `type` is a non-trivially-move-assignable type without user defined move assignment function, move assigns every 
	//! property of the structure recursively.
	//! @param[in] type The type object.
	//! @param[in] dst The pointer to the instance to be move assigned.
	//! @param[in] src The pointer to the instance to move data from.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `dst` and `src` must specify one valid memory address.
	LUNA_RUNTIME_API void move_assign_type(typeinfo_t type, void* dst, void* src);
	//! @brief Move assigns one array of instances of the specified type.
	//! @details The assignment is performed as follows:
	//! 1. If `type` is a trivially move assignable type, use @ref memcpy to move instance data.
	//! 2. If `type` is a non-trivially-move-assignable type with user defined move assignment function, calls the function.
	//! 3. If `type` is a non-trivially-move-assignable type without user defined move assignment function, move assigns every 
	//! property of the structure recursively.
	//! @param[in] type The type object.
	//! @param[in] dst The pointer to the instance array to be move assigned.
	//! @param[in] src The pointer to the instance array to move data from.
	//! @param[in] count The number of instances to assign.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `dst` and `src` must specify one valid memory address.
	LUNA_RUNTIME_API void move_assign_type_range(typeinfo_t type, void* dst, void* src, usize count);
	//! @brief Relocates one instance of the specified type.
	//! @details The relocation is performed as follows:
	//! 1. If `type` is a trivially relocatable type, use @ref memcpy to move instance data.
	//! 2. If `type` is a non-trivially-relocatable type, use @ref move_construct_type to move construct one new instance on new location, 
	//! then use @ref destruct_type to destruct the old instance.
	//! @param[in] type The type object.
	//! @param[in] dst The pointer to the instance to be relocated.
	//! @param[in] src The pointer to the new location of the instance.
	LUNA_RUNTIME_API void relocate_type(typeinfo_t type, void* dst, void* src);
	//! @brief Relocates one array of instances of the specified type.
	//! @details The relocation is performed as follows:
	//! 1. If `type` is a trivially relocatable type, use @ref memcpy to move instance data.
	//! 2. If `type` is a non-trivially-relocatable type, use @ref move_construct_type to move construct one new instance on new location, 
	//! then use @ref destruct_type to destruct the old instance.
	//! @param[in] type The type object.
	//! @param[in] dst The pointer to the instance array to be relocated.
	//! @param[in] src The pointer to the new location of the instance array.
	//! @param[in] count The number of instances to relocate.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `dst` and `src` must specify one valid memory address.
	LUNA_RUNTIME_API void relocate_type_range(typeinfo_t type, void* dst, void* src, usize count);

	//! @brief The equality testing function used by the reflection system.
	//! @param[in] type The type object.
	//! @param[in] lhs The pointer to the first instance to be compared.
	//! @param[in] rhs The pointer to the second instance to be compared.
	//! @return Returns `true` if two instances are equal. Returns `false` otherwise.
	using equal_to_func_t = bool(typeinfo_t type, const void* lhs, const void* rhs);

	//! @brief Checks whether the specified type supports equality testing.
	//! @param[in] type The type object.
	//! @return Returns `true` if the specified type supports equality testing. Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	LUNA_RUNTIME_API bool is_type_equatable(typeinfo_t type);
	//! @brief Sets one type to support equality testing.
	//! @param[in] type The type object.
	//! @param[in] func The equality testing function to use.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `func` must specify one valid function.
	LUNA_RUNTIME_API void set_equatable(typeinfo_t type, equal_to_func_t* func);
	//! @brief Checks whether two instances of one type are equal.
	//! @param[in] type The type object.
	//! @param[in] lhs The pointer to the first instance to be compared.
	//! @param[in] rhs The pointer to the second instance to be compared.
	//! @return Returns `true` if two instances are equal. Returns `false` otherwise.
	//! Returns `false` if the type does not support equality testing.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	LUNA_RUNTIME_API bool equal_to_type(typeinfo_t type, const void* lhs, const void* rhs);

	//! @brief The hash code computing function used by the reflection system.
	//! @param[in] type The type of the instance.
	//! @param[in] inst The pointer to the instance.
	//! @return Returns the computed hash code.
	using hash_func_t = usize(typeinfo_t type, const void* inst);

	//! @brief Checks whether the specified type supports hash code computing.
	//! @param[in] type The type object.
	//! @return Returns `true` if the specified type supports hash code computing. Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	LUNA_RUNTIME_API bool is_type_hashable(typeinfo_t type);
	//! @brief Sets one type to support hash code computing.
	//! @param[in] type The type object.
	//! @param[in] func The hash code computing function to use.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `func` must specify one valid function.
	LUNA_RUNTIME_API void set_hashable(typeinfo_t type, hash_func_t* func);
	//! @brief Computes the hash code of one instance of the specified type.
	//! @param[in] type The type of the instance.
	//! @param[in] inst The pointer to the instance.
	//! @return Returns the computed hash code. Returns `0` if the type does not support hash code computing.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	LUNA_RUNTIME_API usize hash_type(typeinfo_t type, const void* inst);

	//! @brief Sets one attribute of the specified type.
	//! @param[in] type The type object.
	//! @param[in] name The name of the attribute to set.
	//! @param[in] value The value of the attribute.
	//! @remark If `type` specifies one generic type, the attribute will be applied to all instanced types of that generic type.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	//! * `name` must not be empty.
	LUNA_RUNTIME_API void set_type_attribute(typeinfo_t type, const Name& name, const Variant& value = Variant());
	//! @brief Removes one attribute of the specified type.
	//! @param[in] type The type object.
	//! @param[in] name The name of the attribute to remove.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	//! * `name` must not be empty.
	LUNA_RUNTIME_API void remove_type_attribute(typeinfo_t type, const Name& name);
	//! @brief Checks whether the attribute of the specified type exists.
	//! @param[in] type The type object.
	//! @param[in] name The name of the attribute to check.
	//! @return Returns `true` if the attribute exists. Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	//! * `name` must not be empty.
	LUNA_RUNTIME_API bool check_type_attribute(typeinfo_t type, const Name& name);
	//! @brief Gets the attribute of the specified type.
	//! @param[in] type The type object.
	//! @param[in] name The name of the attribute to get.
	//! @return Returns the requested attribute. Returns one null variant if the attribute does not exist.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	//! * `name` must not be empty.
	LUNA_RUNTIME_API Variant get_type_attribute(typeinfo_t type, const Name& name);
	//! @brief Gets all attributes of the specified type.
	//! @param[in] type The type object.
	//! @return Returns one vector that contains attribute names of all attributes of the specified type.
	//! @par Valid Usage
	//! * `type` must specify one valid type object.
	LUNA_RUNTIME_API Vector<Name> get_type_attributes(typeinfo_t type);

	//! @brief Sets one attribute of the specified property.
	//! @param[in] type The type object.
	//! @param[in] property The property name.
	//! @param[in] name The name of the attribute to set.
	//! @param[in] value The value of the attribute.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `property` must not be empty.
	//! * `name` must not be empty.
	LUNA_RUNTIME_API void set_property_attribute(typeinfo_t type, const Name& property, const Name& name, const Variant& value = Variant());
	//! @brief Removes one attribute of the specified property.
	//! @param[in] type The type object.
	//! @param[in] property The property name.
	//! @param[in] name The name of the attribute to remove.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `property` must not be empty.
	//! * `name` must not be empty.
	LUNA_RUNTIME_API void remove_property_attribute(typeinfo_t type, const Name& property, const Name& name);
	//! @brief Checks whether the attribute of the specified property exists.
	//! @param[in] type The type object.
	//! @param[in] property The property name.
	//! @param[in] name The name of the attribute to check.
	//! @return Returns `true` if the attribute exists. Returns `false` otherwise.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `property` must not be empty.
	//! * `name` must not be empty.
	LUNA_RUNTIME_API bool check_property_attribute(typeinfo_t type, const Name& property, const Name& name);
	//! @brief Gets the attribute of the specified property.
	//! @param[in] type The type object.
	//! @param[in] property The property name.
	//! @param[in] name The name of the attribute to get.
	//! @return Returns the requested attribute. Returns one null variant if the attribute does not exist.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `property` must not be empty.
	//! * `name` must not be empty.
	LUNA_RUNTIME_API Variant get_property_attribute(typeinfo_t type, const Name& property, const Name& name);
	//! @brief Gets all attributes of the specified property.
	//! @param[in] type The type object.
	//! @param[in] property The property name.
	//! @return Returns one vector that contains attribute names of all attributes of the specified property.
	//! @par Valid Usage
	//! * `type` must specify one valid type object and cannot be a generic structure type.
	//! * `property` must not be empty.
	LUNA_RUNTIME_API Vector<Name> get_property_attributes(typeinfo_t type, const Name& property);

	// Structure and enumeration type registration.

	struct StructurePropertyDesc
	{
		Name name;
		typeinfo_t type;
		usize offset;
		StructurePropertyDesc() = default;
		StructurePropertyDesc(const Name& name, typeinfo_t type, usize offset) :
			name(name),
			type(type),
			offset(offset) {}
	};
	struct EnumerationOptionDesc
	{
		Name name;
		i64 value;
		EnumerationOptionDesc() = default;
		EnumerationOptionDesc(const Name& name, i64 value) :
			name(name),
			value(value) {}
	};

	using structure_ctor_t = void(typeinfo_t type, void* inst);
	using structure_dtor_t = void(typeinfo_t type, void* inst);
	using structure_copy_ctor_t = void(typeinfo_t type, void* dst, void* src);
	using structure_move_ctor_t = void(typeinfo_t type, void* dst, void* src);
	using structure_copy_assign_t = void(typeinfo_t type, void* dst, void* src);
	using structure_move_assign_t = void(typeinfo_t type, void* dst, void* src);

	struct StructureTypeDesc
	{
		Guid guid;
		Name name;
		Name alias;
		//! The size of the structure type, this should include the size for the base type of this type.
		usize size;
		//! The alignment of the structure type.
		usize alignment;
		//! The base type of this structure type.
		typeinfo_t base_type = nullptr;
		//! The constructor function for this type. If `nullptr`, the default constructor will
		//! be used.
		structure_ctor_t* ctor = nullptr;
		//! The destructor function for this type. If `nullptr`, the default destructor will be used.
		structure_dtor_t* dtor = nullptr;
		//! The copy constructor for this type. If `nullptr`, the default copy constructor will
		//! be used.
		structure_copy_ctor_t* copy_ctor = nullptr;
		//! The move constructor for this type. If `nullptr`, the default move constructor will be used.
		structure_move_ctor_t* move_ctor = nullptr;
		//! The copy assignment operator for this type. If `nullptr`, the default copy assignment operator will be used.
		structure_copy_assign_t* copy_assign = nullptr;
		//! The mvoe assignment operator for this type. If `nullptr`, the default move assignment operator will be used.
		structure_move_assign_t* move_assign = nullptr;
		//! The properties of this structure type.
		Vector<StructurePropertyDesc> properties;
		//! Whether this structure is trivially relocatable.
		bool trivially_relocatable = true;
	};

	struct GenericStructureInstantiateInfo
	{
		//! The size of the structure type, this should include the size for the base type of this type.
		usize size;
		//! The alignment of the structure type.
		usize alignment;
		//! The base type of this structure type.
		typeinfo_t base_type = nullptr;
		//! The constructor function for this type. If `nullptr`, the default constructor will
		//! be used.
		structure_ctor_t* ctor = nullptr;
		//! The destructor function for this type. If `nullptr`, the default destructor will be used.
		structure_dtor_t* dtor = nullptr;
		//! The copy constructor for this type. If `nullptr`, the default copy constructor will
		//! be used.
		structure_copy_ctor_t* copy_ctor = nullptr;
		//! The move constructor for this type. If `nullptr`, the default move constructor will be used.
		structure_move_ctor_t* move_ctor = nullptr;
		//! The copy assignment operator for this type. If `nullptr`, the default copy assignment operator will be used.
		structure_copy_assign_t* copy_assign = nullptr;
		//! The mvoe assignment operator for this type. If `nullptr`, the default move assignment operator will be used.
		structure_move_assign_t* move_assign = nullptr;
		//! The properties of this structure type.
		Vector<StructurePropertyDesc> properties;
		//! Whether this structure is trivially relocatable.
		bool trivially_relocatable = true;
	};

	using generic_structure_instantiate_t = GenericStructureInstantiateInfo(typeinfo_t generic_type, const typeinfo_t* generic_arguments, usize num_generic_arguments);

	struct GenericStructureTypeDesc
	{
		Guid guid;
		Name name;
		Name alias;
		//! The names for every generic parameter.
		Vector<Name> generic_parameter_names;
		//! Whether this type suports variable number of generic arguments.
		//! If this is `true`, the user may specify zero, one or more generic arguments after arguments specified in `generic_argument_names`.
		bool variable_generic_parameters;
		//! The function used to create generic instants for this generic type.
		generic_structure_instantiate_t* instantiate;
	};

	struct EnumerationTypeDesc
	{
		Guid guid;
		Name name;
		Name alias;
		typeinfo_t underlying_type;
		Vector<EnumerationOptionDesc> options;
		bool multienum;
	};

	LUNA_RUNTIME_API typeinfo_t register_struct_type(const StructureTypeDesc& desc);
	LUNA_RUNTIME_API typeinfo_t register_generic_struct_type(const GenericStructureTypeDesc& desc);
	LUNA_RUNTIME_API typeinfo_t register_enum_type(const EnumerationTypeDesc& desc);

	LUNA_RUNTIME_API usize count_struct_properties(typeinfo_t type);
	LUNA_RUNTIME_API StructurePropertyDesc get_struct_property(typeinfo_t type, usize index);

	//! Returns the base type of the specified type.
	//! Returns `nullptr` if the specified type is not a structure type, or if the type
	//! does not have a base type.
	LUNA_RUNTIME_API typeinfo_t get_base_type(typeinfo_t type);
	LUNA_RUNTIME_API usize count_enum_options(typeinfo_t type);
	LUNA_RUNTIME_API EnumerationOptionDesc get_enum_option(typeinfo_t type, usize index);
	LUNA_RUNTIME_API typeinfo_t get_enum_underlying_type(typeinfo_t type);
	LUNA_RUNTIME_API bool is_multienum_type(typeinfo_t type);

	//! Extracts the mapped value of the enumeration, regardless of the underlying type of the enumeration.
	LUNA_RUNTIME_API i64 get_enum_instance_value(typeinfo_t type, const void* data);
	LUNA_RUNTIME_API void set_enum_instance_value(typeinfo_t type, void* data, i64 value);

	LUNA_RUNTIME_API typeinfo_t get_struct_generic_type(typeinfo_t type);
	LUNA_RUNTIME_API usize count_struct_generic_arguments(typeinfo_t type);
	LUNA_RUNTIME_API typeinfo_t get_struct_generic_argument(typeinfo_t type, usize index);
	LUNA_RUNTIME_API usize count_struct_generic_parameters(typeinfo_t type);
	LUNA_RUNTIME_API Name get_struct_generic_parameter_name(typeinfo_t type, usize index);

	template <typename _Ty>
	inline bool default_equal_to(typeinfo_t type, const void* lhs, const void* rhs)
	{
		return equal_to<_Ty>()(*((const _Ty*)lhs), *((const _Ty*)rhs));
	}
	template <typename _Ty>
	inline usize default_hash(typeinfo_t type, const void* inst)
	{
		return hash<_Ty>()(*((const _Ty*)inst));
	}
	template <typename _Ty>
	inline void default_ctor(typeinfo_t type, void* inst)
	{
		value_construct((_Ty*)inst);
	}
	template <typename _Ty>
	inline void default_dtor(typeinfo_t type, void* inst)
	{
		((_Ty*)inst)->~_Ty();
	}
	template <typename _Ty>
	inline void default_copy_ctor(typeinfo_t type, void* dst, void* src)
	{
		copy_construct((_Ty*)dst, (_Ty*)src);
	}
	template <typename _Ty>
	inline void default_move_ctor(typeinfo_t type, void* dst, void* src)
	{
		move_construct((_Ty*)dst, (_Ty*)src);
	}
	template <typename _Ty>
	inline void default_copy_assign(typeinfo_t type, void* dst, void* src)
	{
		copy_assign((_Ty*)dst, (_Ty*)src);
	}
	template <typename _Ty>
	inline void default_move_assign(typeinfo_t type, void* dst, void* src)
	{
		move_assign((_Ty*)dst, (_Ty*)src);
	}
	struct MemoryLayoutMember
	{
		usize size;		// in
		usize alignment;// in
		usize offset;	// out.
		MemoryLayoutMember() = default;
		MemoryLayoutMember(usize size, usize alignment) :
			size(size),
			alignment(alignment) {}
	};
	inline void calculate_struct_memory_layout(MemoryLayoutMember* first, MemoryLayoutMember* last, usize& out_size, usize& out_alignment)
	{
		usize size = 0;
		usize alignment = 0;
		for (auto i = first; i != last; ++i)
		{
			size = align_upper(size, i->alignment);
			i->offset = size;
			size += i->size;
			alignment = max(alignment, i->alignment);
		}
		size = align_upper(size, alignment);
		out_size = size;
		out_alignment = alignment;
	}
	inline void calculate_union_memory_layout(MemoryLayoutMember* first, MemoryLayoutMember* last, usize& out_size, usize& out_alignment)
	{
		usize size = 0;
		usize alignment = 0;
		for (auto i = first; i != last; ++i)
		{
			i->offset = 0;
			size = max(size, i->size);
			alignment = max(alignment, i->alignment);
		}
		size = align_upper(size, alignment);
		out_size = size;
		out_alignment = alignment;
	}

	template <typename _Ty>
	typeinfo_t register_struct_type(InitializerList<StructurePropertyDesc> properties, typeinfo_t base_type = nullptr)
	{
		StructureTypeDesc desc;
		desc.guid = _Ty::__guid;
		desc.name = _Ty::__name;
		desc.alias = Name();
		desc.base_type = base_type;
		desc.size = sizeof(_Ty);
		desc.alignment = alignof(_Ty);
		desc.ctor = is_trivially_constructible_v<_Ty> ? nullptr : default_ctor<_Ty>;
		desc.dtor = is_trivially_destructible_v<_Ty> ? nullptr : default_dtor<_Ty>;
		desc.copy_ctor = is_trivially_copy_constructible_v<_Ty> ? nullptr : default_copy_ctor<_Ty>;
		desc.move_ctor = is_trivially_move_constructible_v<_Ty> ? nullptr : default_move_ctor<_Ty>;
		desc.copy_assign = is_trivially_copy_assignable_v<_Ty> ? nullptr : default_copy_assign<_Ty>;
		desc.move_assign = is_trivially_move_assignable_v<_Ty> ? nullptr : default_move_assign<_Ty>;
		desc.properties = properties;
		desc.trivially_relocatable = is_trivially_relocatable_v<_Ty>;
		return register_struct_type(desc);
	}

	template <typename _Ty>
	typeinfo_t register_enum_type(InitializerList<EnumerationOptionDesc> options, bool multienum = false)
	{
		EnumerationTypeDesc desc;
		desc.guid = EnumTypeInfo<_Ty>::__guid;
		desc.name = EnumTypeInfo<_Ty>::__name;
		desc.alias = Name();
		desc.underlying_type = typeof<underlying_type_t<_Ty>>();
		desc.options = options;
		desc.multienum = multienum;
		return register_enum_type(desc);
	}

	//! @}
}