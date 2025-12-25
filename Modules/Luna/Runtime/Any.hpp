/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Any.hpp
* @author JXMaster
* @date 2024/11/14
 */
#pragma once
#include "Reflection.hpp"

namespace Luna
{
    //! @addtogroup Runtime
    //! @{

    //! A instance wrapper than can contain one instance of any registered type.
    class Any
    {   
    public:
        //! Constructs one empty instance.
        constexpr Any() :
            m_type(nullptr),
            m_data(nullptr) {}

        //! Constructs one instance by copying value from `rhs`.
        //! @param[in] rhs The instance to be copied.
        Any(const Any& rhs) :
            m_type(rhs.m_type)
        {
            if(rhs.m_data)
            {
                m_data = memalloc(get_type_size(rhs.m_type), get_type_alignment(rhs.m_type));
                copy_construct_type(rhs.m_type, m_data, rhs.m_data);
            }
            else
            {
                m_data = nullptr;
            }
        }
        //! Constructs one instance by moving value from `rhs`.
        //! @param[in] rhs The instance to be moved.
        Any(Any&& rhs) :
            m_type(rhs.m_type),
            m_data(rhs.m_data)
        {
            if(rhs.m_data)
            {
                m_data = memalloc(get_type_size(rhs.m_type), get_type_alignment(rhs.m_type));
                move_construct_type(rhs.m_type, m_data, rhs.m_data);
            }
            else
            {
                m_data = nullptr;
            }
        }
        //! Constructs one instance by copying the value directly.
        //! @param[in] value The value to be copied.
        template <typename _Ty>
        Any(const _Ty& value) :
            m_type(typeof<_Ty>())
        {
            m_data = memalloc(get_type_size(m_type), get_type_alignment(m_type));
            copy_construct_type(m_type, m_data, &value);
        }
        //! Constructs one instance by moving the value directly.
        template <typename _Ty>
        Any(_Ty&& value) :
            m_type(typeof<_Ty>())
        {
            m_data = memalloc(get_type_size(m_type), get_type_alignment(m_type));
            move_construct_type(m_type, m_data, &value);
        }
        //! Assigns one instance by copying from another instance.
        //! @param[in] rhs The instance to be copied from.
        //! @return Returns `*this`.
        Any& operator=(const Any& rhs)
        {
            internal_free();
            m_type = rhs.m_type;
            if(rhs.m_data)
            {
                m_data = memalloc(get_type_size(rhs.m_type), get_type_alignment(rhs.m_type));
                copy_construct_type(rhs.m_type, m_data, rhs.m_data);
            }
            return *this;
        }
        //! Assigns one instance by moving from another instance.
        //! @param[in] rhs The instance to be moved from.
        //! @return Returns `*this`.
        Any& operator=(Any&& rhs)
        {
            internal_free();
            m_type = rhs.m_type;
            if(rhs.m_data)
            {
                m_data = memalloc(get_type_size(rhs.m_type), get_type_alignment(rhs.m_type));
                move_construct_type(rhs.m_type, m_data, rhs.m_data);
            }
            return *this;
        }
        //! Assigns one instance by copying one value directly.
        //! @param[in] value The value to be copied from.
        //! @return Returns `*this`.
        template <typename _Ty>
        Any& operator=(const _Ty& value)
        {
            internal_free();
            m_type = typeof<decay_t<_Ty>>();
            m_data = memalloc(get_type_size(m_type), get_type_alignment(m_type));
            copy_construct_type(m_type, m_data, &value);
            return *this;
        }
        //! Assigns one instance by moving one value directly.
        //! @param[in] value The value to be moved from.
        //! @return Returns `*this`.
        template <typename _Ty>
        Any& operator=(_Ty&& value)
        {
            internal_free();
            m_type = typeof<decay_t<_Ty>>();
            m_data = memalloc(get_type_size(m_type), get_type_alignment(m_type));
            move_construct_type(m_type, m_data, &value);
            return *this;
        }
        ~Any()
        {
            internal_free();
        }
        //! Constructs one value by providing the type directly.
        //! @param[in] type The type to construct new value.
        void construct(typeinfo_t type)
        {
            internal_free();
            m_type = type;
            m_data = memalloc(get_type_size(m_type), get_type_alignment(m_type));
            construct_type(m_type, m_data);
        }

