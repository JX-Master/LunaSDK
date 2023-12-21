/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Path.hpp
* @author JXMaster
* @date 2020/8/12
*/
#pragma once
#include "Name.hpp"
#include "Vector.hpp"
#include "Memory.hpp"

namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimePath Path
	//! @}

	//! @addtogroup RuntimePath
    //! @{
	
	//! Separators used when encoding @ref Path into strings.
	enum class PathSeparator : u32
	{
		//! Uses slash (/). 
		slash = 0,
		//! Uses the system preferred separator (back-slash on Windows, slash on other platforms).
		system_preferred = 1,
		//! Uses back-slash(\).
		back_slash = 2
	};

	//! Additional flags used by @ref Path.
	enum class PathFlag : u32
	{
		none = 0x00,
		absolute = 0x01,	// The path is absolute, if this is not set, then the path is relative.
	};

	//! Describes components of @ref Path.
	enum class PathComponent : u32
	{
		none = 0x00,
		root = 0x01,
		nodes = 0x02,
		flags = 0x04,
		all = root | nodes | flags
	};

	namespace PathImpl
	{
		inline bool is_separator(c8 ch)
		{
			return (ch == '\\' || ch == '/') ? true : false;
		}

		inline void get_path_root_name_length(const c8* path, usize slen, usize& len)
		{
			if (is_separator(path[0]) && slen >= 2)
			{
				if (is_separator(path[1]))
				{
					// Remote Computer.
					usize i = 2;
					while (path[i] && !is_separator(path[i]))
					{
						++i;
					}
					len = i;
					return;
				}
				else
				{
					// UNIX/Linux Root directory.
					len = 0;
					return;
				}
			}
			if (path[1] == ':' && isalpha(path[0]) && slen >= 2)
			{
				// Windows disk.
				len = 2;
				return;
			}
			len = 0;
		}

		inline c8 get_preferred_separator(PathSeparator sep)
		{
			switch (sep)
			{
			case PathSeparator::system_preferred:
#ifdef LUNA_PLATFORM_WINDOWS
				return '\\';
#else
				return '/';
#endif
			case PathSeparator::back_slash:
				return '\\';
			case PathSeparator::slash:
			default:
				return '/';
			}
		}

		//! Fetches the next node from the string. The node ends with a separator or end of the string.
		inline usize get_next_node(const c8* p, usize len)
		{
			usize i = 0;
			while ((i < len) && !is_separator(p[i]))
			{
				++i;
			}
			return i;
		}
	}

	//! A container that contains a sequence of names that describe one path.
	//! @details Path is one kind of string that describes the location of one node in a hierarchical-based node tree, given that each
	//! node in the tree can be identified by a name string. For example, a file path is used to identify a file or folder in the given 
	//! file system.
	//! 
	//! @ref Path is designed to be platform-independent and efficient to handle file path related operations.
	//! In implementation, the path object does not store the path string directly, but breaks it down to several parts, and stores
	//! each part independently. This makes path-related operations very fast and consumes less memory if you need to store lots of 
	//! paths.
	//! 
	//! One path is composed by the following components:
	//! 1. The root name, which usually determines the domain of the path. For example, then volume symbol on Windows (like C:) is 
	//! one kind of root name.
	//! 2. The directory nodes that composes the path. For example, "C:\Games\MyGame\" has root name "C:" and two directory nodes 
	//! "Games" and "MyGame". In file object, every directory node as well as the root name is stored independently as one Name  
	//! object, and they are grouped into one array to form the path sequence.
	//! 3. The path flags, see `EPathFlag` for details. Basically, path object uses flags to determine if one path is absolute 
	//! (if begins with one separator), and if one path represents a directory (if it ends with one separator). This flags are properly
	//! set when the path string gets parsed, but it may not be correct. For example, if you parse one path string that represents 
	//! a directory but does not ends with a separator, the `EPathFlag::directory` will not be set for that path. The path object will
	//! not use runtime system calls like `file_attribute` to determine if one path is valid or represents a directory, it is the user's
	//! responsibility to check it before using it.
	class Path
	{
		Vector<Name> m_nodes;
		Name m_root;
		PathFlag m_flags;

	public:

		using reference = Name&;
		using const_reference = const Name&;
		using pointer = Name*;
		using const_pointer = const Name*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = ReverseIterator<iterator>;
		using const_reverse_iterator = ReverseIterator<const_iterator>;
		
		//! Constructs one empty path.
		Path() :
			m_flags(PathFlag::none) {}
		//! Constructs one path by parsing the specified path string.
		//! @param[in] str The path string.
		Path(const String& str) :
			m_flags(PathFlag::none)
		{
			assign(str);
		}
		//! Constructs one path by parsing the specified path string with custom starting position.
		//! @param[in] str The path string.
		//! @param[in] pos The index of the first character to parse.
		Path(const String& str, usize pos) :
			m_flags(PathFlag::none)
		{
			assign(str, pos);
		}
		//! Constructs one path by parsing the specified path string with custom starting position and string size.
		//! @param[in] str The path string.
		//! @param[in] pos The index of the first character to parse.
		//! @param[in] count The number of characters to parse.
		Path(const String& str, usize pos, usize count) :
			m_flags(PathFlag::none)
		{
			assign(str, pos, count);
		}
		//! Constructs one path by parsing the specified path string.
		//! @param[in] s The path string.
		//! @par Valid Usage
		//! * `s` must specifies one null-terminated string.
		Path(const c8* s) :
			m_flags(PathFlag::none)
		{
			assign(s);
		}
		//! Constructs one path by parsing the specified path string.
		//! @param[in] s The path string.
		//! @param[in] count The number of characters to parse.
		Path(const c8* s, usize count) :
			m_flags(PathFlag::none)
		{
			assign(s, count);
		}
		//! Constructs one path by moving coping content from another path.
		//! @param[in] rhs The path to copy from.
		Path(const Path& rhs) :
			m_flags(PathFlag::none)
		{
			assign(rhs);
		}
		//! Constructs one path by moving moving content from another path.
		//! @param[in] rhs The path to move from.
		Path(Path&& rhs) :
			m_nodes(move(rhs.m_nodes)),
			m_flags(rhs.m_flags),
			m_root(move(rhs.m_root))
		{
			rhs.m_flags = PathFlag::none;
		}
		//! Replaces content of the path by parsing the specified path string.
		//! @param[in] str The path string.
		//! @return Returns `*this`.
		Path& operator=(const String& str)
		{
			assign(str);
			return *this;
		}
		//! Replaces content of the path by parsing the specified path string.
		//! @param[in] s The path string.
		//! @return Returns `*this`.
		Path& operator=(const c8* s)
		{
			assign(s);
			return *this;
		}
		//! Replaces content of the path by coping content from another path.
		//! @param[in] rhs The path to copy from.
		//! @return Returns `*this`.
		Path& operator=(const Path& rhs)
		{
			assign(rhs);
			return *this;
		}
		//! Replaces content of the path by coping content from another path.
		//! @param[in] rhs The path to move from.
		//! @return Returns `*this`.
		Path& operator=(Path&& rhs)
		{
			assign(move(rhs));
			return *this;
		}
		//! Gets the path flags.
		//! @return Returns a copy of the path flags.
		PathFlag flags() const
		{
			return m_flags;
		}
		//! Gets the path flags.
		//! @return Returns a reference of the path flags.
		PathFlag& flags()
		{
			return m_flags;
		}
		//! Normalizes the path. 
		//! @details This call remove all unneeded ".." and "." nodes from the path.
		void normalize()
		{
			auto iter = m_nodes.begin();
			while (iter != m_nodes.end())
			{
				if ((!strcmp((*iter).c_str(), "..")) && (iter != m_nodes.begin()))
				{
					auto iter2 = iter;
					--iter2;
					if (strcmp((*iter2).c_str(), ".."))
					{
						iter = m_nodes.erase(iter2, iter2 + 2);
					}
				}
				else if ((!strcmp((*iter).c_str(), ".")))
				{
					iter = m_nodes.erase(iter, iter + 1);
				}
				else
				{
					++iter;
				}
			}
		}
		//! Encodes the current path to a string.
		//! @param[in] separator The separator format to use. Default is slash since it is well supported by all major platforms.
		//! @param[in] has_root Whether to add root name to the path string.
		//! @return Returns the encoded path string.
		String encode(PathSeparator separator = PathSeparator::slash, bool has_root = true) const
		{
			String buf;
			if (m_root && has_root)
			{
				buf.append(m_root.c_str());
			}
			// Append '/' for root.
			if (test_flags(flags(), PathFlag::absolute))
			{
				buf.push_back(PathImpl::get_preferred_separator(separator));
			}
			// Append '.' if not root and is empty.
			else if (m_nodes.empty())
			{
				buf.push_back('.');
			}
			for (usize i = 0; i < m_nodes.size(); ++i)
			{
				buf.append(m_nodes[i].c_str());
				if (i != m_nodes.size() - 1)
				{
					buf.push_back(PathImpl::get_preferred_separator(separator));
				}
			}
			return buf;
		}
		//! Replaces content of the path by coping content from another path.
		//! @param[in] rhs The path to copy from.
		void assign(const Path& rhs)
		{
			m_flags = rhs.m_flags;
			m_nodes = rhs.m_nodes;
			m_root = rhs.m_root;
		}
		//! Replaces content of the path by coping content from another path.
		//! @param[in] rhs The path to move from.
		void assign(Path&& rhs)
		{
			m_flags = rhs.m_flags;
			m_nodes = move(rhs.m_nodes);
			m_root = move(rhs.m_root);
			rhs.m_flags = PathFlag::none;
		}
		//! Replaces content of the path by parsing the specified path string.
		//! @param[in] str The path string.
		//! @remark Currently only the following root string will be recognized:
		//! * Windows volume: C:, D: etc.
		//! * Remove Server: //My_Server or \\My_Server or IP-address format(\\192.168.31.1)
		void assign(const String& str)
		{
			assign(str.c_str(), str.size());
		}
		//! Replaces content of the path by parsing the specified path string with custom starting position.
		//! @param[in] str The path string.
		//! @param[in] pos The index of the first character to parse.
		void assign(const String& str, usize pos)
		{
			assign(str.c_str() + pos, str.size() - pos);
		}
		//! CReplaces content of the path by parsing the specified path string with custom starting position and string size.
		//! @param[in] str The path string.
		//! @param[in] pos The index of the first character to parse.
		//! @param[in] count The number of characters to parse.
		void assign(const String& str, usize pos, usize count)
		{
			assign(str.c_str() + pos, count);
		}
		//! Replaces content of the path by parsing the specified path string.
		//! @param[in] s The path string.
		//! @par Valid Usage
		//! * `s` must be null-terminated.
		void assign(const c8* s)
		{
			if (s) assign(s, strlen(s));
			else reset();
		}
		//! Replaces content of the path by parsing the specified path string.
		//! @param[in] s The path string.
		//! @param[in] count The number of characters to parse.
		void assign(const c8* s, usize count)
		{
			if (!s)
			{
				reset();
				return;
			}
			m_nodes.clear();
			usize cur = 0;
			// Parse root path.
			usize rl;
			PathImpl::get_path_root_name_length(s, count, rl);
			if (rl)
			{
				m_root = Name(s, rl);
			}
			else
			{
				m_root = nullptr;
			}
			cur += rl;
			// Check absolute path.
			m_flags = PathFlag::none;
			if (PathImpl::is_separator(s[cur]))
			{
				m_flags |= PathFlag::absolute;
				cur += 1;
			}
			else
			{
				m_flags &= ~(PathFlag::absolute);
			}
			// Parse nodes.
			while (cur < count)
			{
				usize i = PathImpl::get_next_node(s + cur, count - cur);
				if (!i)
				{
					++cur;
					continue;
				}
				m_nodes.push_back(Name(s + cur, i));
				cur += i + 1;
			}
			normalize();
		}
		//! Assigns the content of this path with a new path that if appended to `base` path, 
		//! creates a path equal to `target` path.
		//! @param[in] base The base path.
		//! @param[in] target The target path.
		//! @return If the system can decide how to redirect target path to this path, returns the relative path,
		//! else, returns failure.
		//! @remark In order to let this function succeeds, the following conditions are required:
		//! 1. The root name of this path and target path must be equal, either `nullptr` or points to the same `Name`
		//! object.
		//! 2. Both path should either all be absolute or all be relative.
		//! The returned path has the same root name as this path, and will always be a relative path. If this path is
		//! a directory, then the returned path will be marked as directory. The target path will always be recognized as
		//! directory, no matter if it is marked as directory.
		void assign_relative(const Path& base, const Path& target)
		{
			luassert_msg(base.root() == target.root(), "The root name for base and target path must be equal.");
			luassert_msg((base.flags() & PathFlag::absolute) == (target.flags() & PathFlag::absolute), "The base and target path must all be absolute or relative");
			clear();
			// Finds the common prefix.
			usize diff_begin;
			usize nodes = min(base.size(), target.size());
			for (diff_begin = 0; diff_begin < nodes; ++diff_begin)
			{
				if (base[diff_begin] != target[diff_begin])
				{
					break;
				}
			}
			m_flags = PathFlag::none;
			m_root = base.root();
			auto dd = Name("..");
			for (usize i = diff_begin; i < base.size(); ++i)
			{
				m_nodes.push_back(dd);
			}
			dd = nullptr;
			for (usize i = diff_begin; i < target.size(); ++i)
			{
				m_nodes.push_back(target[i]);
			}
		}

		//! Gets the path root name.
		//! @return Returns a constant reference to the path root name.
		const Name& root() const
		{
			return m_root;
		}
		//! Gets the path root name.
		//! @return Returns a reference to the path root name.
		Name& root()
		{
			return m_root;
		}
		//! Gets the extension name of the path, that is, the name string after the last dot(.) character.
		//! @return Returns the extension name of the path. The extension is always in lower case.
		//! Returns an empty name if the path does not have an extension name.
		Name extension() const
		{
			if (m_nodes.empty())
			{
				return Name();
			}
			auto& name = m_nodes.back();
			const c8* str = name.c_str();
			usize sz = name.size();
			usize i = sz - 1;	// points to the last valid char.
			while (i)
			{
				if (str[i] == '.')
				{
					// translate to lower case.
					usize ext_sz = sz - i;
					if (ext_sz == 1)
					{
						return Name("");
					}
					c8* buf = (c8*)alloca(sizeof(c8) * ext_sz);
					memcpy(buf, str + i + 1, ext_sz * sizeof(c8));
					for (usize j = 0; j < ext_sz - 1; ++j)
					{
						buf[j] = (c8)tolower(buf[j]);
					}
					return Name(buf);
				}
				--i;
			}
			return Name("");
		}
		//! Gets the filename of the path, which is the last node in the path excluding extension and the separating dot(`.`).
		//! @return Returns the filename of the path.
		//! Returns an empty name if the path is empty.
		Name filename() const
		{
			if (m_nodes.empty())
			{
				return Name();
			}
			auto& name = m_nodes.back();
			const c8* str = name.c_str();
			usize sz = name.size();
			usize i = sz - 1;	// points to the last valid char.
			while (i)
			{
				if (str[i] == '.')
				{
					// translate to lower case.
					usize filename_sz = i;
					if (!filename_sz)
					{
						return Name("");
					}
					c8* buf = (c8*)alloca(sizeof(c8) * filename_sz);
					memcpy(buf, str, filename_sz * sizeof(c8));
					return Name(buf, filename_sz);
				}
				--i;
			}
			// No extension found, return the filename directly.
			return name;
		}
		//! Replaces the extension.
		//! @param[in] new_extension The new extension to replace.
		//! @par Valid Usage
		//! * `new_extension` must be null-terminated.
		void replace_extension(const c8* new_extension)
		{
			replace_extension(new_extension, new_extension ? strlen(new_extension) : 0);
		}
		//! Replaces the extension.
		//! @param[in] new_extension The new extension to replace.
		//! @param[in] count The length of the new extension string.
		void replace_extension(const c8* new_extension, usize count)
		{
			auto& name = m_nodes.back();
			const c8* str = name.c_str();
			usize sz = strlen(str);
			usize i = sz - 1;	// points to the last valid char.
			// Finds the length of the extension.
			while (i)
			{
				if (str[i] == '.')
				{
					break;
				}
				--i;
			}
			if(i == 0 && str[i] != '.')
			{
				// This file does not have extension name, append it.
				append_extension(new_extension, count);
				return;
			}
			usize filename_len = i;
			usize new_filename_len;
			if (new_extension && count)
			{
				new_filename_len = count + filename_len + 1;	// 1 for dot(.)
			}
			else
			{
				new_filename_len = filename_len;
			}
			c8* buf = (c8*)alloca(sizeof(c8) * (new_filename_len + 1));
			buf[new_filename_len] = 0;
			// copy filename.
			memcpy(buf, str, filename_len * sizeof(c8));
			// copy extension.
			if (new_extension && count)
			{
				buf[filename_len] = '.';
				memcpy(buf + filename_len + 1, new_extension, count * sizeof(c8));
				for (usize i = 0; i < count; ++i)
				{
					buf[filename_len + 1 + i] = (c8)tolower(buf[filename_len + 1 + i]);
				}
			}
			name = Name(buf);;
		}
		//! Appends the extension.
		//! @details The system adds one extension separator (".") between extension and filename automatically.
		//! @param[in] new_extension The extension to append.
		//! @par Valid Usage
		//! * `new_extension` must be null-terminated.
		void append_extension(const c8* new_extension)
		{
			append_extension(new_extension, strlen(new_extension));
		}
		//! Appends the extension.
		//! @details The system adds one extension separator (".") between extension and filename automatically.
		//! @param[in] new_extension The new extension to replace.
		//! @param[in] count The length of the new extension string.
		void append_extension(const c8* new_extension, usize count)
		{
			auto& name = m_nodes.back();
			const c8* str = name.c_str();
			usize sz = strlen(str);
			c8* buf = (c8*)alloca(sizeof(c8) * (sz + count + 2));
			// copy original namec8
			memcpy(buf, str, sz * sizeof(c8));
			buf[sz] = '.';
			// copy extension.
			memcpy(buf + sz + 1, new_extension, count * sizeof(c8));
			// ends with NULL.
			buf[sz + count + 1] = 0;
			name = Name(buf);
		}
		//! Removes the extension.
		//! @details The extension separator (".") is removed as well in this operation.
		//! If the path does not have one extension (`extension().empty() == true`), this operation does nothing.
		void remove_extension()
		{
			replace_extension(nullptr);
		}
		//! Gets the name node at the specified index.
		//! @param[in] index The index of the name node.
		//! @return Returns one constant reference of the name node.
		const_reference at(usize index) const
		{
			return m_nodes[index];
		}
		//! Gets the name node at the specified index.
		//! @param[in] index The index of the name node.
		//! @return Returns one reference of the name node.
		reference at(usize index)
		{
			return m_nodes[index];
		}
		//! Gets the name node at the specified index.
		//! @param[in] index The index of the name node.
		//! @return Returns one constant reference of the name node.
		const_reference operator[](usize index) const
		{
			return at(index);
		}
		//! Gets the name node at the specified index.
		//! @param[in] index The index of the name node.
		//! @return Returns one reference of the name node.
		reference operator[](usize index)
		{
			return at(index);
		}
		//! Gets one iterator to the first name node of the path.
		//! @return Returns one iterator to the first name node of the path.
		iterator begin()
		{
			return m_nodes.begin();
		}
		//! Gets one constant iterator to the first name node of the path.
		//! @return Returns one constant iterator to the first name node of the path.
		const_iterator begin() const
		{
			return m_nodes.begin();
		}
		//! Gets one constant iterator to the first name node of the path.
		//! @return Returns one constant iterator to the first name node of the path.
		const_iterator cbegin() const
		{
			return m_nodes.cbegin();
		}
		//! Gets one iterator to the one past last name node of the path.
		//! @return Returns one iterator to the one past last name node of the path.
		iterator end()
		{
			return m_nodes.end();
		}
		//! Gets one constant iterator to the one past last name node of the path.
		//! @return Returns one constant iterator to the one past last name node of the path.
		const_iterator end() const
		{
			return m_nodes.end();
		}
		//! Gets one constant iterator to the one past last name node of the path.
		//! @return Returns one constant iterator to the one past last name node of the path.
		const_iterator cend() const
		{
			return m_nodes.cend();
		}
		//! Gets one reverse iterator to the last name node of the path.
		//! @return Returns one reverse iterator to the last name node of the path.
		reverse_iterator rbegin()
		{
			return m_nodes.rbegin();
		}
		//! Gets one constant reverse iterator to the last name node of the path.
		//! @return Returns one constant reverse iterator to the last name node of the path.
		const_reverse_iterator rbegin() const
		{
			return m_nodes.rbegin();
		}
		//! Gets one constant reverse iterator to the last name node of the path.
		//! @return Returns one constant reverse iterator to the last name node of the path.
		const_reverse_iterator crbegin() const
		{
			return m_nodes.crbegin();
		}
		//! Gets one reverse iterator to the one-before-first name node of the path.
		//! @return Returns one reverse iterator to the one-before-first name node of the path.
		reverse_iterator rend()
		{
			return m_nodes.rend();
		}
		//! Gets one constant reverse iterator to the one-before-first name node of the path.
		//! @return Returns one constant reverse iterator to the one-before-first name node of the path.
		const_reverse_iterator rend() const
		{
			return m_nodes.rend();
		}
		//! Gets one constant reverse iterator to the one-before-first name node of the path.
		//! @return Returns one constant reverse iterator to the one-before-first name node of the path.
		const_reverse_iterator crend() const
		{
			return m_nodes.crend();
		}
		//! Gets the size of the path, that is, the number of name nodes in the path.
		//! @return Returns the size of the path.
		usize size() const
		{
			return m_nodes.size();
		}
		//! Checks whether this path is empty, that is, the size of this path is `0`.
		//! @return Returns `true` if this path is empty, returns `false` otherwise.
		bool empty() const
		{
			return m_nodes.empty();
		}
		//! Gets the first name node in the path.
		//! @return Returns one reference to the first name node in the path.
		//! @par Valid Usage
		//! * `empty()` must be `false` when calling this function.
		reference front()
		{
			return m_nodes.front();
		}
		//! Gets the first name node in the path.
		//! @return Returns one constant reference to the first name node in the path.
		//! @par Valid Usage
		//! * `empty()` must be `false` when calling this function.
		const_reference front() const
		{
			return m_nodes.front();
		}
		//! Gets the last name node in the path.
		//! @return Returns one reference to the last name node in the path.
		//! @par Valid Usage
		//! * `empty()` must be `false` when calling this function.
		reference back()
		{
			return m_nodes.back();
		}
		//! Gets the last name node in the path.
		//! @return Returns one constant reference to the last name node in the path.
		//! @par Valid Usage
		//! * `empty()` must be `false` when calling this function.
		const_reference back() const
		{
			return m_nodes.back();
		}
		//! Inserts one name node at the back of the path.
		//! @param[in] path_node The name node to insert. The name node will be copy-inserted to the path.
		void push_back(const Name& path_node)
		{
			m_nodes.push_back(path_node);
		}
		//! Inserts one name node at the back of the path.
		//! @param[in] path_node The name node to insert. The name node will be move-inserted to the path.
		void push_back(Name&& path_node)
		{
			m_nodes.push_back(move(path_node));
		}
		//! Removes the last name node of the path.
		//! @par Valid Usage
		//! * `empty()` must be `false` when calling this function.
		void pop_back()
		{
			m_nodes.pop_back();
		}
		//! Appends another path to the end of this path. 
		//! @details The flags and the root name of the appended path are ignored.
		//! @param[in] appended_path The path to append.
		void append(const Path& appended_path)
		{
			for (auto& i : appended_path.m_nodes)
			{
				m_nodes.push_back(i);
			}
		}
		//! Appends another path to the end of this path. 
		//! @details The flags and the root name of the appended path are ignored.
		//! @param[in] appended_path The path to append.
		//! @param[in] pos The index of the first node to append. 
		//! Nodes in range [`appended_path.begin() + pos`, `appended_path.end()`) will be appended.
		void append(const Path& appended_path, usize pos)
		{
			for (auto iter = appended_path.m_nodes.begin() + pos; iter != appended_path.m_nodes.end(); ++iter)
			{
				m_nodes.push_back(*iter);
			}
		}
		//! Appends another path to the end of this path. 
		//! @details The flags and the root name of the appended path are ignored.
		//! @param[in] appended_path The path to append.
		//! @param[in] pos The index of the first node to append. 
		//! @param[in] count The number of nodes to append.
		//! Nodes in range [`appended_path.begin() + pos`, `appended_path.begin() + pos + count`) will be appended.
		void append(const Path& appended_path, usize pos, usize count)
		{
			for (usize i = 0; i < count; ++i)
			{
				m_nodes.push_back(appended_path.m_nodes[pos + i]);
			}
		}
		//! Clears all nodes in the path.
		void clear()
		{
			m_nodes.clear();
		}
		//! Resets the path object.
		//! This operation clears all nodes in the path, then clears the root name and flags of the path.
		void reset()
		{
			m_nodes.clear();
			m_root.reset();
			m_flags = PathFlag::none;
		}
		//! Removes one name node from the path.
		//! @param[in] pos The iterator to the name node to be removed.
		//! @return Returns one iterator to the next name node of the removed name node when iterating nodes.
		//! @par Valid Usage
		//! * `pos` must points to a valid name node in the path.
		iterator erase(const_iterator pos)
		{
			return m_nodes.erase(pos);
		}
		//! Removes one range of name nodes from the path.
		//! @param[in] first The iterator to the first name node to be removed.
		//! @param[in] last The iterator to the one-past-last name node to be removed.
		//! @return Returns one iterator to the next name node of the removed name nodes when iterating name nodes.
		//! @par Valid Usage
		//! * `first` must be either `end()` or one valid name node in the path.
		//! * If `first != end()`, [`first`, `last`) must specifies either one empty range (`first == last`) or one valid name node range of the path.
		//! * If `first == end()`, [`first`, `last`) must specifies one empty range (`first == last`).
		iterator erase(const_iterator first, const_iterator last)
		{
			return m_nodes.erase(first, last);
		}
		//! Computes the hash code of this path.
		//! @return Returns the hash code of this path.
		usize hash_code() const
		{
			usize h = test_flags(m_flags, PathFlag::absolute) ? 0x3745 : 0; // Random initial seed to deferent "/A/B" from "A/B".
			u64 id = 0;
			if (m_root)
			{
				id = m_root.id();
				h = memhash<usize>(&id, sizeof(u64), h);
				h = strhash<usize>("://", h);// To deferent "A://B" from "/A/B"
			}
			for (auto& i : m_nodes)
			{
				id = i.id();
				h = memhash<usize>(&id, sizeof(u64), h);
			}
			return h;
		}
		//! Checks whether the current path is one subsequent path of the specified base path. 
		//! @details For example, "/foo/bar" is a subsequent path of "/foo".
		//! @param[in] base The base path to be checked.
		//! @remark Path A is the subsequent path of B if:
		//! 1. Path A contains all nodes of B as its prefix path, in the same order as B.
		//! 2. If both paths have root names, their root names should be identical.
		//! The path flags (absolute/relative, file/directory) are ignored while checking.
		bool is_subpath_of(const Path& base) const
		{
			auto base_root = base.root();
			if (m_root && base_root && (m_root != base_root))
			{
				return false;
			}
			if (m_nodes.size() < base.size())
			{
				return false;
			}
			for (usize i = 0; i < base.size(); ++i)
			{
				if (m_nodes[i] != base[i])
				{
					return false;
				}
			}
			return true;
		}
		//! Compares two paths for equality.
		//! @param[in] rhs The path to compare with.
		//! @param[in] compared_components The components to compare, default is to compare all components
		//! and only `true` if all components of both paths are equal.
		//! @return Returns `true` if all compared components in both paths are equal. Returns `false` otherwise.
		bool equal_to(const Path& rhs, PathComponent compared_components = PathComponent::all) const
		{
			if (test_flags(compared_components, PathComponent::flags))
			{
				if (m_flags != rhs.m_flags)
				{
					return false;
				}
			}
			if (test_flags(compared_components, PathComponent::root))
			{
				if (m_root != rhs.m_root)
				{
					return false;
				}
			}
			if (test_flags(compared_components, PathComponent::nodes))
			{
				if (m_nodes.size() != rhs.m_nodes.size())
				{
					return false;
				}
				for (usize i = 0; i < m_nodes.size(); ++i)
				{
					if (m_nodes[i] != rhs.m_nodes[i])
					{
						return false;
					}
				}
			}
			return true;
		}
		//! Compares all components of two paths for equality.
		//! @param[in] rhs The path to compare with.
		//! @return Returns `true` if all components in both paths are equal. Returns `false` otherwise.
		bool operator==(const Path& rhs) const
		{
			return equal_to(rhs);
		}
		//! Compares all components of two paths for non-equality.
		//! @param[in] rhs The path to compare with.
		//! @return Returns `false` if all components in both paths are equal. Returns `true` otherwise.
		bool operator!=(const Path& rhs) const
		{
			return !equal_to(rhs);
		}
	};

	template <> struct hash<Path>
	{
		usize operator()(const Path& val) const { return val.hash_code(); }
	};

	//! Gets the type object of @ref Path.
	//! @return Returns the type object of @ref Path.
	LUNA_RUNTIME_API typeinfo_t path_type();
	
	template <> struct typeof_t<Path> { typeinfo_t operator()() const { return path_type(); } };

	//! @}
}
