/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Functional.hpp
* @author JXMaster
* @date 2020/2/14
*/
#pragma once
#include "Hash.hpp"
#include "MemoryUtils.hpp"
#include "Tuple.hpp"
namespace Luna
{
	template <typename _Ty>
	struct equal_to
	{
		constexpr bool operator()(const _Ty& lhs, const _Ty& rhs) const
		{
			return lhs == rhs;
		}
	};

	template <typename _Ty>
	struct less
	{
		constexpr bool operator()(const _Ty& lhs, const _Ty& rhs) const
		{
			return lhs < rhs;
		}
	};

	//! hash class is a function object that hashes the specified type into a usize 
	//! hash code that can be used in hash map and hash set.
	template <typename _Ty> struct hash;

	template <> struct hash<bool>
	{
		usize operator()(bool val) const { return static_cast<usize>(val); }
	};

    template <> struct hash<char>
    {
        usize operator()(char val) const { return static_cast<usize>(val); }
    };

    template <> struct hash<signed char>
    {
        usize operator()(signed char val) const { return static_cast<usize>(val); }
    };

    template <> struct hash<unsigned char>
    {
        usize operator()(unsigned char val) const { return static_cast<usize>(val); }
    };

	template <> struct hash<char16_t>
	{
		usize operator()(char16_t val) const { return static_cast<usize>(val); }
	};

	template <> struct hash<char32_t>
	{
		usize operator()(char32_t val) const { return static_cast<usize>(val); }
	};

	template <> struct hash<wchar_t>
	{
		usize operator()(wchar_t val) const { return static_cast<usize>(val); }
	};

    template <> struct hash<short>
    {
        usize operator()(short val) const { return static_cast<usize>(val); }
    };

	template <> struct hash<unsigned short>
	{
		usize operator()(unsigned short val) const { return static_cast<usize>(val); }
	};

	template <> struct hash<int>
	{
		usize operator()(int val) const { return static_cast<usize>(val); }
	};

	template <> struct hash<unsigned int>
	{
		usize operator()(unsigned int val) const { return static_cast<usize>(val); }
	};

    template <> struct hash<long>
    {
        usize operator()(long val) const { return static_cast<usize>(val); }
    };

    template <> struct hash<unsigned long>
    {
        usize operator()(unsigned long val) const { return static_cast<usize>(val); }
    };

	template <> struct hash<long long>
	{
		usize operator()(long long val) const { return static_cast<usize>(val); }
	};

	template <> struct hash<unsigned long long>
	{
		usize operator()(unsigned long long val) const { return static_cast<usize>(val); }
	};

	template <> struct hash<float>
	{
		usize operator()(float val) const { return static_cast<usize>(val); }
	};

	template <> struct hash<double>
	{
		usize operator()(double val) const { return static_cast<usize>(val); }
	};

    template <> struct hash<long double>
    {
        usize operator()(long double val) const { return static_cast<usize>(val); }
    };

	template <typename _Ty>
	struct hash<_Ty*>
	{
		usize operator()(_Ty* val) const { return (usize)(val); }
	};

	template <> struct hash<Guid>
	{
		usize operator()(const Guid& guid) const
		{
#ifdef LUNA_PLATFORM_64BIT
			return (usize)(guid.low ^ guid.high);
#else
			return (usize)((u32)guid.low ^ ((u32)(guid.low >> 32)) ^ (u32)guid.high ^ ((u32)(guid.high >> 32)));
#endif
		}
	};

	/*template <typename _Func, typename... _Args>
	invoke_result_t<_Func, _Args...> invoke(_Func&& f, _Args&&... args);*/

	template <typename _Ty>
	class ReferenceWrapper
	{
	private:
		_Ty* m_val;
	public:
		using type = _Ty;

		ReferenceWrapper(_Ty& v) :
			m_val(addressof(v)) {}
		ReferenceWrapper(_Ty&&) = delete;
		ReferenceWrapper(const ReferenceWrapper<_Ty>& v) :
			m_val(v.m_val) {}
		ReferenceWrapper& operator=(const ReferenceWrapper<_Ty>& v)
		{
			m_val = v.m_val;
			return *this;
		}
		operator _Ty& () const
		{
			return *m_val;
		}
		_Ty& get() const
		{
			return *m_val;
		}
		template <typename... _Args>
		invoke_result_t<_Ty&, _Args...> operator()(_Args&&... args) const
		{
			return invoke(get(), forward<_Args>(args)...);
		}
	};

