/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Ref.hpp
* @author JXMaster
* @date 2022/7/14
*/
#pragma once
#include "Object.hpp"
#include "Interface.hpp"

namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	
	//! The smart pointer that represents one typeless strong reference to one boxed object.
	class ObjRef
	{
	public:
		//! Resets the reference to null.
		//! @details This function decreases the strong reference counter of the boxed object before resetting the reference. 
		//! If this reference is null when this function is called, this function does nothing.
		void reset() { internal_clear(); }
		//! Checks whether this reference is valid.
		//! @details One strong reference is valid when it is not null.
		//! @return Returns `true` when the reference is valid. Returns `false` otherwise.
		bool valid() const { return m_obj != nullptr; }
		//! Gets the boxed object.
		//! @details This call does not modify the reference counter of the object.
		//! @return Returns one pointer to the boxed object. Returns `nullptr` if the reference is null.
		object_t get() const { return m_obj; }
		//! Attaches provided pointer.
		//! @details This call does not modify the reference counter of the new boxed object.
		//! The strong reference counter of the original boxed object, if not null, will be decreased before new pointer is attached.
		//! @param[in] ptr The pointer to attach.
		void attach(object_t ptr) { internal_clear(); m_obj = ptr; }
		//! Detaches the stored pointer. The reference becomes null after this operation.
		//! @details This operation does not modify the reference counter of the original boxed object.
		//! @return Returns the pointer to the original boxed object.
		//! Returns `nullptr` if the reference is null when this function is called.
		object_t detach() { return atom_exchange_pointer(&m_obj, nullptr); }
		//! Constructs one null reference.
		ObjRef() : m_obj{ nullptr } {}
		~ObjRef() { internal_clear(); }
		//! Constructs one reference by coping the pointer from another reference.
		//! @details The strong reference counter of the new boxed object, if not null, will be increased.
		//! @param[in] rhs The reference to copy from.
		ObjRef(const ObjRef& rhs) : m_obj{ rhs.m_obj } { internal_addref(); }
		//! Constructs one reference by moving the pointer from another reference.
		//! @details The reference counter of the new boxed object is not modified.
		//! @param[in] rhs The reference to move from. This reference will be null after this operation.
		ObjRef(ObjRef&& rhs) : m_obj{ atom_exchange_pointer(&rhs.m_obj, nullptr) } {}
		//! Assigns this reference by coping the pointer from another reference.
		//! @details The strong reference counter of the new boxed object, if not null, will be increased.
		//! The strong reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] rhs The reference to copy from.
		//! @return Returns `*this`.
		ObjRef& operator=(const ObjRef& rhs) { internal_clear(); m_obj = rhs.m_obj; internal_addref(); return *this; }
		//! Assigns this reference by moving the pointer from another reference.
		//! @details The reference counter of the new boxed object is not modified.
		//! The strong reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] rhs The reference to move from. This reference will be null after this operation.
		//! @return Returns `*this`.
		ObjRef& operator=(ObjRef&& rhs) { internal_clear(); m_obj = atom_exchange_pointer(&rhs.m_obj, nullptr); return *this; }
		//! Constructs one reference by providing the underlying pointer directly.
		//! @details The strong reference counter of the new boxed object will be increased if the provided pointer is valid.
		//! @param[in] ptr The pointer to set.
		explicit ObjRef(object_t ptr) : m_obj(ptr) { internal_addref(); }
		//! Replaces the underlying pointer of this reference with the given pointer.
		//! @details The strong reference counter of the new boxed object will be increased if the provided pointer is valid.
		//! The weak reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] rhs The pointer to set.
		//! @return Returns `*this`.
		ObjRef& operator=(object_t rhs) { internal_clear(); m_obj = rhs; internal_addref(); return *this; }
		//! Compares two references for equality. 
		//! @details Two references are equal if their underlying pointers are equal.
		//! @param[in] rhs The reference to compare with.
		//! @return Returns `true` if two references are equal. Returns `false` otherwise.
		bool operator== (const ObjRef& rhs) const { return m_obj == rhs.m_obj; }
		//! Compares two references for non-equality.
		//! @details Two references are not equal if their underlying pointers are not equal.
		//! @param[in] rhs The reference to compare with.
		//! @return Returns `true` if two references are not equal. Returns `false` otherwise.
		bool operator!= (const ObjRef& rhs) const { return m_obj != rhs.m_obj; }
		//! Compares two references. 
		//! @details The referneces are compared by comparing their underlying pointers after converted to unsigned integers.
		//! If the reference is not valid, the converted integer will be `0`.
		//! @param[in] rhs The reference to compare with.
		//! @return Returns `true` if this reference is less than the incoming reference. Returns `false` otherwise.
		bool operator< (const ObjRef& rhs) const { return (usize)m_obj < (usize)(rhs.m_obj); }
		//! Gets the type object of the boxed object.
		//! @return Returns the type object of the boxed object. Returns `nullptr` if the reference is not valid.
		typeinfo_t type() const { return m_obj ? get_object_type(m_obj) : nullptr; }
		//! Checks whether this reference is valid.
		//! @details One reference is valid when the underlying pointer is not `nullptr`.
		//! @return Returns `true` when the reference is valid. Returns `false` otherwise.
		operator bool() const { return valid(); }
	private:
		object_t m_obj;
		void internal_addref() const { if (m_obj) object_retain(m_obj); }
		void internal_clear()
		{
			object_t ptr = atom_exchange_pointer(&m_obj, nullptr);
			if (ptr)
			{
				object_release(ptr);
			}
		}
	};

	//! The smart pointer that represents one typed strong reference to one boxed object.
	template <typename _Ty>
	class Ref
	{
		static constexpr bool has_get_object = is_base_of_v<Interface, _Ty>;

		_Ty* m_vtable;

		static void* internal_query_interface(object_t obj, const Guid& guid)
		{
			if (object_is_type(obj, get_type_by_guid(guid)))
			{
				return obj;
			}
			return query_interface(obj, guid);
		}

		template <bool _HasGetObject>
		struct InterfaceAdapter;
		template <> struct InterfaceAdapter<true>
		{
			static void internal_addref(_Ty* obj) { if (obj) object_retain(obj->get_object()); }
			static void internal_clear(_Ty** obj)
			{
				_Ty* ptr = atom_exchange_pointer(obj, nullptr);
				if (ptr)
				{
					object_release(ptr->get_object());
				}
			}
			static object_t get_object(_Ty* obj) { return obj ? obj->get_object() : nullptr; }
			static object_t detach(_Ty** obj) { _Ty* vt = atom_exchange_pointer(obj, nullptr); return vt ? vt->get_object() : nullptr; }
		};
		template <> struct InterfaceAdapter<false>
		{
			static void internal_addref(_Ty* obj) { if (obj) object_retain((object_t)obj); }
			static void internal_clear(_Ty** obj)
			{
				_Ty* ptr = atom_exchange_pointer(obj, nullptr);
				if (ptr)
				{
					object_release((object_t)ptr);
				}
			}
			static object_t get_object(_Ty* obj) { return (object_t)obj; }
			static object_t detach(_Ty** obj) { return (object_t)atom_exchange_pointer(obj, nullptr); }
		};

		void internal_addref() const { InterfaceAdapter<has_get_object>::internal_addref(m_vtable); }
		void internal_clear() { InterfaceAdapter<has_get_object>::internal_clear(&m_vtable); }
	public:
		//! Resets the reference to null.
		//! @details This function decreases the strong reference counter of the boxed object before resetting the reference. 
		//! If this reference is null when this function is called, this function does nothing.
        void reset() { internal_clear(); }
		//! Checks whether this reference is valid.
		//! @details One strong reference is valid when it is not null.
		//! @return Returns `true` when the reference is valid. Returns `false` otherwise.
		bool valid() const { return m_vtable != nullptr; }
		//! Gets the boxed object.
        //! @details This call does not modify the reference counter of the object.
		//! @return Returns one pointer to the boxed object. Returns `nullptr` if the reference is null.
		object_t object() const { return InterfaceAdapter<has_get_object>::get_object(m_vtable); }
		//! Gets the boxed object casted to `_Ty`.
		//! @details This call does not modify the reference counter of the object.
		//! @return Returns the interface or object pointer of the boxed object. Returns `nullptr` if the reference is not valid.
		//! @remark Note that the pointer returned by @ref get may not equal to the pointer returned by @ref object due to interface vtable offsetting.
		//! When you perform typeless object operations like increasing/decreasing reference counters, casting types using RTTI, etc, always call @ref object 
        //! on this reference or @ref Interface::get_object on the interface pointer to get object pointer.
		_Ty* get() const { luassert(m_vtable); return m_vtable; }
		//! Gets the boxed object casted to `_Ty`.
		//! @details This call does not modify the reference counter of the object.
		//! @return Returns the interface or object pointer of the boxed object. Returns `nullptr` if the reference is not valid.
		//! @remark See remarks of @ref get for details.
		_Ty* operator->() const { return get(); }
		//! Attaches provided pointer.
		//! @details This call does not modify the reference counter of the new boxed object.
		//! The original boxed object, if not null, will be released before new pointer is attached.
		//! @param[in] ptr The pointer to attach.
		void attach(object_t ptr)
		{
			internal_clear();
			if (ptr)
			{
				m_vtable = (_Ty*)internal_query_interface(ptr, _Ty::__guid);
				if (!m_vtable) object_release(ptr);
			}
		}
		//! Detaches the stored pointer. The reference becomes null after this operation.
		//! @details This operation does not modify the reference counter of the original boxed object.
		//! @return Returns the pointer to the original boxed object.
		//! Returns `nullptr` if the reference is null when this function is called.
		object_t detach() { return InterfaceAdapter<has_get_object>::detach(&m_vtable); }
		//! Constructs one null reference.
		Ref() : m_vtable(nullptr) {}
		~Ref() { internal_clear(); }
		//! Constructs one reference by coping the pointer from another reference of the same type.
		//! @details The strong reference counter of the new boxed object, if not null, will be increased.
		//! @param[in] rhs The reference to copy from.
		Ref(const Ref& rhs) : m_vtable(rhs.m_vtable) { internal_addref(); }
		//! Constructs one reference by moving the pointer from another reference of the same type.
		//! @details The reference counter of the new boxed object is not modified.
		//! @param[in] rhs The reference to move from. This reference will be null after this operation.
		Ref(Ref&& rhs) : m_vtable{ atom_exchange_pointer(&rhs.m_vtable, nullptr) } {}
        //! Assigns this reference by coping the pointer from another reference of the same type.
		//! @details The strong reference counter of the new boxed object, if not null, will be increased.
		//! The strong reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] rhs The reference to copy from.
		//! @return Returns `*this`.
		Ref& operator=(const Ref& rhs) { internal_clear(); m_vtable = rhs.m_vtable; internal_addref(); return *this; }
		//! Assigns this reference by moving the pointer from another reference of the same type.
		//! @details The reference counter of the new boxed object is not modified.
		//! The strong reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] rhs The reference to move from. This reference will be null after this operation.
		//! @return Returns `*this`.
		Ref& operator=(Ref&& rhs) { internal_clear(); m_vtable = atom_exchange_pointer(&rhs.m_vtable, nullptr); return *this; }
		//! Constructs one reference by coping the pointer from another reference of one different type.
		//! @details The assignment will fail if the new reference is null or cannot be casted to `_Ty`.
        //! If the assignment fails, this reference will be null after this operation.
        //! If the assignment succeeds, The strong reference counter of the new boxed object will be increased.
		//! @param[in] rhs The reference to copy from.
		template <typename _Rty>
		Ref(const Ref<_Rty>& rhs)
		{
			object_t obj = rhs.object();
			m_vtable = obj ? (_Ty*)internal_query_interface(obj, _Ty::__guid) : nullptr;
			internal_addref();
		}
		//! Assigns this reference by coping the pointer from another reference of one different type.
        //! @details The assignment will fail if the new reference is null or cannot be casted to `_Ty`.
		//! If the assignment fails, this reference will be null after this operation.
        //! If the assignment succeeds, The strong reference counter of the new boxed object will be increased.
		//! The strong reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] rhs The reference to copy from.
		//! @return Returns `*this`.
		template <typename _Rty>
		Ref& operator=(const Ref<_Rty>& rhs)
		{
			internal_clear();
			object_t obj = rhs.object();
			m_vtable = obj ? (_Ty*)internal_query_interface(obj, _Ty::__guid) : nullptr;
			internal_addref();
			return *this;
		}
		//! Constructs one reference by moving the pointer from another reference of one different type.
        //! @details The assignment will fail if the new reference is null or cannot be casted to `_Ty`.
		//! If the assignment fails, this reference will be null after this operation, and the strong reference counter 
        //! of the new boxed object, if not null, will be decreased.
        //! If the assignment succeeds, the reference counter of the new boxed object will not be modified.
		//! @param[in] rhs The reference to move from. This reference will be null after this operation.
		template <typename _Rty>
		Ref(Ref<_Rty>&& rhs)
		{
			object_t obj = rhs.detach();
			m_vtable = obj ? (_Ty*)internal_query_interface(obj, _Ty::__guid) : nullptr;
			if (obj && !m_vtable) object_release(obj);
		}
		//! Assigns this reference by moving the pointer from another reference of one different type.
		//! @details The assignment will fail if the new reference is null or cannot be casted to `_Ty`.
		//! If the assignment fails, this reference will be null after this operation, and the strong reference counter 
        //! of the new boxed object, if not null, will be decreased.
        //! If the assignment succeeds, the reference counter of the new boxed object will not be modified.
        //! The strong reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] rhs The reference to move from. This reference will be null after this operation.
		//! @return Returns `*this`.
		template <typename _Rty>
		Ref& operator=(Ref<_Rty>&& rhs)
		{
			internal_clear();
			object_t obj = rhs.detach();
			m_vtable = obj ? (_Ty*)internal_query_interface(obj, _Ty::__guid) : nullptr;
			if (obj && !m_vtable) object_release(obj);
			return *this;
		}
		//! Constructs one reference using the native pointer of the same type.
		//! @details The strong reference counter of the new boxed object, if not null, will be increased.
		//! @param[in] ptr The native pointer to set.
		Ref(_Ty* ptr)
		{
			m_vtable = ptr;
			internal_addref();
		}
		//! Assigns this reference using the native pointer of the same type.
		//! @details The strong reference counter of the new boxed object, if not null, will be increased.
		//! The strong reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] ptr The native pointer to set.
		Ref& operator=(_Ty* ptr)
		{
			internal_clear();
			m_vtable = ptr;
			internal_addref();
			return *this;
		}
		//! Constructs one reference using the native pointer of one different type.
		//! @details The assignment will fail if the new pointer is `nullptr` or cannot be casted to `_Ty`.
        //! If the assignment fails, this reference will be null after this operation.
        //! If the assignment succeeds, The strong reference counter of the new boxed object will be increased.
		//! @param[in] ptr The native pointer to set.
		template <typename _Rty, typename _Enable = enable_if_t<is_base_of_v<Interface, _Rty>>>
		Ref(_Rty* ptr)
		{
			_Ty* v = query_interface<_Ty>(ptr->get_object());
			m_vtable = v;
			internal_addref();
		}
		//! Constructs one reference by coping the pointer from one typeless reference.
		//! @details The assignment will fail if the new reference is null or cannot be casted to `_Ty`.
        //! If the assignment fails, this reference will be null after this operation.
        //! If the assignment succeeds, The strong reference counter of the new boxed object will be increased.
		//! @param[in] rhs The reference to copy from.
		Ref(const ObjRef& rhs)
		{
			if (rhs)
			{
				m_vtable = (_Ty*)internal_query_interface(rhs.get(), _Ty::__guid);
				internal_addref();
			}
			else
			{
				m_vtable = nullptr;
			}
		}
		//! Constructs one reference by moving the pointer from one typeless reference.
		//! @details The assignment will fail if the new reference is null or cannot be casted to `_Ty`.
		//! If the assignment fails, this reference will be null after this operation, and the strong reference counter 
        //! of the new boxed object, if not null, will be decreased.
        //! If the assignment succeeds, the reference counter of the new boxed object will not be modified.
		//! @param[in] rhs The reference to move from. This reference will be null after this operation.
		Ref(ObjRef&& rhs)
		{
			object_t ptr = rhs.detach();
			if (ptr)
			{
				m_vtable = (_Ty*)internal_query_interface(ptr, _Ty::__guid);
				if (!m_vtable) object_release(ptr);
			}
			else
			{
				m_vtable = nullptr;
			}
		}
        //! Assigns this reference by coping the pointer from one typeless reference.
        //! @details The assignment will fail if the new reference is null or cannot be casted to `_Ty`.
		//! If the assignment fails, this reference will be null after this operation.
        //! If the assignment succeeds, The strong reference counter of the new boxed object will be increased.
		//! The strong reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] rhs The reference to copy from.
		//! @return Returns `*this`.
		Ref& operator=(const ObjRef& rhs)
		{
			internal_clear();
			if (rhs)
			{
				m_vtable = (_Ty*)internal_query_interface(rhs.get(), _Ty::__guid);
				internal_addref();
			}
			return *this;
		}
        //! Assigns this reference by moving the pointer from one typeless reference.
		//! @details The assignment will fail if the new reference is null or cannot be casted to `_Ty`.
		//! If the assignment fails, this reference will be null after this operation, and the strong reference counter 
        //! of the new boxed object, if not null, will be decreased.
        //! If the assignment succeeds, the reference counter of the new boxed object will not be modified.
        //! The strong reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] rhs The reference to move from. This reference will be null after this operation.
		//! @return Returns `*this`.
		Ref& operator=(ObjRef&& rhs)
		{
			internal_clear();
			object_t ptr = rhs.detach();
			if (ptr)
			{
				m_vtable = (_Ty*)internal_query_interface(ptr, _Ty::__guid);
				if (!m_vtable) object_release(ptr);
			}
			return *this;
		}
        //! Compares two references for equality. 
		//! @details Two references are equal if their underlying pointers are equal.
		//! @param[in] rhs The reference to compare with.
		//! @return Returns `true` if two references are equal. Returns `false` otherwise.
		bool operator== (const Ref& rhs) const { return object() == rhs.object(); }
        //! Compares two references for non-equality.
		//! @details Two references are not equal if their underlying pointers are not equal.
		//! @param[in] rhs The reference to compare with.
		//! @return Returns `true` if two references are not equal. Returns `false` otherwise.
		bool operator!= (const Ref& rhs) const { return object() != rhs.object(); }
        //! Compares one reference with one native pointer for equality. 
		//! @param[in] rhs The native pointer to compare with.
		//! @return Returns `true` if `get() == rhs` . Returns `false` otherwise.
		bool operator== (_Ty* rhs) const { return m_vtable == rhs; }
        //! Compares one reference with one native pointer for non-equality. 
		//! @param[in] rhs The native pointer to compare with.
		//! @return Returns `true` if `get() != rhs` . Returns `false` otherwise.
		bool operator!= (_Ty* rhs) const { return m_vtable != rhs; }
        //! Compares two references. 
		//! @details The referneces are compared by comparing their underlying pointers after comverted to unsigned integers.
		//! If the reference is not valid, the converted integer will be `0`.
		//! @param[in] rhs The reference to compare with.
		//! @return Returns `true` if this reference is less than the incoming reference. Returns `false` otherwise.
		bool operator< (const Ref& rhs) const { return (usize)object() < (usize)rhs.object(); }
        //! Gets the boxed object casted to `_Ty`.
		//! @details This call does not modify the reference counter of the object.
		//! @return Returns the interface or object pointer of the boxed object. Returns `nullptr` if the reference is not valid.
		//! @remark Note that the pointer returned by @ref get may not equal to the pointer returned by @ref object due to interface vtable offsetting.
		//! When you perform typeless object operations like increasing/decreasing reference counters, casting types using RTTI, etc, always call @ref object 
        //! on this reference or @ref Interface::get_object on the interface pointer to get object pointer.
		operator _Ty* () const { return m_vtable; }
		//! Gets the boxed object casted to `_Rty`.
		//! @return Returns one pointer to the boxed object casted to `_Rty`.
		//! Returns `nullptr` if the reference is null or the boxed object cannot be casted to `_Rty`.
		template <typename _Rty>
		_Rty* as() const
		{
			auto obj = object();
			if (!obj) return nullptr;
			return (_Rty*)internal_query_interface(obj, _Rty::__guid);
		}
	};

	//! Creates a strong reference from one raw pointer without modifing its reference count.
	//! @param[in] obj The raw pointer.
	//! @return Returns the strong reference created from the raw pointer.
	template <typename _Ty>
	Ref<_Ty> box_ptr(_Ty* obj)
	{
		Ref<_Ty> r;
		r.attach(obj);
		return r;
	}
	//! Creates one new boxed object.
	//! @details This function uses @ref object_alloc to allocate one new boxed object, then use
	//! placement new operator to initialize the object.
	//! @param[in] args The arguments to construct the new boxed object.
	//! @return Returns one strong reference to the new boxed object.
	template <typename _Ty, typename... _Args>
	inline Ref<_Ty> new_object(_Args&&... args)
	{
		_Ty* o = reinterpret_cast<_Ty*>(object_alloc(typeof<_Ty>()));
		new (o) _Ty(forward<_Args>(args)...);
		return box_ptr(o);
	}

	    //! The smart pointer that represents one typeless weak reference to one boxed object.
	class WeakObjRef
	{
		mutable object_t m_obj;
		void internal_addref() { if (m_obj) object_retain_weak(m_obj); }
		void internal_clear() const
		{
			object_t ptr = atom_exchange_pointer(&m_obj, nullptr);
			if (ptr)
			{
				object_release_weak(ptr);
			}
		}
		object_t internal_get() const
		{
			if (m_obj && object_expired(m_obj))
			{
				internal_clear();
			}
			return m_obj;
		}
	public:
		//! Resets the reference to null.
		//! @details This function decreases the weak reference counter of the boxed object before resetting the reference. 
		//! If this reference is null when this function is called, this function does nothing.
        void reset() { internal_clear(); }
        //! Checks whether this reference is valid.
		//! @details One weak reference is valid when it is not null, and the boxed object is not expired.
		//! @return Returns `true` when the reference is valid. Returns `false` otherwise.
		bool valid() const { return internal_get() != nullptr; }
        //! Gets the boxed object.
		//! @details This call does not modify the reference counter of the object.
		//! @return Returns one pointer to the boxed object. Returns `nullptr` if the reference is null or the boxed object is expired.
        //! @remark It is not safe to use the returned boxed object directly, since one weak reference does not prevent one object from
        //! destructing itself if all strong references are released. This function only guarantees that the returned boxed object is valid
        //! when this function returns, but not after. To use the boxed object, the user should use @ref pin to create one strong
        //! reference from this reference, then use that reference instead.
		object_t get() const { return internal_get(); }
        //! Attaches provided pointer.
		//! @details This call does not modify the reference counter of the new boxed object.
		//! The weak reference counter of the original boxed object, if not null, will be decreased before new pointer is attached.
		//! @param[in] ptr The pointer to attach.
		void attach(object_t ptr) { internal_clear(); m_obj = ptr; }
        //! Detaches the stored pointer. The reference becomes null after this operation.
		//! @details This operation does not modify the reference counter of the original boxed object.
		//! @return Returns the pointer to the original boxed object.
		//! Returns `nullptr` if the reference is null when this function is called.
		object_t detach() { object_t r = internal_get(); m_obj = nullptr; return r; }
        //! Constructs one null reference.
		WeakObjRef() : m_obj{ nullptr } {}
		~WeakObjRef() { internal_clear(); }
        //! Constructs one reference by coping the pointer from another reference.
		//! @details The weak reference counter of the new boxed object, if not null, will be increased.
		//! @param[in] rhs The reference to copy from.
		WeakObjRef(const WeakObjRef& rhs) : m_obj{ rhs.get() } { internal_addref(); }
        //! Constructs one reference by moving the pointer from another reference.
		//! @details The weak reference counter of the new boxed object is not modified.
		//! @param[in] rhs The reference to move from. This reference will be null after this operation.
		WeakObjRef(WeakObjRef&& rhs) : m_obj{ rhs.detach() } {}
        //! Assigns this reference by coping the pointer from another reference.
		//! @details The weak reference counter of the new boxed object, if not null, will be increased.
		//! The weak reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] rhs The reference to copy from.
		//! @return Returns `*this`.
		WeakObjRef& operator=(const WeakObjRef& rhs) { internal_clear(); m_obj = rhs.get(); internal_addref(); return *this; }
        //! Assigns this reference by moving the pointer from another reference.
		//! @details The reference counter of the new boxed object is not modified.
		//! The weak reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] rhs The reference to move from. This reference will be null after this operation.
		//! @return Returns `*this`.
		WeakObjRef& operator=(WeakObjRef&& rhs) { internal_clear(); m_obj = rhs.detach(); return *this; }
        //! Constructs one weak reference from one strong reference.
		//! @details The weak reference counter of the new boxed object, if not null, will be increased.
		//! @param[in] rhs The reference to set.
		explicit WeakObjRef(const ObjRef& rhs) : m_obj(rhs.get()) { internal_addref(); }
        //! Assigns this reference by coping the pointer from one strong reference.
        //! @details The weak reference counter of the new boxed object, if not null, will be increased.
        //! The weak reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] rhs The reference to set.
		WeakObjRef& operator=(const ObjRef& rhs) { internal_clear(); m_obj = rhs.get(); internal_addref(); return *this; }
        //! Constructs one reference by providing the underlying pointer directly.
		//! @details The weak reference counter of the new boxed object will be increased if the provided pointer is valid.
		//! @param[in] ptr The pointer to set.
		explicit WeakObjRef(object_t rhs) : m_obj(rhs) { internal_addref(); }
        //! Replaces the underlying pointer of this reference with the given pointer.
		//! @details The weak reference counter of the new boxed object will be increased if the provided pointer is valid.
		//! The weak reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] rhs The pointer to set.
		//! @return Returns `*this`.
		WeakObjRef& operator=(object_t rhs) { internal_clear(); m_obj = rhs; internal_addref(); return *this; }
        //! Compares two references for equality. 
		//! @details Two references are equal if their underlying pointers are either equal or both invalid.
		//! @param[in] rhs The reference to compare with.
		//! @return Returns `true` if two references are equal. Returns `false` otherwise.
		bool operator== (const WeakObjRef& rhs) const { return get() == rhs.get(); }
        //! Compares two references for non-equality. 
		//! @details Two references are equal if their underlying pointers are either equal or both invalid.
		//! @param[in] rhs The reference to compare with.
		//! @return Returns `true` if two references are not equal. Returns `false` otherwise.
		bool operator!= (const WeakObjRef& rhs) const { return get() != rhs.get(); }
        //! Compares two references. 
		//! @details The referneces are compared by comparing their underlying pointers after converted to unsigned integers.
        //! If the reference is not valid, the converted integer will be `0`.
		//! @param[in] rhs The reference to compare with.
		//! @return Returns `true` if this reference is less than the incoming reference. Returns `false` otherwise.
		bool operator< (const WeakObjRef& rhs) const { return (usize)get() < (usize)(rhs.get()); }
        //! Checks whether this reference is valid.
		//! @details One weak reference is valid when it is not null, and the boxed object is not expired.
		//! @return Returns `true` when the reference is valid. Returns `false` otherwise.
        operator bool() const { return valid(); }
        //! Creates one strong reference from this weak reference.
        //! @return Returns the created strong reference if this weak reference is valid.
        //! Returns one null reference if this weak reference is not valid.
		ObjRef pin() const
		{
			if (m_obj && !object_retain_if_not_expired(m_obj)) internal_clear();
            ObjRef r;
            r.attach(m_obj);
            return r;
		}
	};
    
    //! The smart pointer that represents one typed weak reference to one boxed object.
	template <typename _Ty>
	class WeakRef
	{
		static constexpr bool has_get_object = is_base_of_v<Interface, _Ty>;

		template <bool _HasGetObject>
		struct InterfaceAdapter;
		template <> struct InterfaceAdapter<true>
		{
			static void internal_addref(_Ty* obj) { if (obj) object_retain_weak(obj->get_object()); }
			static void internal_clear(_Ty** obj)
			{
				_Ty* ptr = atom_exchange_pointer(obj, nullptr);
				if (ptr)
				{
					object_release_weak(ptr->get_object());
				}
			}
			static object_t get_object(_Ty* obj) { return obj ? obj->get_object() : nullptr; }
			static object_t detach(_Ty** obj) { _Ty* vt = atom_exchange_pointer(obj, nullptr); return vt ? vt->get_object() : nullptr; }
		};
		template <> struct InterfaceAdapter<false>
		{
			static void internal_addref(_Ty* obj) { if (obj) object_retain_weak((object_t)obj); }
			static void internal_clear(_Ty** obj)
			{
				_Ty* ptr = atom_exchange_pointer(obj, nullptr);
				if (ptr)
				{
					object_release_weak((object_t)ptr);
				}
			}
			static object_t get_object(_Ty* obj) { return (object_t)obj; }
			static object_t detach(_Ty** obj) { return (object_t)atom_exchange_pointer(obj, nullptr); }
		};

		mutable _Ty* m_vtable;
		void internal_addref() { InterfaceAdapter<has_get_object>::internal_addref(m_vtable); }
		void internal_clear() const
		{
			InterfaceAdapter<has_get_object>::internal_clear(&m_vtable);
		}
		object_t internal_get() const
		{
			object_t obj = InterfaceAdapter<has_get_object>::get_object(m_vtable);
			if (obj && object_expired(obj))
			{
				internal_clear();
				obj = nullptr;
			}
			return obj;
		}
	public:
		//! Resets the reference to null.
		//! @details This function decreases the weak reference counter of the boxed object before resetting the reference. 
		//! If this reference is null when this function is called, this function does nothing.
        void reset() { internal_clear(); }
		//! Checks whether this reference is valid.
		//! @details One weak reference is valid when it is not null, and the boxed object is not expired.
		//! @return Returns `true` when the reference is valid. Returns `false` otherwise.
		bool valid() const { return internal_get() != nullptr; }
		//! Gets the boxed object.
		//! @details This call does not modify the reference counter of the object.
		//! @return Returns one pointer to the boxed object. Returns `nullptr` if the reference is null or the boxed object is expired.
        //! @remark It is not safe to use the returned boxed object directly, see remarks of `WeakObjRef::get` for details.
		object_t object() const { return internal_get(); }
        //! Constructs one null reference.
		WeakRef() : m_vtable(nullptr) {}
		~WeakRef() { internal_clear(); }
		//! Constructs one reference by coping the pointer from another reference of the same type.
		//! @details The weak reference counter of the new boxed object, if not null, will be increased.
		//! @param[in] rhs The reference to copy from.
		WeakRef(const WeakRef& rhs) : m_vtable(rhs.m_vtable) { internal_addref(); }
        //! Constructs one reference by moving the pointer from another reference.
		//! @details The weak reference counter of the new boxed object is not modified.
		//! @param[in] rhs The reference to move from. This reference will be null after this operation.
		WeakRef(WeakRef&& rhs) : m_vtable{ atom_exchange_pointer(&rhs.m_vtable, nullptr) } {}
		//! Assigns this reference by coping the pointer from another reference of the same type.
		//! @details The weak reference counter of the new boxed object, if not null, will be increased.
		//! The weak reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] rhs The reference to copy from.
		//! @return Returns `*this`.
        WeakRef& operator=(const WeakRef& rhs) { internal_clear(); m_vtable = rhs.m_vtable; internal_addref(); return *this; }
		//! Assigns this reference by moving the pointer from another reference of the same type.
		//! @details The reference counter of the new boxed object is not modified.
		//! The weak reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] rhs The reference to move from. This reference will be null after this operation.
		//! @return Returns `*this`.
        WeakRef& operator=(WeakRef&& rhs) { internal_clear(); m_vtable = atom_exchange_pointer(&rhs.m_vtable, nullptr); return *this; }
		//! Constructs one weak reference from one strong reference of the same type.
		//! @details The weak reference counter of the new boxed object, if not null, will be increased.
		//! @param[in] rhs The reference to set.
		WeakRef(const Ref<_Ty>& rhs) : m_vtable(rhs.vtable()) { internal_addref(); }
        //! Assigns this reference by coping the pointer from one strong reference of the same type.
        //! @details The weak reference counter of the new boxed object, if not null, will be increased.
        //! The weak reference counter of the original boxed object, if not null, will be decreased before assignment.
		//! @param[in] rhs The reference to set.
		WeakRef& operator=(const Ref<_Ty>& rhs) { internal_clear(); m_vtable = rhs.get(); internal_addref(); return *this; }
        //! Compares two references for equality. 
		//! @details Two references are equal if their underlying pointers are either equal or both invalid.
		//! @param[in] rhs The reference to compare with.
		//! @return Returns `true` if two references are equal. Returns `false` otherwise.
		bool operator== (const WeakRef& rhs) const { return (usize)object() == (usize)rhs.object(); }
         //! Compares two references for non-equality. 
		//! @details Two references are equal if their underlying pointers are either equal or both invalid.
		//! @param[in] rhs The reference to compare with.
		//! @return Returns `true` if two references are not equal. Returns `false` otherwise.
		bool operator!= (const WeakRef& rhs) const { return (usize)object() != (usize)rhs.object(); }
        //! Compares two references. 
		//! @details The referneces are compared by comparing their underlying pointers after converted to unsigned integers.
        //! If the reference is not valid, the converted integer will be `0`.
		//! @param[in] rhs The reference to compare with.
		//! @return Returns `true` if this reference is less than the incoming reference. Returns `false` otherwise.
		bool operator< (const WeakRef& rhs) const { return (usize)object() < (usize)rhs.object(); }
        //! Checks whether this reference is valid.
		//! @details One weak reference is valid when it is not null, and the boxed object is not expired.
		//! @return Returns `true` when the reference is valid. Returns `false` otherwise.
        operator bool() const { return valid(); }
        //! Creates one strong reference from this weak reference.
        //! @return Returns the created strong reference if this weak reference is valid.
        //! Returns one null reference if this weak reference is not valid.
		Ref<_Ty> pin() const
		{
			object_t obj = InterfaceAdapter<has_get_object>::get_object(m_vtable);
			if (obj && !object_retain_if_not_expired(obj))
			{
				internal_clear();
				return Ref<_Ty>();
			}
			Ref<_Ty> ret;
			ret.attach(obj);
			return ret;
		}
	};

	//! @}
}