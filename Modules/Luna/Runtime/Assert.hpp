/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Assert.hpp
* @author JXMaster
* @date 2018/10/26
 */
#pragma once
#include "Base.hpp"

#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif

//! @addtogroup Runtime
//! @{
//! @defgroup RuntimeAssert Assertions
//! @}

//! @addtogroup RuntimeAssert
//! @{

namespace Luna
{
	//! Reports an assertion failure information to the underlying OS or CRT.
	//! @details This function works in all builds, and can be called even if the runtime is not initialized.
	//! The behavior of this function depends on the OS/CRT implementation, but in general it will 
	//! present an error message box and then terminate the program.
	//! @param[in] msg The UTF-8 error message to show.
	//! @param[in] file The UTF-8 name of the file that causes the panic.
	//! @param[in] line The code line the assertion is placed.
	LUNA_RUNTIME_API void assert_fail(const c8* msg, const c8* file, u32 line);

	//! Triggers a debug break, pauses the program and attaches the debugger to the program.
	//! @details This only works in debug build.
	LUNA_RUNTIME_API void debug_break();
}

#define luna_u8_string_(s) u8 ## s
#define luna_u8_string(s) luna_u8_string_(s)

//! Evaluates the given expression, and if the result value cannot pass `if` statement, calls `assert_fail`.
//! This function works in all builds.
#define luassert_always(_condition) (void)(                                                   \
            (!!(_condition)) ||                                                               \
            (::Luna::assert_fail(u8###_condition, luna_u8_string(__FILE__), (unsigned)(__LINE__)), 0)       \
        )

//! Same as @ref luassert_always, but displays a custom message instead of the expression.
#define luassert_msg_always(_condition, _message) (void)(                                     \
            (!!(_condition)) ||                                                               \
            (::Luna::assert_fail(u8##_message, luna_u8_string(__FILE__), (unsigned)(__LINE__)), 0)         \
        )

//! Triggers an assertion failure directly.
#define lupanic_always() luassert_msg_always(false, "Panic has been called.")

//! Triggers an assertion failure with custom message.
#define lupanic_msg_always(_message) luassert_msg_always(false, _message)

#ifdef LUNA_DEBUG
//! Checks whether the specified condition evaluates to a non-zero value, and reports an assertion failure if not.
//! @details This only works in Debug compile mode.
//! @param[in] _condition The condition to evaluate.
#define luassert(_condition) luassert_always(_condition)
//! Reports an assertion failure immediately.
//! @details This only works in Debug compile mode.
#define lupanic() lupanic_always()
//! Checks whether the specified condition evaluates to a non-zero value, and reports an assertion failure with the user-defined message if not.
//! @details This only works in Debug compile mode.
//! @param[in] _condition The condition to evaluate.
//! @param[in] _message The message to output along with the assertion failure.
#define luassert_msg(_condition, _message) luassert_msg_always(_condition, _message)
//! Reports an assertion failure immediately.
//! @details This only works in Debug compile mode.
//! @param[in] _message The message to output along with the assertion failure.
#define lupanic_msg(_message) lupanic_msg_always(_message)
#else
#define luassert(_condition) ((void)0)
#define lupanic() ((void)0)
#define luassert_msg(_condition, _message) ((void)0)
#define lupanic_msg(_message) ((void)0)
#endif

#ifdef LUNA_ENABLE_CONTRACT_ASSERTION
//! Checks whether the specified condition evaluates to a non-zero value, and reports an assertion failure if not.
//! @details This only works when contract assertion option is enabled.
//! @param[in] _condition The condition to evaluate.
#define lucheck(_condition) luassert_always(_condition)
//! Checks whether the specified condition evaluates to a non-zero value, and reports an assertion failure with the user-defined message if not.
//! @details This only works when contract assertion option is enabled.
//! @param[in] _condition The condition to evaluate.
//! @param[in] _message The message to output along with the assertion failure.
#define lucheck_msg(_condition, _message) luassert_msg_always(_condition, _message)
#else
#define lucheck(_condition) ((void)0)
#define lucheck_msg(_condition, _message) ((void)0)
#endif

//! @}