	template <typename _Ty>
	struct is_reference_wrapper : public false_type {};
	template <typename _Ty>
	struct is_reference_wrapper<ReferenceWrapper<_Ty>> : public true_type {};
	template <typename _Ty>
	struct is_reference_wrapper<const ReferenceWrapper<_Ty>> : public true_type {};
	template <typename _Ty>
	struct is_reference_wrapper<volatile ReferenceWrapper<_Ty>> : public true_type {};
	template <typename _Ty>
	struct is_reference_wrapper<const volatile ReferenceWrapper<_Ty>> : public true_type {};

	/*namespace Impl
	{
		template <typename _Class, typename _Pointed, typename _T1, typename... _Args>
		inline decltype(auto) invoke_member_pointer(_Pointed _Class::* f, _T1&& t1, _Args&&... args)
		{
			LUNA_IF_CONSTEXPR(is_function_v<_Pointed>)
			{
				LUNA_IF_CONSTEXPR(is_base_of_v<_Class, decay_t<_T1>>)
					return (forward<_T1>(t1).*f)(forward<_Args>(args)...);
				else LUNA_IF_CONSTEXPR(is_reference_wrapper_v<decay_t<_T1>>)
					return (t1.get().*f)(forward<_Args>(args)...);
				else
					return ((*forward<_T1>(t1)).*f)(forward<_Args>(args)...);
			}
			else
			{
				static_assert(is_object_v<_Pointed> && sizeof...(args) == 0);
				LUNA_IF_CONSTEXPR(is_base_of_v<_Class, decay_t<_T1>>)
					return forward<_T1>(t1).*f;
				else LUNA_IF_CONSTEXPR(is_reference_wrapper_v<decay_t<_T1>>)
					return t1.get().*f;
				else
					return (*forward<_T1>(t1)).*f;
			}
		}
	}

	template <typename _Func, typename... _Args>
	inline enable_if_t<is_member_pointer_v<decay<_Func>>, invoke_result_t<_Func, _Args...>> invoke(_Func&& f, _Args&&... args)
	{
		return Impl::invoke_member_pointer(f, forward<_Args>(args)...);
	}

	template <typename _Func, typename... _Args>
	inline enable_if_t<!is_member_pointer_v<decay<_Func>>, invoke_result_t<_Func, _Args...>> invoke(_Func&& f, _Args&&... args)
	{
		return forward<_Func>(f)(forward<_Args>(args)...);
	}*/

	//template <typename _Func, typename... _Args>
	//inline invoke_result_t<_Func, _Args...> invoke(_Func&& f, _Args&&... args)
	//{
	//	LUNA_IF_CONSTEXPR(is_member_pointer_v<decay<_Func>>)
	//	{
	//		return Impl::invoke_member_pointer(f, forward<_Args>(args)...);
	//	}
	//	else
	//	{
	//		return forward<_Func>(f)(forward<_Args>(args)...);
	//	}
	//}

	namespace Impl
	{
		struct InvokeFunction
		{
			template <typename _Func, typename... _Args>
			static auto call(_Func&& f, _Args&&... args)
			{
				return f(forward<_Args>(args)...);
			}
		};

		struct InvokeMemberFunc
		{
			template <typename _Func, typename _Obj, typename... _Args>
			static auto call(_Func&& f, _Obj&& obj, _Args&&... args) -> 
				decltype((obj->*f)(forward<_Args>(args)...))
			{
				return (obj->*f)(forward<_Args>(args)...);
			}

			template <typename _Func, typename _Obj, typename... _Args>
			static auto call(_Func&& f, _Obj&& obj, _Args&&... args) ->
				decltype((obj.*f)(forward<_Args>(args)...))
			{
				return (obj.*f)(forward<_Args>(args)...);
			}
		};

		struct InvokeMemberObject
		{
			template <typename _Func, typename _Obj>
			static auto call(_Func&& f, _Obj&& obj) ->
				decltype((obj->*f))
			{
				return (obj->*f);
			}
			template <typename _Func, typename _Obj>
			static auto call(_Func&& f, _Obj&& obj) ->
				decltype((obj.*f))
			{
				return (obj.*f);
			}
		};

