/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Name.hpp
* @author JXMaster
* @date 2020/8/7
* @brief Runtime name string APIs.
*/
#pragma once
#include "String.hpp"
#include "Functional.hpp"

#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif

namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeName Name strings
	//! @}

	//! @addtogroup RuntimeName
	//! @{
	
	//! The name ID type.
	using name_id_t = u32;

	//! Interns one name string to the runtime and fetches the interned address for it.
	//! @param[in] name The name string to intern.
	//! @return Returns the interned address for the name string. If `name` is `nullptr` or points to an empty string (`""`), the returned 
	//! address is `nullptr` and the memory block is not interned.
	//! @remark The name string is saved in the runtime and is reference counted. The first call to @ref intern_name with a new string
	//! allocates the memory block to store the string and returns the block address. Additional calls to @ref intern_name with the same string
	//! will only increase the reference count of the memory block and returns the same address, so the user can simply compares
	//! two address (pointer) to check if they refer to the same string.
	//! 
	//! For each call to @ref intern_name, one call to @ref release_name is needed to finally release the internal name string block.
	//! 
	//! For end user, prefer using @ref Name objects instead of calling these APIs directly.
	//! @par Valid Usage
	//! * If `name` is not `nullptr`, it must be ended with one null terminator.
	LUNA_RUNTIME_API const c8* intern_name(const c8* name);

	//! Interns one name string to the runtime and fetches the interned address for it.
	//! @param[in] name The name string to intern.
	//! @param[in] count The number of characters that should be copied in `name`, excluding the null terminator if any.
	//! @return Returns the interned address for the name string. If `name` is `nullptr` or `size` is `0`, the returned 
	//! address is `nullptr` and the memory block is not interned.
	//! @remark The name string is saved in the runtime and is reference counted. The first call to @ref intern_namewith a new string
	//! allocates the memory block to store the string and returns the block address. Additional calls to @ref intern_name with the same string
	//! will only increase the reference count of the memory block and returns the same address, so the user can simply compares
	//! two address (pointer) to check if they refer to the same string.
	//! 
	//! For each call to @ref intern_name, one call to @ref release_name is needed to finally release the internal name string block.
	//! 
	//! For end user, prefer using @ref Name objects instead of calling these APIs directly.
	LUNA_RUNTIME_API const c8* intern_name(const c8* name, usize count);

	//! Increases the reference count of the name string by 1.
	//! @param[in] name The pointer of the string. If this is `nullptr`, this call does nothing.
	//! @par Valid Usage
	//! * If `name` is not `nullptr`, it must be a string pointer returned by @ref intern_name.
	LUNA_RUNTIME_API void retain_name(const c8* name);

	//! Decreases the reference count of the name string by 1, and eventually frees the name string when the reference count goes to 0.
	//! @param[in] name The pointer of the string. If this is `nullptr`, this call does nothing.
	//! @par Valid Usage
	//! * If `name` is not `nullptr`, it must be a string pointer returned by @ref intern_name.
	LUNA_RUNTIME_API void release_name(const c8* name);

	//! Gets the ID for the specified name. The ID keeps constant between multiple processes.
	//! @details Since the name ID is hashed from the name string, technically multiple names may have the same ID. 
	//! The name system handles such confliction, so that different name string will always have different pointer, even they have the same ID.
	//! The user should compare the string pointer returned by @ref intern_name rather than the string ID to check whether two strings are equal.
	//! @param[in] name The pointer of the string. If this is `nullptr`, this call returns `0`.
	//! @par Valid Usage
	//! * If `name` is not `nullptr`, it must be a string pointer returned by @ref intern_name.
	LUNA_RUNTIME_API name_id_t get_name_id(const c8* name);

	//! Fetches the size of the name string.
	//! @param[in] name The pointer of the string.
	//! @return Returns the size of the name. Returns `0` if `name` is `nullptr`.
	//! @remark Note that the size returned is not always equal to the size returnd by `strlen`, since the string that contains the null terminator 
	//! is allowed as a name, so always fetches the name size using this API.
	//!  
	//! The size of the name is cached in the system, so this call returns in constant time.
	//! @par Valid Usage
	//! * If `name` is not `nullptr`, it must be a string pointer returned by @ref intern_name.
	LUNA_RUNTIME_API usize get_name_size(const c8* name);

	//! Represents one name string.
	//! @details The name string is one constant string that is mainly used to identify entities in LunaSDK. Name strings 
	//! are reference counted and managed by system, all @ref Name objects containing the same name string will refer the same name
	//! internal string data, enabling fast comparison: instead of comparing the whole string, we only need to compare one pointer to 
	//! determine whether two name strings are the same.
	class Name
	{
	private:
		const c8* m_str;
	public:
		//! Constructs one empty name.
		Name() :
			m_str(nullptr) {}
		//! Constructs one name with the provided name string.
		//! @param[in] name The name string.
		//! @par Valid Usage
		//! * If `name` is not `nullptr`, it must be null-terminated.
		Name(const c8* name) :
			m_str(intern_name(name)) {}
		//! Constructs one name with the provided name string and size.
		//! @param[in] name The name string.
		//! @param[in] count The number of characters in the string used to create the name.
		Name(const c8* name, usize count) :
			m_str(intern_name(name, count)) {}
		//! Constructs one name from one string.
		//! @param[in] str The name string.
		Name(const String& str) :
			m_str(intern_name(str.c_str())) {}
		//! Constructs one name from one substring of the provided string.
		//! @param[in] str The name string.
		//! @param[in] pos The first character used for the name.
		//! @param[in] count The number of characters used for the name.
		Name(const String& str, usize pos, usize count) :
			m_str(intern_name(str.c_str() + pos, count)) {}
		~Name()
		{
			release_name(m_str);
		}
		Name(const Name& rhs) :
			m_str(rhs.m_str)
		{
			retain_name(m_str);
		}
		Name(Name&& rhs) :
			m_str(rhs.m_str)
		{
			rhs.m_str = nullptr;
		}
		Name& operator=(const Name& rhs)
		{
			release_name(m_str);
			m_str = rhs.m_str;
			retain_name(m_str);
			return *this;
		}
		Name& operator=(Name&& rhs)
		{
			release_name(m_str);
			m_str = rhs.m_str;
			rhs.m_str = nullptr;
			return *this;
		}
		Name& operator=(const c8* name)
		{
			release_name(m_str);
			m_str = intern_name(name);
			return *this;
		}
		Name& operator=(const String& str)
		{
			release_name(m_str);
			m_str = intern_name(str.c_str());
			return *this;
		}
		//! Gets the internal string pointer of this name.
		//! @return Returns the internal string pointer of this name, returns one empty string if this is a null name.
		const c8* c_str() const
		{
			return m_str ? m_str : u8"";
		}
		//! Gets the size of the name string.
		//! @return Returns the size of the name string.
		usize size() const
		{
			return get_name_size(m_str);
		}
		//! Gets the ID of the name string.
		//! @return Returns the ID of the name string.
		name_id_t id() const
		{
			return get_name_id(m_str);
		}
		//! Checks whether this name string is empty.
		//! @return Returns `true` if this name string is empty, returns `false` otherwise.
		bool empty() const
		{
			return m_str == nullptr;
		}
		//! Checks whether this name string is empty.
		//! @return Returns `true` if this name string is empty, returns `false` otherwise.
		operator bool() const
		{
			return !empty();
		}
		//! Clears the name string and resets it to one empty name.
		void reset()
		{
			release_name(m_str);
			m_str = nullptr;
		}
		//! Compares two names for equality.
		//! @param[in] rhs The name to compare with.
		//! @return Returns `true` if two names are equal, returns `false` otherwise.
		bool operator==(const Name& rhs) const
		{
			return m_str == rhs.m_str;
		}
		//! Compares two names for non-equality.
		//! @param[in] rhs The name to compare with.
		//! @return Returns `true` if two names are not equal, returns `false` otherwise.
		bool operator!=(const Name& rhs) const
		{
			return m_str != rhs.m_str;
		}
		//! Converts one name to one string.
		//! @return Returns one string containing name characters.
		operator String() const
		{
			return String(c_str(), get_name_size(m_str));
		}
	};

	template <> struct hash<Name>
	{
		usize operator()(const Name& val) const { return static_cast<usize>(val.id()); }
	};

	//! Gets the type object of @ref Name.
	//! @return Returns the type object of @ref Name.
	LUNA_RUNTIME_API typeinfo_t name_type();
	template <> struct typeof_t<Name> { typeinfo_t operator()() const { return name_type(); } };

	//! @}
}