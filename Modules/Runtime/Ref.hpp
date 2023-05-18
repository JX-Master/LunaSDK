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
	//! Represents one typeless strong reference to one boxed object.
	class ObjRef
	{
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
	public:
		void reset() { internal_clear(); }
		bool valid() const { return m_obj != nullptr; }
		object_t get() const { return m_obj; }
		void attach(object_t ptr) { internal_clear(); m_obj = ptr; }
		object_t detach() { return atom_exchange_pointer(&m_obj, nullptr); }
		object_t* get_address_of() { return &m_obj; }
		object_t* release_and_get_address_of() { internal_clear(); return &m_obj; }
		ObjRef() : m_obj{ nullptr } {}
		~ObjRef() { internal_clear(); }
		ObjRef(const ObjRef& rhs) : m_obj{ rhs.m_obj } { internal_addref(); }
		ObjRef(ObjRef&& rhs) : m_obj{ atom_exchange_pointer(&rhs.m_obj, nullptr) } {}
		ObjRef& operator=(const ObjRef& rhs) { internal_clear(); m_obj = rhs.m_obj; internal_addref(); return *this; }
		ObjRef& operator=(ObjRef&& rhs) { internal_clear(); m_obj = atom_exchange_pointer(&rhs.m_obj, nullptr); return *this; }
		explicit ObjRef(object_t rhs) : m_obj(rhs) { internal_addref(); }
		ObjRef& operator=(object_t rhs) { internal_clear(); m_obj = rhs; internal_addref(); return *this; }
		bool operator== (const ObjRef& rhs) const { return m_obj == rhs.m_obj; }
		bool operator!= (const ObjRef& rhs) const { return m_obj != rhs.m_obj; }
		bool operator< (const ObjRef& rhs) const { return (usize)m_obj < (usize)(rhs.m_obj); }
		typeinfo_t type() const { return m_obj ? get_object_type(m_obj) : nullptr; }
		operator bool() const { return valid(); }
	};

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
		//! resets the smart pointer and makes it point to `nullptr`.
		void reset() { internal_clear(); }
		//! Tests if this pointer is valid.
		//! A pointer is valid if it is not `nullptr`.
		bool valid() const { return m_vtable != nullptr; }
		//! Gets the underlying object pointer without adding reference count.
		object_t object() const { return InterfaceAdapter<has_get_object>::get_object(m_vtable); }
		//! Gets the underlying pointer to object or interface.
		_Ty* get() const { luassert(m_vtable); return m_vtable; }
		//! Same as `get()`
		_Ty* operator->() const { return get(); }
		//! Releases the underlying pointer and attaches the specified pointer without increasing ref count.
		void attach(object_t ptr)
		{
			internal_clear();
			if (ptr)
			{
				m_vtable = (_Ty*)internal_query_interface(ptr, _Ty::__guid);
				if (!m_vtable) object_release(ptr);
			}
		}
		//! Sets the underlying pointer to `nullptr` and return the pointer.
		object_t detach() { return InterfaceAdapter<has_get_object>::detach(&m_vtable); }
		Ref() : m_vtable(nullptr) {}
		~Ref() { internal_clear(); }
		// Ctor from Ref of the same type.
		Ref(const Ref& rhs) : m_vtable(rhs.m_vtable) { internal_addref(); }
		Ref(Ref&& rhs) : m_vtable{ atom_exchange_pointer(&rhs.m_vtable, nullptr) } {}
		Ref& operator=(const Ref& rhs) { internal_clear(); m_vtable = rhs.m_vtable; internal_addref(); return *this; }
		Ref& operator=(Ref&& rhs) { internal_clear(); m_vtable = atom_exchange_pointer(&rhs.m_vtable, nullptr); return *this; }
		// Ctor from another `Ref` of different type.
		template <typename _Rty>
		Ref(const Ref<_Rty>& rhs)
		{
			object_t obj = rhs.object();
			m_vtable = obj ? (_Ty*)internal_query_interface(obj, _Ty::__guid) : nullptr;
			internal_addref();
		}
		template <typename _Rty>
		Ref& operator=(const Ref<_Rty>& rhs)
		{
			internal_clear();
			object_t obj = rhs.object();
			m_vtable = obj ? (_Ty*)internal_query_interface(obj, _Ty::__guid) : nullptr;
			internal_addref();
			return *this;
		}
		template <typename _Rty>
		Ref(Ref<_Rty>&& rhs)
		{
			object_t obj = rhs.detach();
			m_vtable = obj ? (_Ty*)internal_query_interface(obj, _Ty::__guid) : nullptr;
			if (obj && !m_vtable) object_release(obj);
		}
		template <typename _Rty>
		Ref& operator=(Ref<_Rty>&& rhs)
		{
			internal_clear();
			object_t obj = rhs.detach();
			m_vtable = obj ? (_Ty*)internal_query_interface(obj, _Ty::__guid) : nullptr;
			if (obj && !m_vtable) object_release(obj);
			return *this;
		}
		Ref(_Ty* vtable)
		{
			m_vtable = vtable;
			internal_addref();
		}
		Ref& operator=(_Ty* vtable)
		{
			internal_clear();
			m_vtable = vtable;
			internal_addref();
			return *this;
		}
		template <typename _Rty, typename _Enable = enable_if_t<is_base_of_v<Interface, _Rty>>>
		Ref(_Rty* vtable)
		{
			_Ty* v = query_interface<_Ty>(vtable->get_object());
			m_vtable = v;
			internal_addref();
		}
		Ref(const ObjRef& obj)
		{
			if (obj)
			{
				m_vtable = (_Ty*)internal_query_interface(obj.get(), _Ty::__guid);
				internal_addref();
			}
			else
			{
				m_vtable = nullptr;
			}
		}
		Ref(ObjRef&& obj)
		{
			object_t ptr = obj.detach();
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
		Ref& operator=(const ObjRef& obj)
		{
			internal_clear();
			if (obj)
			{
				m_vtable = (_Ty*)internal_query_interface(obj.get(), _Ty::__guid);
				internal_addref();
			}
			return *this;
		}
		Ref& operator=(ObjRef&& obj)
		{
			internal_clear();
			object_t ptr = obj.detach();
			if (ptr)
			{
				m_vtable = (_Ty*)internal_query_interface(ptr, _Ty::__guid);
				if (!m_vtable) object_release(ptr);
			}
			return *this;
		}
		bool operator== (const Ref& rhs) const { return object() == rhs.object(); }
		bool operator!= (const Ref& rhs) const { return object() != rhs.object(); }
		bool operator== (_Ty* rhs) const { return m_vtable == rhs; }
		bool operator!= (_Ty* rhs) const { return m_vtable != rhs; }
		bool operator< (const Ref& rhs) const { return (usize)object() < (usize)rhs.object(); }
		operator _Ty* () const { return m_vtable; }
		template <typename _Rty>
		_Rty* as() const
		{
			auto obj = object();
			if (!obj) return nullptr;
			return (_Rty*)internal_query_interface(obj, _Rty::__guid);
		}
	};

	//! Creates an Ref smart pointer from a raw pointer without increasing its reference count.
	template <typename _Ty>
	Ref<_Ty> box_ptr(_Ty* obj)
	{
		Ref<_Ty> r;
		r.attach(obj);
		return r;
	}
	//! Creates one reference counted object.
	template <typename _Ty, typename... _Args>
	inline Ref<_Ty> new_object(_Args&&... args)
	{
		_Ty* o = reinterpret_cast<_Ty*>(object_alloc(typeof<_Ty>()));
		new (o) _Ty(forward<_Args>(args)...);
		return box_ptr(o);
	}

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
		void reset() { internal_clear(); }
		bool valid() const { return internal_get() != nullptr; }
		object_t get() const { return internal_get(); }
		void attach(object_t ptr) { internal_clear(); m_obj = ptr; }
		object_t detach() { object_t r = internal_get(); m_obj = nullptr; return r; }
		WeakObjRef() : m_obj{ nullptr } {}
		~WeakObjRef() { internal_clear(); }
		WeakObjRef(const WeakObjRef& rhs) : m_obj{ rhs.get() } { internal_addref(); }
		WeakObjRef(WeakObjRef&& rhs) : m_obj{ rhs.detach() } {}
		WeakObjRef& operator=(const WeakObjRef& rhs) { internal_clear(); m_obj = rhs.get(); internal_addref(); return *this; }
		WeakObjRef& operator=(WeakObjRef&& rhs) { internal_clear(); m_obj = rhs.detach(); return *this; }
		explicit WeakObjRef(const ObjRef& rhs) : m_obj(rhs.get()) { internal_addref(); }
		WeakObjRef& operator=(const ObjRef& rhs) { internal_clear(); m_obj = rhs.get(); internal_addref(); return *this; }
		explicit WeakObjRef(object_t rhs) : m_obj(rhs) { internal_addref(); }
		WeakObjRef& operator=(object_t rhs) { internal_clear(); m_obj = rhs; internal_addref(); return *this; }
		bool operator== (const WeakObjRef& rhs) const { return get() == rhs.get(); }
		bool operator!= (const WeakObjRef& rhs) const { return get() != rhs.get(); }
		bool operator< (const WeakObjRef& rhs) const { return (usize)get() < (usize)(rhs.get()); }
		object_t pin() const
		{
			if (m_obj && !object_retain_if_not_expired(m_obj)) internal_clear();
			return m_obj;
		}
	};
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
		//! resets the smart pointer and makes it point to `nullptr`.
		void reset() { internal_clear(); }
		//! Tests if this pointer is valid.
		//! A pointer is valid if it is not `nullptr`.
		bool valid() const { return internal_get() != nullptr; }
		//! Gets the underlying object pointer without adding reference count.
		object_t object() const { return internal_get(); }
		WeakRef() : m_vtable(nullptr) {}
		~WeakRef() { internal_clear(); }
		// Ctor from another WeakRef of the same type.
		WeakRef(const WeakRef& rhs) : m_vtable(rhs.m_vtable) { internal_addref(); }
		WeakRef(WeakRef&& rhs) : m_vtable{ atom_exchange_pointer(&rhs.m_vtable, nullptr) } {}
		WeakRef& operator=(const WeakRef& rhs) { internal_clear(); m_vtable = rhs.m_vtable; internal_addref(); return *this; }
		WeakRef& operator=(WeakRef&& rhs) { internal_clear(); m_vtable = atom_exchange_pointer(&rhs.m_vtable, nullptr); return *this; }
		// Ctor from Ref of the same type.
		WeakRef(const Ref<_Ty>& rhs) : m_vtable(rhs.vtable()) { internal_addref(); }
		WeakRef& operator=(const Ref<_Ty>& rhs) { internal_clear(); m_vtable = rhs.get(); internal_addref(); return *this; }
		bool operator== (const WeakRef& rhs) const { return (usize)object() == (usize)rhs.object(); }
		bool operator!= (const WeakRef& rhs) const { return (usize)object() != (usize)rhs.object(); }
		bool operator< (const WeakRef& rhs) const { return (usize)object() < (usize)rhs.object(); }
		_Ty* vtable() const { return m_vtable; }
		operator bool() const { return valid(); }
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
}