		template <typename _Func, typename _FirstTy,
			typename _Decay = decay_t<_Func>,
			bool _IsMemFunc = is_member_function_pointer_v<_Decay>,
			bool _IsMemObj = is_member_object_pointer_v<_Decay>>
			struct InvokeHelper;

		template <typename _Func, typename _FirstTy, typename _Decayed>
		struct InvokeHelper<_Func, _FirstTy, _Decayed, true, false> :
			InvokeMemberFunc {};

		template <typename _Func, typename _FirstTy, typename _Decayed>
		struct InvokeHelper<_Func, _FirstTy, _Decayed, false, true> :
			InvokeMemberObject {};

		template <typename _Func, typename _FirstTy, typename _Decayed>
		struct InvokeHelper<_Func, _FirstTy, _Decayed, false, false> :
			InvokeFunction {};
	}

	template <typename _Func>
	auto invoke(_Func&& f) -> decltype(static_cast<_Func&&>(f)())
	{
		return static_cast<_Func&&>(f)();
	}

	template <typename _Func, typename _Ty, typename... _Args>
	auto invoke(_Func&& f, _Ty&& arg1, _Args&&... args) -> invoke_result_t<_Func, _Ty, _Args...>
	{
		return Impl::InvokeHelper<_Func, _Ty>::template call<_Func, _Ty, _Args...>(forward<_Func>(f), forward<_Ty>(arg1), forward<_Args>(args)...);
	}

	namespace Impl
	{
		template <typename _Return, bool _IsVoidRet = is_void_v<_Return>>
		struct InvokeRHelper;

		template <typename _Return>
		struct InvokeRHelper<_Return, true>
		{
			template <typename _Func, typename... _Args>
			static void call(_Func&& f, _Args&&... args)
			{
				invoke(forward<_Func>(f), forward<_Args>(args)...);
			}
		};

		template <typename _Return>
		struct InvokeRHelper<_Return, false>
		{
			template <typename _Func, typename... _Args>
			static _Return call(_Func&& f, _Args&&... args)
			{
				return invoke(forward<_Func>(f), forward<_Args>(args)...);
			}
		};
	}

	template <typename _Return, typename _Func, typename... _Args>
	inline _Return invoke_r(_Func&& f, _Args&&... args)
	{
		return Impl::InvokeRHelper<_Return>::template call<_Func, _Args...>(forward<_Func>(f), forward<_Args>(args)...);
	}

	namespace Impl
	{
		template <typename _Result, typename... _Args>
		struct FunctionObjectInvoker
		{
			_Result(*m_func)(void* self, _Args... args);
			void* m_self;

			_Result operator()(_Args... args)
			{
				return m_func(m_self, forward<_Args>(args)...);
			}
		};
	}

	//template <typename _Func>
	//struct Function
	//{
	//private:

	//	enum class Type : u8
	//	{
	//		empty = 0,
	//		function = 1,
	//		object = 2,
	//		closure = 3,
	//	};

	//	struct ObjectWitnessTable
	//	{
	//		void (*copy_ctor)(void* dst, const void* src);
	//		void (*move_ctor)(void* dst, void* src);
	//		void (*dtor)(void* obj);
	//	};

	//	Type m_type;

	//	union
	//	{
	//		//! Used if this is a normal function.
	//		_Func* m_func;
	//		//! Used if this is a member function.
	//		
	//	};

	//public:

	//};

	/*namespace Impl
	{
		template <i32 _Num>
		struct PlaceHolder {};
	}*/

	//namespace Placeholders
	//{
	//	inline constexpr Impl::PlaceHolder<1> _1;
	//	inline constexpr Impl::PlaceHolder<2> _2;
	//	inline constexpr Impl::PlaceHolder<3> _3;
	//	inline constexpr Impl::PlaceHolder<4> _4;
	//	inline constexpr Impl::PlaceHolder<5> _5;
	//	inline constexpr Impl::PlaceHolder<6> _6;
	//	inline constexpr Impl::PlaceHolder<7> _7;
	//	inline constexpr Impl::PlaceHolder<8> _8;
	//	inline constexpr Impl::PlaceHolder<9> _9;
	//	inline constexpr Impl::PlaceHolder<10> _10;
	//}

