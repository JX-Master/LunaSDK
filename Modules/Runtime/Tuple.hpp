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
	template <typename _Ty, typename... _Tys>
	class Tuple
	{
	public:
		_Ty value;
		Tuple<_Tys...> rest;

		Tuple() {}
		Tuple(const _Ty& arg1, const _Tys&... args) :
			value(arg1),
			rest(args...) {}
		template <typename _UTy1, typename... _UTys>
		Tuple(_UTy1&& arg1, _UTys&&... args) :
			value(forward<_UTy1>(arg1)),
			rest(forward<_UTys>(args)...) {}
		template <typename _UTy1, typename... _UTys>
		Tuple(const Tuple<_UTy1, _UTys...>& rhs) :
			value(rhs.value),
			rest(rhs.rest) {}
		template <typename _UTy1, typename... _UTys>
		Tuple(Tuple<_UTy1, _UTys...>&& rhs) :
			value(move(rhs.value)),
			rest(move(rhs.rest)) {}
		Tuple(const Tuple&) = default;
		Tuple(Tuple&&) = default;
		Tuple& operator=(const Tuple& rhs)
		{
			value = rhs.value;
			rest = rhs.rest;
			return *this;
		}
		Tuple& operator=(Tuple&& rhs)
		{
			value = move(rhs.value);
			rest = move(rhs.rest);
			return *this;
		}
		template <typename _UTy1, typename... _UTys>
		Tuple& operator=(const Tuple<_UTy1, _UTys...>& rhs)
		{
			value = rhs.value;
			rest = rhs.rest;
			return *this;
		}
		template <typename _UTy1, typename... _UTys>
		Tuple& operator=(Tuple<_UTy1, _UTys...>&& rhs)
		{
			value = move(rhs.value);
			rest = move(rhs.rest);
			return *this;
		}
		bool operator==(const Tuple& rhs) const
		{
			return value == rhs.value && rest == rhs.rest;
		}
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

	// base case
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

	template <usize _I, typename... _Tys>
	typename TupleElement<_I, Tuple<_Tys...>>::type& get(Tuple<_Tys...>& t)
	{
		return Impl::TupleGetter<_I>::get(t);
	}

	template <usize _I, typename... _Tys>
	typename TupleElement<_I, Tuple<_Tys...>>::type&& get(Tuple<_Tys...>&& t)
	{
		return Impl::TupleGetter<_I>::get(move(t));
	}

	template <usize _I, typename... _Tys>
	typename TupleElement<_I, Tuple<_Tys...>>::type const& get(const Tuple<_Tys...>& t)
	{
		return Impl::TupleGetter<_I>::get(t);
	}

	template <usize _I, typename... _Tys>
	typename TupleElement<_I, Tuple<_Tys...>>::type const&& get(const Tuple<_Tys...>&& t)
	{
		return Impl::TupleGetter<_I>::get(move(t));
	}

	template <usize _I, typename... _Tys>
	typename TupleElement<_I, Tuple<_Tys...>>::type volatile& get(volatile Tuple<_Tys...>& t)
	{
		return Impl::TupleGetter<_I>::get(t);
	}

	template <usize _I, typename... _Tys>
	typename TupleElement<_I, Tuple<_Tys...>>::type volatile&& get(volatile Tuple<_Tys...>&& t)
	{
		return Impl::TupleGetter<_I>::get(move(t));
	}

	template <usize _I, typename... _Tys>
	typename TupleElement<_I, Tuple<_Tys...>>::type const volatile& get(const volatile Tuple<_Tys...>& t)
	{
		return Impl::TupleGetter<_I>::get(t);
	}

	template <usize _I, typename... _Tys>
	typename TupleElement<_I, Tuple<_Tys...>>::type const volatile&& get(const volatile Tuple<_Tys...>&& t)
	{
		return Impl::TupleGetter<_I>::get(move(t));
	}

	//! Used to unpack tuple when using tuple to store function arguments.
	template <usize... _Indices>
	struct IndexTuple
	{
		using next = IndexTuple<_Indices..., sizeof...(_Indices)>;
	};

	template <usize _N>
	struct build_index_tuple
	{
		using type = typename build_index_tuple<_N - 1>::type::next;
	};

	template <>
	struct build_index_tuple<0>
	{
		using type = IndexTuple<>;
	};

	template <typename _Ty> struct TupleSize;

	template<typename... _Types>
	struct TupleSize<Tuple<_Types...>> : integral_constant<usize, sizeof...(_Types)> { };

	template <typename... _Args>
	inline Tuple<_Args&&...> forward_as_tuple(_Args&&... args)
	{
		return Tuple<_Types&&...>(forward<_Args>(args)...);
	}

	LUNA_RUNTIME_API typeinfo_t tuple_type();
	template <typename... _Tys> struct typeof_t<Tuple<_Tys...>>
	{
		typeinfo_t operator()() const { return get_generic_instanced_type(tuple_type(), { typeof<_Tys>()... }); }
	};
}