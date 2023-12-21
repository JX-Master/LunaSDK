/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Result.hpp
* @author JXMaster
* @date 2020/6/29
 */
#pragma once
#include "MemoryUtils.hpp"
#include "Assert.hpp"
#include "Error.hpp"

#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif

namespace Luna
{
	//! @addtogroup RuntimeError
    //! @{

	//! A wrapper object for the return value of one function that may fail.
	//! @details If the function succeeds, this object contains the return value; 
	//! if the function fails, this object contains the error code so that it can be identified.
	template <typename _Ty>
	struct [[nodiscard]] R
	{
	private:
		ErrCode m_err_code;
		Unconstructed<_Ty> m_value;

		R() = default;

	public:

		//! Tests if the result is successful and the return value is valid.
		//! @return Returns `true` if the call is successful. Returns `false` otherwise.
		bool valid() const
		{
			return m_err_code == ErrCode(0);
		}
		//! Constructs one successful result object with the specified return value.
		//! @param[in] v The return value.
		R(const _Ty& v) :
			m_err_code(0)
		{
			m_value.construct(v);
		}
		//! Constructs one successful result object with the specified return value.
		//! @param[in] v The return value.
		R(_Ty&& v) :
			m_err_code(0)
		{
			m_value.construct(move(v));
		}
		//! Constructs one failed result object with the specified error code.
		//! @param[in] error The error code to set.
		R(ErrCode error) :
			m_err_code(error)
		{
			luassert(m_err_code.code);
		}
		//! Constructs one result object by coping from another result object.
		//! @details The return value will be copy-constructed if valid.
		//! @param[in] rhs The object to copy from.
		R(const R& rhs) :
			m_err_code(rhs.m_err_code)
		{
			if (valid())
			{
				m_value.construct(rhs.m_value.get());
			}
		}
		//! Constructs one result object by moving from another result object.
		//! @details The return value will be move-constructed if valid.
		//! @param[in] rhs The object to move from.
		R(R&& rhs) :
			m_err_code(rhs.m_err_code)
		{
			if (valid())
			{
				m_value.construct(move(rhs.m_value.get()));
			}
		}
		//! Assigns the result object by coping from another result object.
		//! @details The return value will be copy-assigned if valid.
		//! @param[in] rhs The object to copy from.
		//! @return Returns `*this`.
		R& operator=(const R& rhs)
		{
			if (m_err_code.code && !rhs.m_err_code.code)	// this is failure and rhs is success.
			{
				m_value.construct(rhs.m_value.get());
			}
			else if (!m_err_code.code && !rhs.m_err_code.code)	// this is success and rhs is success.
			{
				m_value.get() = rhs.m_value.get();
			}
			else if (!m_err_code.code && rhs.m_err_code.code) // this is success and rhs is failure.
			{
				m_value.destruct();
			}
			m_err_code = rhs.m_err_code;
			return *this;
		}
		//! Assigns the result object by moving from another result object.
		//! @details The return value will be move-assigned if valid.
		//! @param[in] rhs The object to move from.
		//! @return Returns `*this`.
		R& operator=(R&& rhs)
		{
			if (m_err_code.code && !rhs.m_err_code.code)	// this is failure and rhs is success.
			{
				m_value.construct(move(rhs.m_value.get()));
			}
			else if (!m_err_code.code && !rhs.m_err_code.code)	// this is success and rhs is success.
			{
				m_value.get() = move(rhs.m_value.get());
			}
			else if (!m_err_code.code && rhs.m_err_code.code) // this is success and rhs is failure.
			{
				m_value.destruct();
			}
			m_err_code = rhs.m_err_code;
			return *this;
		}

		~R()
		{
			if (!m_err_code.code)
			{
				m_value.destruct();
			}
		}

		//! Gets the return value of the result object.
		//! @return Returns one reference of the containing return value.
		//! @par Valid Usage
		//! * `valid()` must be `true` when calling this function.
		const _Ty& get() const
		{
			luassert(valid());
			return m_value.get();
		}
		//! Gets the return value of the result object.
		//! @return Returns one reference of the containing return value.
		//! @par Valid Usage
		//! * `valid()` must be `true` when calling this function.
		_Ty& get()
		{
			luassert(valid());
			return m_value.get();
		}
		//! Gets the error code of the result object.
		//! @return Returns the error code of the result object. Returns `ErrCode(0)` if
		//! the result is successful (no error is occured).
		ErrCode errcode() const
		{
			return m_err_code;
		}
	};

	//! Specification of @ref R for void type.
	template <>
	struct [[nodiscard]] R<void>
	{
	private:
		ErrCode m_err_code;

	public:

