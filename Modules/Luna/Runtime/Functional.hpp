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
#include "Assert.hpp"
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

	template <typename _Func>
	struct Function;

	template <typename _R, typename... _Args>
	struct Function<_R(_Args...)>
	{
	private:
		enum class Type : u8
		{
			//! The function is empty.
			empty = 0,
			//! The function contains one function pointer.
			function = 1,
			//! The function contains one callable object.
			object = 2,
		};
		struct ICallable
		{
			virtual ~ICallable() {}
			virtual _R invoke(_Args... args) = 0;
			virtual ICallable* clone() const = 0;
		};
		template <typename _Ty>
		struct Callable : ICallable
		{
			Callable(const _Ty& obj) :
				m_obj(obj) {}
			Callable(_Ty&& obj) :
				m_obj(move(obj)) {}
			virtual _R invoke(_Args... args) override
			{
				return m_obj(forward<_Args>(args)...);
			}
			virtual ICallable* clone() const override
			{
				return memnew<Callable>(m_obj);
			}
			_Ty m_obj;
		};
		using function_t = _R(_Args...);
		Type m_type;
		union
		{
			//! Used if this is a normal function.
			function_t* m_func;
			//! Used if this is a function object.
			ICallable* m_callable;
		};
		void internal_clear()
		{
			if (m_type == Type::object) memdelete(m_callable);
		}
	public:
		using result_type = _R;
		Function() :
			m_type(Type::empty),
			m_func(nullptr) {}
		Function(nullptr_t ) :
			m_type(Type::empty),
			m_func(nullptr) {}
		Function(const Function& rhs) :
			m_type(rhs.m_type)
		{
			if (m_type == Type::object) m_callable = rhs.m_callable->clone();
			else m_func = rhs.m_func;
		}
		Function(Function&& rhs) :
			m_type(rhs.m_type)
		{
			if (m_type == Type::object)
			{
				m_callable = rhs.m_callable;
				rhs.m_callable = nullptr;
			}
			else
			{
				m_func = rhs.m_func;
				rhs.m_func = nullptr;
			}
			rhs.m_type = Type::empty;
		}
		Function(function_t* func) :
			m_type(Type::function),
			m_func(func) {}
		template <typename _Ty>
		Function(_Ty&& value) :
			m_type(Type::object),
			m_callable(memnew<Callable<remove_cv_t<remove_reference_t<_Ty>>>>(forward<_Ty>(value))) {}
		~Function()
		{
			internal_clear();
		}
		Function& operator=(const Function& rhs)
		{
			internal_clear();
			m_type = rhs.m_type;
			if (m_type == Type::object) m_callable = rhs.m_callable->clone();
			else m_func = rhs.m_func;
			return *this;
		}
		Function& operator=(Function&& rhs)
		{
			internal_clear();
			m_type = rhs.m_type;
			if (m_type == Type::object)
			{
				m_callable = rhs.m_callable;
				rhs.m_callable = nullptr;
			}
			else
			{
				m_func = rhs.m_func;
				rhs.m_func = nullptr;
			}
			rhs.m_type = Type::empty;
			return *this;
		}
		Function& operator=(function_t* func)
		{
			internal_clear();
			m_type = Type::function;
			m_func = func;
			return *this;
		}
		template <typename _Ty>
		Function& operator=(_Ty&& value)
		{
			internal_clear();
			m_type = Type::object;
			m_callable = memnew<Callable<remove_cv_t<remove_reference_t<_Ty>>>>(forward<_Ty>(value));
			return *this;
		}
		void swap(Function& rhs)
		{
			auto type = m_type;
			void* data;
			if (type == Type::function) data = m_func;
			else if (type == Type::object) data = m_callable;
			else data = nullptr;

			m_type = rhs.m_type;
			if (m_type == Type::function) m_func = rhs.m_func;
			else if (m_type == Type::object) m_callable = rhs.m_callable;
			else m_func = nullptr;

			rhs.m_type = type;
			if (rhs.m_type == Type::function) rhs.m_func = (function_t*)data;
			else if (rhs.m_type == Type::object) rhs.m_callable = (ICallable*)data;
			else rhs.m_func = nullptr;
		}
		operator bool() const
		{
			return m_type != Type::empty;
		}
		_R operator()(_Args... args) const
		{
			lucheck_msg(m_type != Type::empty, "Try to invoke one empty Function.");
			if (m_type == Type::function) return m_func(forward<_Args>(args)...);
			else return m_callable->invoke(forward<_Args>(args)...);
		}
	};
}
