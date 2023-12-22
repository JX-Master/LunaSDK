/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Tuple.hpp
* @author JXMaster
* @date 2021/9/23
*/
#pragma once
#include "TypeInfo.hpp"
namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	
	//! Represents a sequence of fixed-size elements. Every element can have one different type.
	//! @details Elements of one tuple type can be fetched by @ref get.
	template <typename _Ty, typename... _Tys>
	class Tuple
	{
	public:
		_Ty value;
		Tuple<_Tys...> rest;

		//! Constructs one tuple with every element being default-initialized.
		Tuple() {}
		//! Constructs one tuple with every element being copy-initialized.
		//! @param[in] arg1 The value used to initialize the first tuple element.
		//! @param[in] args Values used to initialize the rest tuple elements.
		Tuple(const _Ty& arg1, const _Tys&... args) :
			value(arg1),
			rest(args...) {}
		//! Constructs one tuple with every element being converted from the specified value.
		//! @details Values are passed to the constructor of elements using @ref forward.
		//! @param[in] arg1 The value used to initialize the first tuple element.
		//! @param[in] args Values used to initialize the rest tuple elements.
		template <typename _UTy1, typename... _UTys>
		Tuple(_UTy1&& arg1, _UTys&&... args) :
			value(forward<_UTy1>(arg1)),
			rest(forward<_UTys>(args)...) {}
		//! Constructs one tuple by coping elements from another tuple.
		//! @details The element types of two tuples do not need to be equal, so long as every element in target 
		//! tuple can be copy constructed from the corresponding element in source tuple.
		//! @param[in] rhs The tuple to copy elements from.
		template <typename _UTy1, typename... _UTys>
		Tuple(const Tuple<_UTy1, _UTys...>& rhs) :
			value(rhs.value),
			rest(rhs.rest) {}
		//! Constructs one tuple by moving elements from another tuple.
		//! @details The element types of two tuples do not need to be equal, so long as every element in target 
		//! tuple can be move constructed from the corresponding element in source tuple.
		//! @param[in] rhs The tuple to move elements from.
		template <typename _UTy1, typename... _UTys>
		Tuple(Tuple<_UTy1, _UTys...>&& rhs) :
			value(move(rhs.value)),
			rest(move(rhs.rest)) {}
		//! Constructs one tuple by coping elements from another tuple of the same type.
		//! @param[in] rhs The tuple to copy elements from.
		Tuple(const Tuple& rhs) = default;
		//! Constructs one tuple by moving elements from another tuple of the same type.
		//! @param[in] rhs The tuple to move elements from.
		Tuple(Tuple&& rhs) = default;
		//! Assigns elements of one tuple by coping elements from another tuple of the same type.
		//! @param[in] rhs The tuple to copy elements from.
		Tuple& operator=(const Tuple& rhs)
		{
			value = rhs.value;
			rest = rhs.rest;
			return *this;
		}
		//! Assigns elements of one tuple by moving elements from another tuple of the same type.
		//! @param[in] rhs The tuple to move elements from.
		Tuple& operator=(Tuple&& rhs)
		{
			value = move(rhs.value);
			rest = move(rhs.rest);
			return *this;
		}
		//! Assigns elements of one tuple by coping elements from another tuple.
		//! @details The element types of two tuples do not need to be equal, so long as every element in target 
		//! tuple can be copy assigned from the corresponding element in source tuple.
		//! @param[in] rhs The tuple to copy elements from.
		template <typename _UTy1, typename... _UTys>
		Tuple& operator=(const Tuple<_UTy1, _UTys...>& rhs)
		{
			value = rhs.value;
			rest = rhs.rest;
			return *this;
		}
		//! Assigns elements of one tuple by moving elements from another tuple.
		//! @details The element types of two tuples do not need to be equal, so long as every element in target 
		//! tuple can be move constructed from the corresponding element in source tuple.
		//! @param[in] rhs The tuple to move elements from.
		template <typename _UTy1, typename... _UTys>
		Tuple& operator=(Tuple<_UTy1, _UTys...>&& rhs)
		{
			value = move(rhs.value);
			rest = move(rhs.rest);
			return *this;
		}
		//! Compares two tuples for equality.
		//! @param[in] rhs The tuple to compare with.
		//! @return Returns `true` if two tuples are equal. Returns `false` otherwise.
		bool operator==(const Tuple& rhs) const
		{
			return value == rhs.value && rest == rhs.rest;
		}
		//! Compares two tuples for non-equality.
		//! @param[in] rhs The tuple to compare with.
		//! @return Returns `true` if two tuples are not equal. Returns `false` otherwise.
		bool operator!=(const Tuple& rhs) const
		{
			return !(*this == rhs);
		}
	};

	template <typename _Ty>
	class Tuple<_Ty>
	{
	public:
		_Ty value;

		Tuple() {}
		Tuple(const _Ty& arg1) :
			value(arg1) {}
		template <typename _UTy>
		Tuple(_UTy&& arg1) :
			value(forward<_UTy>(arg1)) {}
		template <typename _UTy>
		Tuple(const Tuple<_UTy>& rhs) :
			value(rhs.value) {}
		template <typename _UTy>
		Tuple(Tuple<_UTy>&& rhs) :
			value(move(rhs.value)) {}
		Tuple(const Tuple&) = default;
		Tuple(Tuple&&) = default;
		Tuple& operator=(const Tuple& rhs)
		{
			value = rhs.value;
			return *this;
		}
		Tuple& operator=(Tuple&& rhs)
		{
			value = move(rhs.value);
			return *this;
		}
		template <typename _UTy1>
		Tuple& operator=(const Tuple<_UTy1>& rhs)
		{
			value = rhs.value;
			return *this;
		}
		template <typename _UTy1>
		Tuple& operator=(Tuple<_UTy1>&& rhs)
		{
			value = move(rhs.value);
			return *this;
		}
		bool operator==(const Tuple& rhs) const
		{
			return value == rhs.value;
		}
		bool operator!=(const Tuple& rhs) const
		{
			return !(*this == rhs);
		}
	};

	template<usize _I, typename _Ty >
	struct TupleElement;

	template<usize _I, typename _Ty, typename... _Tys>
	struct TupleElement<_I, Tuple<_Ty, _Tys...>> : TupleElement<_I - 1, Tuple<_Tys...>> 
	{
		static_assert(_I < sizeof...(_Tys) + 1, "Index out of bounds");
	};

	template<typename _Ty, typename... _Tys >
	struct TupleElement<0, Tuple<_Ty, _Tys...>> 
	{
		using type = _Ty;
	};

	namespace Impl
	{
		template <usize _I>
		struct TupleGetter
		{
			template <typename... _Tys>
			static typename TupleElement<_I, Tuple<_Tys...>>::type& get(Tuple<_Tys...>& t)
			{
				return TupleGetter<_I - 1>::get(t.rest);
			}
			template <typename... _Tys>
			static typename TupleElement<_I, Tuple<_Tys...>>::type&& get(Tuple<_Tys...>&& t)
			{
				return TupleGetter<_I - 1>::get(t.rest);
			}
			template <typename... _Tys>
			static typename TupleElement<_I, Tuple<_Tys...>>::type const& get(const Tuple<_Tys...>& t)
			{
				return TupleGetter<_I - 1>::get(t.rest);
			}
			template <typename... _Tys>
			static typename TupleElement<_I, Tuple<_Tys...>>::type const&& get(const Tuple<_Tys...>&& t)
			{
				return TupleGetter<_I - 1>::get(t.rest);
			}
			template <typename... _Tys>
			static typename TupleElement<_I, Tuple<_Tys...>>::type volatile& get(volatile Tuple<_Tys...>& t)
			{
				return TupleGetter<_I - 1>::get(t.rest);
			}
			template <typename... _Tys>
			static typename TupleElement<_I, Tuple<_Tys...>>::type volatile&& get(volatile Tuple<_Tys...>&& t)
			{
				return TupleGetter<_I - 1>::get(t.rest);
			}
			template <typename... _Tys>
			static typename TupleElement<_I, Tuple<_Tys...>>::type const volatile& get(const volatile Tuple<_Tys...>& t)
			{
				return TupleGetter<_I - 1>::get(t.rest);
			}
			template <typename... _Tys>
			static typename TupleElement<_I, Tuple<_Tys...>>::type const volatile&& get(const volatile Tuple<_Tys...>&& t)
			{
				return TupleGetter<_I - 1>::get(t.rest);
			}
		};

		template <>
		struct TupleGetter<0>
		{
			template <typename _Ty, typename... _Tys>
			static _Ty& get(Tuple<_Ty, _Tys...>& t)
			{
				return t.value;
			}
			template <typename _Ty, typename... _Tys>
			static _Ty& get(Tuple<_Ty, _Tys...>&& t)
			{
				return move(t.value);
			}
			template <typename _Ty, typename... _Tys>
			static const _Ty& get(const Tuple<_Ty, _Tys...>& t)
			{
				return t.value;
			}
			template <typename _Ty, typename... _Tys>
			static const _Ty& get(const Tuple<_Ty, _Tys...>&& t)
			{
				return move(t.value);
			}
			template <typename _Ty, typename... _Tys>
			static volatile _Ty& get(volatile Tuple<_Ty, _Tys...>& t)
			{
				return t.value;
			}
			template <typename _Ty, typename... _Tys>
			static volatile _Ty& get(volatile Tuple<_Ty, _Tys...>&& t)
			{
				return move(t.value);
			}
			template <typename _Ty, typename... _Tys>
			static const volatile _Ty& get(const volatile Tuple<_Ty, _Tys...>& t)
			{
				return t.value;
			}
			template <typename _Ty, typename... _Tys>
			static const volatile _Ty& get(const volatile Tuple<_Ty, _Tys...>&& t)
			{
				return move(t.value);
			}
		};
	}

	//! Gets the `_I`th element from the tuple.
	//! @param[in] t The tuple to extract element from.
	//! @return Returns one lvalue reference to the element in the tuple.
	template <usize _I, typename... _Tys>
	typename TupleElement<_I, Tuple<_Tys...>>::type& get(Tuple<_Tys...>& t)
	{
		return Impl::TupleGetter<_I>::get(t);
	}

	//! Gets the `_I`th element from the tuple.
	//! @param[in] t The tuple to extract element from.
	//! @return Returns one rvalue reference to the element in the tuple.
	template <usize _I, typename... _Tys>
	typename TupleElement<_I, Tuple<_Tys...>>::type&& get(Tuple<_Tys...>&& t)
	{
		return Impl::TupleGetter<_I>::get(move(t));
	}

	//! Gets the `_I`th element from the tuple.
	//! @param[in] t The tuple to extract element from.
	//! @return Returns one lvalue constant reference to the element in the tuple.
	template <usize _I, typename... _Tys>
	typename TupleElement<_I, Tuple<_Tys...>>::type const& get(const Tuple<_Tys...>& t)
	{
		return Impl::TupleGetter<_I>::get(t);
	}

	//! Gets the `_I`th element from the tuple.
	//! @param[in] t The tuple to extract element from.
	//! @return Returns one rvalue constant reference to the element in the tuple.
	template <usize _I, typename... _Tys>
	typename TupleElement<_I, Tuple<_Tys...>>::type const&& get(const Tuple<_Tys...>&& t)
	{
		return Impl::TupleGetter<_I>::get(move(t));
	}

	//! Gets the `_I`th element from the tuple.
	//! @param[in] t The tuple to extract element from.
	//! @return Returns one lvalue volatile reference to the element in the tuple.
	template <usize _I, typename... _Tys>
	typename TupleElement<_I, Tuple<_Tys...>>::type volatile& get(volatile Tuple<_Tys...>& t)
	{
		return Impl::TupleGetter<_I>::get(t);
	}

	//! Gets the `_I`th element from the tuple.
	//! @param[in] t The tuple to extract element from.
	//! @return Returns one rvalue volatile reference to the element in the tuple.
	template <usize _I, typename... _Tys>
	typename TupleElement<_I, Tuple<_Tys...>>::type volatile&& get(volatile Tuple<_Tys...>&& t)
	{
		return Impl::TupleGetter<_I>::get(move(t));
	}

	//! Gets the `_I`th element from the tuple.
	//! @param[in] t The tuple to extract element from.
	//! @return Returns one lvalue constant volatile reference to the element in the tuple.
	template <usize _I, typename... _Tys>
	typename TupleElement<_I, Tuple<_Tys...>>::type const volatile& get(const volatile Tuple<_Tys...>& t)
	{
		return Impl::TupleGetter<_I>::get(t);
	}

	//! Gets the `_I`th element from the tuple.
	//! @param[in] t The tuple to extract element from.
	//! @return Returns one rvalue constant volatile reference to the element in the tuple.
	template <usize _I, typename... _Tys>
	typename TupleElement<_I, Tuple<_Tys...>>::type const volatile&& get(const volatile Tuple<_Tys...>&& t)
	{
		return Impl::TupleGetter<_I>::get(move(t));
	}

	//! Gets the type object of @ref Tuple.
	//! @return Returns the type object of @ref Tuple.
	LUNA_RUNTIME_API typeinfo_t tuple_type();
	template <typename... _Tys> struct typeof_t<Tuple<_Tys...>>
	{
		typeinfo_t operator()() const { return get_generic_instanced_type(tuple_type(), { typeof<_Tys>()... }); }
	};

	//! @}
}