	//template <typename _Ty>
	//struct is_placeholder : public integral_constant<i32, 0> {};
	//template <i32 _Num>
	//struct is_placeholder<Impl::PlaceHolder<_Num>> : public integral_constant<i32, _Num> {};
	//template <i32 _Num>
	//struct is_placeholder<const Impl::PlaceHolder<_Num>> : public integral_constant<i32, _Num> {};

	//template <typename _Ty>
	//inline constexpr i32 is_placeholder_v = is_placeholder<_Ty>::value;

	//template <typename _Signature>
	//struct Bind;
	//template <typename _Result, typename _Signature>
	//struct BindResult;

	//template <typename _Ty>
	//struct is_bind_expression : public false_type {};
	//template <typename _Signature>
	//struct is_bind_expression<Bind<_Signature>> : public true_type {};
	//template <typename _Signature>
	//struct is_bind_expression<const Bind<_Signature>> : public true_type {};
	//template <typename _Signature>
	//struct is_bind_expression<volatile Bind<_Signature>> : public true_type {};
	//template <typename _Signature>
	//struct is_bind_expression<const volatile Bind<_Signature>> : public true_type {};

	//template <typename _Result, typename _Signature>
	//struct is_bind_expression<BindResult<_Result, _Signature>> : public true_type {};
	//template <typename _Result, typename _Signature>
	//struct is_bind_expression<const BindResult<_Result, _Signature>> : public true_type {};
	//template <typename _Result, typename _Signature>
	//struct is_bind_expression<volatile BindResult<_Result, _Signature>> : public true_type {};
	//template <typename _Result, typename _Signature>
	//struct is_bind_expression<const volatile BindResult<_Result, _Signature>> : public true_type {};

	//template <typename _Ty>
	//inline constexpr bool is_bind_expression_v = is_bind_expression<_Ty>::value;

	//namespace Impl
	//{
	//	template <typename _Arg, 
	//		bool _IsBindExp = is_bind_expression<_Arg>::value, 
	//		bool _IsPlaceholder = (is_placeholder<_Arg>::value > 0)>
	//	struct BindArgMapper;

	//	//! ReferenceWrapper case. 
	//	template <typename _Ty>
	//	struct BindArgMapper<ReferenceWrapper<_Ty>, false, false>
	//	{
	//		template <typename _CVRef, typename _Tuple>
	//		_Ty& operator()(_CVRef& arg, _Tuple&) const volatile { return arg.get(); }
	//	};

	//	//! Bind expression case.
	//	template <typename _Arg>
	//	struct BindArgMapper<_Arg, true, false>
	//	{
	//	private:
	//		template<typename _CVArg, typename... _Args, usize... _Indices>
	//		auto call(_CVArg& arg, Tuple<_Args...>& tuple, const IndexTuple<_Indices...>&) const volatile -> decltype(arg(declval<_Args>()...))
	//		{
	//			return arg(get<_Indices>(move(tuple))...);
	//		}
	//	public:
	//		template <typename _CVArg, typename... _Args>
	//		auto operator()(_CVArg& arg, Tuple<_Args...>& tuple) const volatile -> decltype(arg(declval<_Args>()...))
	//		{
	//			using Indices = typename build_index_tuple<sizeof...(_Args)>::type;
	//			return this->call(arg, tuple, Indices());
	//		}
	//	};

	//	//! Bind placeholder case.
	//	template <typename _Arg>
	//	struct BindArgMapper<_Arg, false, true>
	//	{
	//		template<typename _Tuple>
	//		TupleElement<(is_placeholder<_Arg>::value - 1), _Tuple>&& operator()(const volatile _Arg&, _Tuple& tuple) const volatile
	//		{
	//			return get<(is_placeholder<_Arg>::value - 1)>(move(tuple));
	//		}
	//	};

	//	//! Bind normal value case.
	//	template <typename _Arg>
	//	struct BindArgMapper<_Arg, false, false>
	//	{
	//		template <typename _CVArg, typename _Tuple>
	//		_CVArg&& operator()(_CVArg&& arg, _Tuple&) const volatile
	//		{
	//			return forward<_CVArg>(arg);
	//		}
	//	};
	//}