		//! Tests if the result is successful.
		//! @return Returns `true` if the call is successful. Returns `false` otherwise.
		bool valid() const
		{
			return m_err_code == ErrCode(0);
		}
		//! Constructs one successful result object.
		constexpr R() : m_err_code(0) {}
		//! Constructs one failed result object with the specified error code.
		//! @param[in] error The error code to set.
		R(ErrCode error) :
			m_err_code(error)
		{
			luassert(m_err_code.code);
		}
		//! Constructs one result object by coping from another result object.
		//! @param[in] rhs The object to copy from.
		R(const R& rhs) :
			m_err_code(rhs.m_err_code) {}
		//! Assigns the result object by coping from another result object.
		//! @param[in] rhs The object to copy from.
		//! @return Returns `*this`.
		R& operator=(const R& rhs)
		{
			m_err_code = rhs.m_err_code;
			return *this;
		}
		//! Gets the error code of the result object.
		//! @return Returns the error code of the result object. Returns `ErrCode(0)` if
		//! the result object is valid (no error is occured).
		ErrCode errcode() const
		{
			return m_err_code;
		}
	};

	//! An alias of `R<void>` for representing one throwable function with no return value.
	using RV = R<void>;

	//! Tests whether the specified result is successful.
	//! @param[in] r The result to test.
	//! @return Returns `true` if the result is successful. Returns `false` otherwise.
	template <typename _Ty>
	bool succeeded(const R<_Ty>& r)
	{
		return r.valid();
	}

	//! Tests whether the specified result is failed.
	//! @param[in] r The result to test.
	//! @return Returns `true` if the result is failed. Returns `false` otherwise.
	template <typename _Ty>
	bool failed(const R<_Ty>& r)
	{
		return !r.valid();
	}

	//! Unwraps the real error code from the result.
	//! @details If the error code of this result object is `BasicError::error_object`, this function returns the error code stored in 
	//! the error object of the current thread. If the error code of this object is not `BasicError::error_object`,
	//! this function returns the error code as is.
	//! @param[in] obj The original result object received from the called function.
	//! @return Returns the real error code from the result.
	template <typename _Ty>
	inline ErrCode unwrap_errcode(const R<_Ty>& obj)
	{
		return unwrap_errcode(obj.errcode());
	}

	//! A special constant result object that represents one successful result.
	//! You can return `ok` instead of `RV()` to clearly represent one successful call for one function without return value.
	constexpr RV ok;

	//! @}
}

//! @addtogroup RuntimeError
//! @{

//! Crashes the program if the specified result is failed.
//! @param[in] _res The result to test.
#define lupanic_if_failed(_res) {Luna::ErrCode _err = (_res).errcode(); if(_err.code != 0) Luna::assert_fail(Luna::explain(_err), luna_u8_string(__FILE__), (unsigned)(__LINE__)); }

//! Crashes the program if the specified result is failed with custom message.
//! @param[in] _res The result to test.
//! @param[in] _msg The message to display.
#define lupanic_if_failed_msg(_res, _msg) {Luna::ErrCode _err = (_res).errcode(); if(_err.code != 0) Luna::assert_fail(_msg, luna_u8_string(__FILE__), (unsigned)(__LINE__)); }

//! The error code used in `lucatch` block to identify the error.
//! @par Valid Usage
//! * This may only be used in one `lucatch` block.
#define luerr _try_res

//! Opens one try block that encapsulates expressions that may fail.
//! @par Valid Usage
//! * Every function body can have only one try-catch block that encapsulates all expressions that may fail. If you need multiple try-catch blocks,
//! you can use dedicated functions to wrap each try-catch block, then call such functions in your original function.
#define lutry ErrCode luerr = ErrCode(0);

//! Throws one error and jumps execution to the `lucatch` block.
//! @param[in] _r The error code to throw.
//! @par Valid Usage
//! * This may only be used in one `lutry` block.
#define luthrow(_r) { luerr = (_r); goto _try_err; }
//! Opens one catch block that handles errors thrown from try block.
//! @par Valid Usage
//! * This must be declared directly after the `lutry` block.
#define lucatch _try_err: if((luerr).code)
//! Defines one catch block that returns the error code (if any) thrown from try block.
//! @details This can be used if the error cannot be handled in this function.
//! * This must be declared directly after the `lutry` block.
#define lucatchret _try_err: if((luerr).code) { return luerr; }
//! Tests whether the specified expression returns one failed result, and throws the error code
//! if failed.
//! @param[in] _exp The expression to be evaluated.
#define luexp(_exp) { luerr = (_exp).errcode(); if((luerr).code) { goto _try_err; } }
//! Assigns the return value of the specified expression to the specified variable if the return value is valid,
//! and throws the error code if not.
//! @param[in] _v The variable to be assigned.
//! @param[in] _exp The expression to be evaluated.
#define luset(_v, _exp)  { auto _res = (_exp); if(!_res.valid()) { luerr = _res.errcode(); goto _try_err; } (_v) = move(_res.get()); }
//! Creates one local variable to hold the return value of the specified expression if the return value is valid,
//! and throws the error code if not.
//! @param[in] _v The name of the local variable to be created.
//! @param[in] _exp The expression to be evaluated.
#define lulet(_v, _exp) auto _r_##_v = (_exp); if(!(_r_##_v).valid()) { luerr = _r_##_v.errcode(); goto _try_err; } auto& _v = _r_##_v.get();

//@ }