        //! Copy-constructs one value by providing the type and data directly.
        //! @param[in] type The type to construct new value.
        //! @param[in] data The data to be copied.
        void copy_construct(typeinfo_t type, const void* data)
        {
            internal_free();
            m_type = type;
            m_data = memalloc(get_type_size(m_type), get_type_alignment(m_type));
            copy_construct_type(m_type, m_data, data);
        }

        //! Move-constructs one value by providing the type and data directly.
        //! @param[in] type The type to construct new value.
        //! @param[in] data The data to be moved.
        void move_construct(typeinfo_t type, void* data)
        {
            internal_free();
            m_type = type;
            m_data = memalloc(get_type_size(m_type), get_type_alignment(m_type));
            move_construct_type(m_type, m_data, data);
        }

        //! Constructs the value directly from arguments passed in.
        //! @param[in] args The arguments used to construct value.
        template <typename _Ty, typename... _Args>
        decay_t<_Ty>& emplace(_Args&&... args)
        {
            internal_free();
            m_type = typeof<_Ty>();
            _Ty* data = memnew<_Ty>(forward<_Args>(args)...);
            m_data = data;
            return *data;
        }
        //! Destructs the contained value and resets the instance to empty.
        void reset()
        {
            internal_free();
            m_type = nullptr;
        }
        //! Swaps values of the current and incoming instance.
        //! @param[in] rhs The instance to swap with.
        void swap(Any& rhs)
        {
            Luna::swap(*this, rhs);
        }
        //! Checks whether this instance contains a value.
        //! @return Returns `true` if this instance contains a value. Returns `false` otherwise.
        bool has_value() const
        {
            return m_data != nullptr;
        }
        //! Returns the type object of the contained value.
        //! @return Returns the type object of the contained value. Returns `nullptr` if this instance is empty.
        typeinfo_t type() const
        {
            return m_type;
        }
        //! Checks whether the contained value can be converted to the specified type.
        //! @return Returns `true` if the instance is not empty and the value can be converted to the specified type.
        //! Returns `false` otherwise.
        template <typename _Ty>
        bool is_type() const
        {
            if(!m_data) return false;
            typeinfo_t obj_type = m_type;
            while(obj_type)
            {
                if(typeof<_Ty>() == obj_type) return true;
                obj_type = get_base_type(obj_type);
            }
            return false;
        }
        //! Gets an untyped pointer to the contained value.
        //! @return Returns a pointer to the contained value. Returns `nullptr` if the instance is empty.
        void* data()
        {
            return m_data;
        }

        //! Gets an untyped pointer to the contained value.
        //! @return Returns a pointer to the contained value. Returns `nullptr` if the instance is empty.
        const void* data() const
        {
            return m_data;
        }

        //! Gets a typed pointer to the contained value as type `_Ty`.
        //! @return Returns a typed pointer to the contained value as type `_Ty`. Returns `nullptr` if the instance is empty 
        //! or if the value cannot be converted to type `_Ty`.
        template <typename _Ty>
        _Ty* as()
        {
            return is_type<_Ty>() ? (_Ty*)m_data : nullptr;
        }

        //! Gets a typed pointer to the contained value as type `_Ty`.
        //! @return Returns a typed pointer to the contained value as type `_Ty`. Returns `nullptr` if the instance is empty 
        //! or if the value cannot be converted to type `_Ty`.
        template <typename _Ty>
        const _Ty* as() const
        {
            return is_type<_Ty>() ? (const _Ty*)m_data : nullptr;
        }

    private:
        // ---------------------------------------- Begin of ABI compatible part ----------------------------------------
        typeinfo_t m_type;
        void* m_data;
        // ----------------------------------------  End of ABI compatible part  ----------------------------------------
        void internal_free()
        {
            if(m_data)
            {
                destruct_type(m_type, m_data);
                memfree(m_data, get_type_alignment(m_type));
                m_data = nullptr;
            }
        }
    };

    //! Gets the type object of @ref Any.
    //! @return Returns the type object of @ref Any.
    LUNA_RUNTIME_API typeinfo_t any_type();
    template <> struct typeof_t<Any>
    {
        typeinfo_t operator()() const { return any_type(); }
    };

    //! @}
}