	//template <typename _Func, typename... _BoundArgs>
	//struct Bind<_Func(_BoundArgs...)>
	//{
	//private:
	//	using BoundIndices = typename build_index_tuple<sizeof...(_BoundArgs)>::type;

	//	_Func m_f;
	//	Tuple<_BoundArgs...> m_bound_args;

	//	template <typename _Result, typename... _Args, usize... _Indices>
	//	_Result call(Tuple<_Args...>&& args, IndexTuple<_Indices...>)
	//	{
	//		return invoke(m_f,
	//			Impl::BindArgMapper<_BoundArgs>()(get<_Indices>(m_bound_args), args)...
	//		);
	//	}
	//	template <typename _Result, typename... _Args, usize... _Indices>
	//	_Result call_c(Tuple<_Args...>&& args, IndexTuple<_Indices...>) const
	//	{
	//		return invoke(m_f,
	//			Impl::BindArgMapper<_BoundArgs>()(get<_Indices>(m_bound_args), args)...
	//		);
	//	}
	//	template <typename _Result, typename... _Args, usize... _Indices>
	//	_Result call_v(Tuple<_Args...>&& args, IndexTuple<_Indices...>) volatile
	//	{
	//		return invoke(m_f,
	//			Impl::BindArgMapper<_BoundArgs>()(get<_Indices>(m_bound_args), args)...
	//		);
	//	}
	//	template <typename _Result, typename... _Args, usize... _Indices>
	//	_Result call_cv(Tuple<_Args...>&& args, IndexTuple<_Indices...>) const volatile
	//	{
	//		return invoke(m_f,
	//			Impl::BindArgMapper<_BoundArgs>()(get<_Indices>(m_bound_args), args)...
	//		);
	//	}
	//	template <typename _BoundArg, typename _CallArgs>
	//	using BindArgMapperType = decltype(
	//		Impl::BindArgMapper<remove_cv_t<_BoundArg>>()(
	//			declval<_BoundArg&>(), declval<_CallArgs&>()));

	//	template <typename _Fn, typename _CallArgs, typename... _BArgs>
	//	using ResTypeImpl = invoke_result_t<_Fn&, BindArgMapperType<_BArgs, _CallArgs>&&...>;

	//	template <typename _CallArgs>
	//	using ResType = ResTypeImpl<_Func, _CallArgs, _BoundArgs...>;

	//	template <typename _CallArgs>
	//	using ResTypeC = ResTypeImpl<add_const_t<_Func>, _CallArgs, add_const_t<_BoundArgs>...>;
	//public:
	//	template <typename... _Args>
	//	explicit Bind(const _Func& f, _Args&&... args) :
	//		m_f(f),
	//		m_bound_args(forward<_Args>(args)...) {}
	//	template <typename... _Args>
	//	explicit Bind(_Func&& f, _Args&&... args) :
	//		m_f(move(f)),
	//		m_bound_args(forward<_Args>(args)...) {}
	//	Bind(const Bind&) = default;
	//	Bind(Bind&& b) :
	//		m_f(move(b.m_f)),
	//		m_bound_args(move(b.m_bound_args)) {}

	//	template <typename... _Args, typename _Result = ResType<Tuple<_Args...>>>
	//	_Result operator()(_Args&&... args)
	//	{
	//		return this->call<_Result>(
	//			forward_as_tuple(forward<_Args>(args)...),
	//			BoundIndices());
	//	}
	//	template <typename... _Args, typename _Result = ResTypeC<Tuple<_Args...>>>
	//	_Result operator()(_Args&&... args) const
	//	{
	//		return this->call_c<_Result>(
	//			forward_as_tuple(forward<_Args>(args)...),
	//			BoundIndices());
	//	}
	//};

	//template <typename _Func, typename... _BoundArgs>
	//inline Bind<decay_t<_Func>(decay_t<_BoundArgs>...)> bind(_Func&& f, _BoundArgs&&... args)
	//{
	//	return Bind<decay_t<_Func>(decay_t<_BoundArgs>...)>(
	//		forward<_Func>(f),
	//		forward<_BoundArgs>(args)...);
	//}
}
