/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ErrorTest.cpp
* @author JXMaster
* @date 2020/2/23
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/Result.hpp>

namespace Luna
{
    constexpr u32 TEST_ERROR_DOMAIN = 0x81234567;
    constexpr u16 TEST_ERROR_CATEGORY_ID = 0x89AB;
    constexpr errcat_t TEST_ERROR_CATEGORY = make_error_category(TEST_ERROR_DOMAIN, TEST_ERROR_CATEGORY_ID);
    constexpr ResultCode E_TEST_FAILURE = make_error_code(TEST_ERROR_DOMAIN, TEST_ERROR_CATEGORY_ID, -1);
    constexpr ResultCode E_TEST_MIN_FAILURE = make_error_code(TEST_ERROR_DOMAIN, TEST_ERROR_CATEGORY_ID, -32768);
    constexpr ResultCode S_TEST_ZERO_STATUS = make_error_code(TEST_ERROR_DOMAIN, TEST_ERROR_CATEGORY_ID, 0);
    constexpr ResultCode S_TEST_POSITIVE_STATUS = make_error_code(TEST_ERROR_DOMAIN, TEST_ERROR_CATEGORY_ID, 1);
    constexpr ResultCode S_TEST_MAX_STATUS = make_error_code(TEST_ERROR_DOMAIN, TEST_ERROR_CATEGORY_ID, 32767);

    static_assert(E_TEST_FAILURE.code == 0x8123456789ABFFFF);
    static_assert(sizeof(ResultCode) == sizeof(u64));
    static_assert(is_registered_error_domain(0x7FFFFFFF));
    static_assert(!is_registered_error_domain(TEST_ERROR_DOMAIN));
    static_assert(is_self_allocated_error_domain(TEST_ERROR_DOMAIN));
    static_assert(get_error_code_domain(E_TEST_FAILURE) == TEST_ERROR_DOMAIN);
    static_assert(get_error_code_category_id(E_TEST_FAILURE) == TEST_ERROR_CATEGORY_ID);
    static_assert(get_error_code_result(E_TEST_FAILURE) == -1);
    static_assert(get_error_code_result(E_TEST_MIN_FAILURE) == -32768);
    static_assert(failed(E_TEST_FAILURE));
    static_assert(failed(E_TEST_MIN_FAILURE));
    static_assert(succeeded(ResultCode()));
    static_assert(succeeded(S_TEST_ZERO_STATUS));
    static_assert(succeeded(S_TEST_POSITIVE_STATUS));
    static_assert(get_error_code_result(S_TEST_MAX_STATUS) == 32767);
    static_assert(succeeded(S_TEST_MAX_STATUS));
    static_assert(is_plain_success(ResultCode()));
    static_assert(!is_plain_success(S_TEST_ZERO_STATUS));
    static_assert(is_informative_success(S_TEST_ZERO_STATUS));
    static_assert(is_informative_success(S_TEST_POSITIVE_STATUS));
    static_assert(is_informative_success(make_error_code(TEST_ERROR_DOMAIN, 0, 0)));
    static_assert(is_informative_success(make_error_code(0, 1, 0)));
    static_assert(get_error_code_category(E_TEST_FAILURE) == TEST_ERROR_CATEGORY);
    static_assert(ok.valid());
    static_assert(is_plain_success(ok.errcode()));

    void error_test()
    {
        lutest(!strcmp(get_error_category_name(ERROR_CATEGORY), "Runtime"));
        lutest(register_error_category(TEST_ERROR_CATEGORY, "TestError"));
        lutest(register_error_category(TEST_ERROR_CATEGORY, "TestError"));
        lutest(!register_error_category(TEST_ERROR_CATEGORY, "ConflictingTestError"));

        lutest(register_error_code(E_TEST_FAILURE, "failure", "Test failure."));
        lutest(register_error_code(S_TEST_ZERO_STATUS, "zero_status", "Successful status with a zero local result."));
        lutest(register_error_code(S_TEST_POSITIVE_STATUS, "positive_status", "Successful status with a positive local result."));
        lutest(register_error_code(E_TEST_FAILURE, "failure", "Test failure."));
        lutest(!register_error_code(E_TEST_FAILURE, "renamed_failure", "Test failure."));

        lutest(!strcmp(get_error_category_name(TEST_ERROR_CATEGORY), "TestError"));
        lutest(!strcmp(get_error_code_name(E_TEST_FAILURE), "failure"));
        lutest(!strcmp(get_error_code_description(E_TEST_FAILURE), "Test failure."));
        lutest(!strcmp(explain(E_TEST_FAILURE), "Test failure."));

        auto codes = get_all_error_codes_of_category(TEST_ERROR_CATEGORY);
        lutest(codes.size() == 3);
        HashSet<ResultCode> code_set;
        for(ResultCode code : codes) code_set.insert(code);
        lutest(code_set.contains(E_TEST_FAILURE));
        lutest(code_set.contains(S_TEST_ZERO_STATUS));
        lutest(code_set.contains(S_TEST_POSITIVE_STATUS));
        RV plain_success;
        RV informative_success(S_TEST_ZERO_STATUS);
        R<i32> value_with_status(42, S_TEST_POSITIVE_STATUS);
        R<i32> failure(E_TEST_FAILURE);
        lutest(plain_success.valid());
        lutest(informative_success.valid());
        lutest(is_informative_success(informative_success.errcode()));
        lutest(value_with_status.valid());
        lutest(value_with_status.get() == 42);
        lutest(value_with_status.errcode() == S_TEST_POSITIVE_STATUS);
        lutest(!failure.valid());
        lutest(failure.errcode() == E_TEST_FAILURE);
    }
}
