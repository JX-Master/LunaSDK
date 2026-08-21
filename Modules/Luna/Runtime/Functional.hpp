/*!
* This file is a portion of LunaSDK.
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
    //! @addtogroup Runtime
    //! @{
    
    //! Function object for performing comparisons. Unless specialised, invokes `operator==` on type T.
    template <typename _Ty>
    struct equal_to
    {
        constexpr bool operator()(const _Ty& lhs, const _Ty& rhs) const
        {
            return lhs == rhs;
        }
    };

    //! Function object for performing comparisons. Unless specialized, invokes `operator<` on type T.
    template <typename _Ty>
    struct less
    {
        constexpr bool operator()(const _Ty& lhs, const _Ty& rhs) const
        {
            return lhs < rhs;
        }
    };

    //! Function object that hashes the specified type into a `usize` 
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

    template <> struct hash<typeinfo_t>
    {
        usize operator()(typeinfo_t rhs) const { return (usize)rhs.handle; }
    };

    //! Wraps one reference to one copyable, assignable object.
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

    //! Invokes the specified callable object.
    //! @param[in] f The callable object to invoke.
    //! @return Returns the return value of the callable object.
    template <typename _Func>
    auto invoke(_Func&& f) -> decltype(static_cast<_Func&&>(f)())
    {
        return static_cast<_Func&&>(f)();
    }

    //! Invokes the specified callable object.
    //! @param[in] f The callable object to invoke.
    //! @param[in] arg1 The first argument passed to the callable object.
    //! @param[in] args The rest arguments passed to the callable object.
    //! @return Returns the return value of the callable object.
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

    //! Invokes the specified callable object.
    //! @param[in] f The callable object to invoke.
    //! @param[in] arg1 The first argument passed to the callable object.
    //! @param[in] args The rest arguments passed to the callable object.
    //! @return Returns the return value of the callable object, implicitly converted to `_Return`, if `_Return` is not void. None otherwise.
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

    //! A function wrapper that can store one callable object, and enable coping, moving and invoking of such callable object.
    //! @details The callable object can be a function pointer or a function object (types that overloads `operator()`).
    //!
    //! The function wrapper stores one inline buffer with the size of `4 * sizeof(void*)` bytes. If the size and alignment
    //! requirement of the callable object is no greater than the size and alignment requirement of the inline buffer, the
    //! callable object is stored in the inline buffer directly without allocating heap memory. Otherwise, the callable
    //! object is allocated on heap, and the function wrapper stores one pointer to such object. Function pointers are also
    //! stored in the inline buffer directly, without allocating heap memory.
    template <typename _R, typename... _Args>
    struct Function<_R(_Args...)>
    {
    private:
        //! The operation kind that can be performed by one manager function.
        enum class Op : u8
        {
            //! Copy-constructs the object in the source buffer to the destination buffer.
            copy_construct = 0,
            //! Move-constructs the object in the source buffer to the destination buffer, then destroys
            //! the object in the source buffer.
            move_construct = 1,
            //! Destroys the object in the destination buffer.
            destroy = 2,
        };
        //! The function that manages the lifecycle of the object stored in the buffer of the function wrapper.
        using manager_t = void(*)(void* dst, const void* src, Op op);
        //! The function that invokes the object stored in the buffer of the function wrapper.
        using invoke_t = _R(*)(void* obj, _Args... args);
        using function_t = _R(_Args...);
        //! The size of the inline buffer, in bytes. The buffer stores at most 4 pointers.
        static constexpr usize buffer_size = 4 * sizeof(void*);
        //! Tests whether the specified callable type can be stored in the inline buffer directly.
        template <typename _Ty>
        static constexpr bool is_small_v = (sizeof(_Ty) <= buffer_size) && (alignof(_Ty) <= MAX_ALIGN);
        //! The inline buffer that stores the callable object. If the callable object is stored on heap, the buffer stores
        //! the pointer to the object. If the function wrapper stores one function pointer, the buffer stores the function pointer.
        alignas(MAX_ALIGN) u8 m_buffer[buffer_size];
        //! The invoke function used to invoke the object stored in `m_buffer`. This is `nullptr` if the function wrapper is empty.
        invoke_t m_invoke;
        //! The manager function used to copy, move and destroy the object stored in `m_buffer`. This is `nullptr` if the
        //! function wrapper is empty.
        manager_t m_manager;
        //! Invokes the function pointer stored in the buffer.
        static _R function_pointer_invoke(void* obj, _Args... args)
        {
            return (*(function_t**)obj)(forward<_Args>(args)...);
        }
        //! Manages the lifecycle of the function pointer stored in the buffer. Function pointers are trivially copyable, so
        //! this function only performs bitwise copy for copy and move operations, and performs no operation for destruction.
        static void function_pointer_manage(void* dst, const void* src, Op op)
        {
            if (op != Op::destroy)
            {
                memcpy(dst, src, sizeof(function_t*));
            }
        }
        //! Invokes the callable object stored in the inline buffer.
        template <typename _Ty>
        static _R small_callable_invoke(void* obj, _Args... args)
        {
            return (*(_Ty*)obj)(forward<_Args>(args)...);
        }
        //! Manages the lifecycle of the callable object stored in the inline buffer.
        template <typename _Ty>
        static void small_callable_manage(void* dst, const void* src, Op op)
        {
            if constexpr (is_trivially_copyable_v<_Ty>)
            {
                // Trivially copyable objects can be copied and moved by bitwise copy, and no destruction operation
                // needs to be performed.
                if (op != Op::destroy)
                {
                    memcpy(dst, src, sizeof(_Ty));
                }
            }
            else
            {
                switch (op)
                {
                case Op::copy_construct:
                    new (dst) _Ty(*(const _Ty*)src);
                    break;
                case Op::move_construct:
                    new (dst) _Ty(move(*(_Ty*)src));
                    ((_Ty*)src)->~_Ty();
                    break;
                case Op::destroy:
                    ((_Ty*)dst)->~_Ty();
                    break;
                }
            }
        }
        //! Invokes the callable object stored on heap.
        template <typename _Ty>
        static _R big_callable_invoke(void* obj, _Args... args)
        {
            return (**(_Ty**)obj)(forward<_Args>(args)...);
        }
        //! Manages the lifecycle of the callable object stored on heap.
        template <typename _Ty>
        static void big_callable_manage(void* dst, const void* src, Op op)
        {
            _Ty*& dst_ptr = *(_Ty**)dst;
            _Ty* const& src_ptr = *(_Ty* const*)src;
            switch (op)
            {
            case Op::copy_construct:
                dst_ptr = memnew<_Ty>(*src_ptr);
                break;
            case Op::move_construct:
                dst_ptr = src_ptr;
                break;
            case Op::destroy:
                if (dst_ptr)
                {
                    memdelete(dst_ptr);
                    dst_ptr = nullptr;
                }
                break;
            }
        }
        //! Destroys the object stored in the function wrapper and resets the function wrapper to empty state.
        void internal_clear()
        {
            if (m_manager)
            {
                m_manager(m_buffer, m_buffer, Op::destroy);
            }
            m_invoke = nullptr;
            m_manager = nullptr;
        }
        //! Assigns one callable object to this function wrapper. The function wrapper must be empty when calling this function.
        template <typename _Ty>
        void assign_callable(_Ty&& value)
        {
            using _Decay = remove_cv_t<remove_reference_t<_Ty>>;
            if constexpr (is_same_v<_Decay, function_t>)
            {
                // Stores one function (not function pointer) as one function pointer. This branch exists for the case where
                // the template constructor or assignment operator is selected when the user passes one function (not function
                // pointer) to this function wrapper.
                *(function_t**)m_buffer = value;
                m_invoke = &function_pointer_invoke;
                m_manager = &function_pointer_manage;
            }
            else if constexpr (is_small_v<_Decay>)
            {
                new (m_buffer) _Decay(forward<_Ty>(value));
                m_invoke = &small_callable_invoke<_Decay>;
                m_manager = &small_callable_manage<_Decay>;
            }
            else
            {
                *(_Decay**)m_buffer = memnew<_Decay>(forward<_Ty>(value));
                m_invoke = &big_callable_invoke<_Decay>;
                m_manager = &big_callable_manage<_Decay>;
            }
        }
    public:
        using result_type = _R;
        //! Constructs an empty function wrapper.
        Function() :
            m_invoke(nullptr),
            m_manager(nullptr) {}
        //! Constructs an empty function wrapper with `nullptr`.
        Function(nullptr_t) :
            m_invoke(nullptr),
            m_manager(nullptr) {}
        //! Constructs an function wrapper by coping from another function object.
        //! @param[in] rhs The function object to copy from.
        Function(const Function& rhs) :
            m_invoke(rhs.m_invoke),
            m_manager(rhs.m_manager)
        {
            if (m_manager)
            {
                m_manager(m_buffer, rhs.m_buffer, Op::copy_construct);
            }
        }
        //! Constructs an function wrapper by moving from another function object.
        //! @param[in] rhs The function object to move from. The function wrapper `rhs` is empty after this call.
        Function(Function&& rhs) :
            m_invoke(rhs.m_invoke),
            m_manager(rhs.m_manager)
        {
            if (m_manager)
            {
                m_manager(m_buffer, rhs.m_buffer, Op::move_construct);
            }
            rhs.m_invoke = nullptr;
            rhs.m_manager = nullptr;
        }
        //! Constructs an function wrapper using one function pointer.
        //! @param[in] func The function pointer to assign.
        //! @remark If `func` is `nullptr`, the function wrapper is empty.
        Function(function_t* func) :
            m_invoke(nullptr),
            m_manager(nullptr)
        {
            if (func)
            {
                *(function_t**)m_buffer = func;
                m_invoke = &function_pointer_invoke;
                m_manager = &function_pointer_manage;
            }
        }
        //! Constructs an function wrapper using one function object.
        //! @param[in] value The function object to assign. The function object will be copy-constructed or move-constructed
        //! into the function wrapper.
        //! @remark If the size and alignment requirement of the function object is no greater than the size and alignment
        //! requirement of the inline buffer of the function wrapper, the function object will be stored in the inline buffer
        //! directly without allocating heap memory, otherwise the function object will be allocated on heap.
        //! @remark This constructor is disabled if `value` is `Function` itself, so the copy constructor and move constructor
        //! are always used to copy or move one `Function`.
        template <typename _Ty, typename = enable_if_t<!is_same_v<remove_cv_t<remove_reference_t<_Ty>>, Function>>>
        Function(_Ty&& value) :
            m_invoke(nullptr),
            m_manager(nullptr)
        {
            assign_callable(forward<_Ty>(value));
        }
        ~Function()
        {
            internal_clear();
        }
        Function& operator=(const Function& rhs)
        {
            if (this == &rhs)
            {
                return *this;
            }
            internal_clear();
            m_invoke = rhs.m_invoke;
            m_manager = rhs.m_manager;
            if (m_manager)
            {
                m_manager(m_buffer, rhs.m_buffer, Op::copy_construct);
            }
            return *this;
        }
        Function& operator=(Function&& rhs)
        {
            if (this == &rhs)
            {
                return *this;
            }
            internal_clear();
            m_invoke = rhs.m_invoke;
            m_manager = rhs.m_manager;
            if (m_manager)
            {
                m_manager(m_buffer, rhs.m_buffer, Op::move_construct);
            }
            rhs.m_invoke = nullptr;
            rhs.m_manager = nullptr;
            return *this;
        }
        Function& operator=(function_t* func)
        {
            internal_clear();
            if (func)
            {
                *(function_t**)m_buffer = func;
                m_invoke = &function_pointer_invoke;
                m_manager = &function_pointer_manage;
            }
            return *this;
        }
        //! @remark This operator is disabled if `value` is `Function` itself, so the copy assignment operator and move
        //! assignment operator are always used to copy or move one `Function`.
        template <typename _Ty, typename = enable_if_t<!is_same_v<remove_cv_t<remove_reference_t<_Ty>>, Function>>>
        Function& operator=(_Ty&& value)
        {
            internal_clear();
            assign_callable(forward<_Ty>(value));
            return *this;
        }
        //! Swaps the data of this function wrapper with another function wrapper.
        //! @param[in] rhs The function wrapper to swap with.
        void swap(Function& rhs)
        {
            if (this == &rhs)
            {
                return;
            }
            if (!m_manager && !rhs.m_manager)
            {
                return;
            }
            if (!m_manager)
            {
                // `this` is empty, moves the object stored in `rhs` to `this`.
                m_invoke = rhs.m_invoke;
                m_manager = rhs.m_manager;
                m_manager(m_buffer, rhs.m_buffer, Op::move_construct);
                rhs.m_invoke = nullptr;
                rhs.m_manager = nullptr;
                return;
            }
            if (!rhs.m_manager)
            {
                // `rhs` is empty, moves the object stored in `this` to `rhs`.
                rhs.m_invoke = m_invoke;
                rhs.m_manager = m_manager;
                m_manager(rhs.m_buffer, m_buffer, Op::move_construct);
                m_invoke = nullptr;
                m_manager = nullptr;
                return;
            }
            // Both function wrappers are non-empty, moves the objects through one temporary buffer.
            alignas(MAX_ALIGN) u8 tmp[buffer_size];
            m_manager(tmp, m_buffer, Op::move_construct);
            rhs.m_manager(m_buffer, rhs.m_buffer, Op::move_construct);
            m_manager(rhs.m_buffer, tmp, Op::move_construct);
            auto invoke = m_invoke;
            m_invoke = rhs.m_invoke;
            rhs.m_invoke = invoke;
            auto manager = m_manager;
            m_manager = rhs.m_manager;
            rhs.m_manager = manager;
        }
        //! Tests whether this function wrapper is valid.
        //! @return Return `true` if this function wrapper is valid, that is, contains one callable object. 
        //! Return `false` otherwise.
        bool valid() const
        {
            return m_invoke != nullptr;
        }
        //! Tests whether this function wrapper is valid.
        //! @return Return `true` if this function wrapper is valid, that is, contains one callable object. 
        //! Return `false` otherwise.
        operator bool() const
        {
            return m_invoke != nullptr;
        }
        //! Clears the function wrapper. The function wrapper contains no callable object after this operation.
        void reset()
        {
            internal_clear();
        }
        //! Invokes the function wrapper. This will invoke the callable object that is stored in the function.
        //! @param[in] args The arguments passed to the callable object.
        //! @return Returns the return value of the callable object if `_R` is not `void`. Returns nothing otherwise.
        _R operator()(_Args... args) const
        {
            lucheck_msg(m_invoke, "Try to invoke one empty Function.");
            return m_invoke((void*)m_buffer, forward<_Args>(args)...);
        }
    };

    //! @}
}
