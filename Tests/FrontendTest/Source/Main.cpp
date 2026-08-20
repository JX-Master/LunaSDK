/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/8/13
*/
#include <Luna/Frontend/Frontend.hpp>
#include <Luna/Runtime/Assert.hpp>
#include <Luna/Runtime/Memory.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>

using namespace Luna;
using namespace Luna::Frontend;

#define lutest luassert_always

namespace
{
    struct TrackedUserdata
    {
        usize* destructor_count;
    };

    void destroy_tracked_userdata(void* data)
    {
        TrackedUserdata* tracked = (TrackedUserdata*)data;
        ++(*tracked->destructor_count);
        memdelete(tracked);
    }

    struct CountingHandler
    {
        usize* copy_count;

        CountingHandler(usize* copy_count) : copy_count(copy_count) {}

        CountingHandler(const CountingHandler& rhs) : copy_count(rhs.copy_count)
        {
            ++(*copy_count);
        }

        CountingHandler(CountingHandler&& rhs) : copy_count(rhs.copy_count) {}

        R<Variant> operator()(IFrontend*, const Variant& params)
        {
            return params;
        }
    };

    void resource_data_test()
    {
        Ref<IFrontend> frontend = new_frontend();
        lutest(frontend->get_resource_type("/data") == ResourceType::null);

        lupanic_if_failed(frontend->set_resource_data("/data", Variant((i64)42)));
        lutest(frontend->get_resource_type("/data") == ResourceType::data);
        auto data = frontend->get_resource_data("/data");
        lutest(data.valid());
        lutest(data.get().type() == VariantType::number);
        lutest(data.get().number_type() == VariantNumberType::number_i64);
        lutest(data.get().inum() == 42);

        RV duplicate = frontend->set_resource_data("/data", Variant((i64)43));
        lutest(failed(duplicate));
        lutest(duplicate.errcode() == BasicError::already_exists());
        data = frontend->get_resource_data("/data");
        lutest(data.valid() && data.get().inum() == 42);

        lupanic_if_failed(frontend->set_resource_data("/data", Variant((i64)43), true));
        data = frontend->get_resource_data("/data");
        lutest(data.valid() && data.get().inum() == 43);

        auto missing = frontend->get_resource_data("/missing");
        lutest(!missing.valid());
        lutest(missing.errcode() == FrontendError::resource_not_found());

        RV empty_url = frontend->set_resource_data(Name(), Variant());
        lutest(failed(empty_url));
        lutest(empty_url.errcode() == BasicError::bad_arguments());

        lupanic_if_failed(frontend->remove_resource("/data"));
        lutest(frontend->get_resource_type("/data") == ResourceType::null);
        lupanic_if_failed(frontend->remove_resource("/data"));
        RV empty_remove = frontend->remove_resource(Name());
        lutest(failed(empty_remove));
        lutest(empty_remove.errcode() == BasicError::bad_arguments());
    }

    void resource_userdata_lifetime_test()
    {
        usize destructor_count = 0;
        Ref<IFrontend> frontend = new_frontend();

        TrackedUserdata* first = memnew<TrackedUserdata>(&destructor_count);
        lupanic_if_failed(frontend->set_resource_userdata(
            "/userdata", first, destroy_tracked_userdata));
        auto userdata = frontend->get_resource_userdata("/userdata");
        lutest(userdata.valid());
        lutest(userdata.get() == first);

        TrackedUserdata* rejected = memnew<TrackedUserdata>(&destructor_count);
        RV duplicate = frontend->set_resource_userdata(
            "/userdata", rejected, destroy_tracked_userdata);
        lutest(failed(duplicate));
        lutest(duplicate.errcode() == BasicError::already_exists());
        lutest(destructor_count == 0);
        destroy_tracked_userdata(rejected);
        lutest(destructor_count == 1);

        lupanic_if_failed(frontend->set_resource_data("/userdata", Variant(true), true));
        lutest(destructor_count == 2);
        auto wrong_type = frontend->get_resource_userdata("/userdata");
        lutest(!wrong_type.valid());
        lutest(wrong_type.errcode() == FrontendError::type_mismatch());

        TrackedUserdata* second = memnew<TrackedUserdata>(&destructor_count);
        lupanic_if_failed(frontend->set_resource_userdata(
            "/userdata", second, destroy_tracked_userdata, true));
        lupanic_if_failed(frontend->remove_resource("/userdata"));
        lutest(destructor_count == 3);

        TrackedUserdata* third = memnew<TrackedUserdata>(&destructor_count);
        lupanic_if_failed(frontend->set_resource_userdata(
            "/userdata", third, destroy_tracked_userdata));
        frontend.reset();
        lutest(destructor_count == 4);
    }

    void invocation_test()
    {
        Ref<IFrontend> frontend = new_frontend();
        usize handler_copy_count = 0;
        FunctionHandler echo_handler = CountingHandler(&handler_copy_count);
        lupanic_if_failed(frontend->set_resource_function("/echo", move(echo_handler)));
        usize copy_count_before_invoke = handler_copy_count;

        Variant params(VariantType::object);
        params["value"] = Variant((i64)7);
        auto result = frontend->invoke("/echo", params);
        lutest(result.valid());
        lutest(result.get() == params);
        lutest(handler_copy_count == copy_count_before_invoke);

        FunctionHandler empty_handler;
        RV empty_function = frontend->set_resource_function("/empty", move(empty_handler));
        lutest(failed(empty_function));
        lutest(empty_function.errcode() == BasicError::bad_arguments());

        RV empty_url = frontend->set_resource_function(
            Name(), FunctionHandler(CountingHandler(&handler_copy_count)));
        lutest(failed(empty_url));
        lutest(empty_url.errcode() == BasicError::bad_arguments());

        auto missing = frontend->invoke("/missing", Variant());
        lutest(!missing.valid());
        lutest(missing.errcode() == FrontendError::method_not_found());

        lupanic_if_failed(frontend->set_resource_data("/not-a-function", Variant()));
        auto not_a_function = frontend->invoke("/not-a-function", Variant());
        lutest(!not_a_function.valid());
        lutest(not_a_function.errcode() == FrontendError::method_not_found());

        lupanic_if_failed(frontend->set_resource_function(
            "/failure",
            FunctionHandler([](IFrontend*, const Variant&) -> R<Variant>
            {
                return BasicError::bad_data();
            })));
        auto failure = frontend->invoke("/failure", Variant());
        lutest(!failure.valid());
        lutest(failure.errcode() == BasicError::bad_data());
    }

    void nested_invocation_test()
    {
        Ref<IFrontend> frontend = new_frontend();
        lupanic_if_failed(frontend->set_resource_function(
            "/inner",
            FunctionHandler([](IFrontend*, const Variant& params) -> R<Variant>
            {
                return params;
            })));
        lupanic_if_failed(frontend->set_resource_function(
            "/outer",
            FunctionHandler([](IFrontend* frontend, const Variant& params) -> R<Variant>
            {
                return frontend->invoke("/inner", params);
            })));

        Variant params((i64)9);
        auto nested_result = frontend->invoke("/outer", params);
        lutest(nested_result.valid());
        lutest(nested_result.get().inum() == 9);

        lupanic_if_failed(frontend->set_resource_function(
            "/register-data",
            FunctionHandler([](IFrontend* frontend, const Variant& params) -> R<Variant>
            {
                RV result = frontend->set_resource_data("/created-in-handler", Variant(params));
                if(failed(result)) return result.errcode();
                return Variant();
            })));
        auto registration_result = frontend->invoke("/register-data", params);
        lutest(registration_result.valid());
        auto created_data = frontend->get_resource_data("/created-in-handler");
        lutest(created_data.valid());
        lutest(created_data.get().inum() == 9);
    }

    void self_modifying_handler_test()
    {
        Ref<IFrontend> frontend = new_frontend();
        lupanic_if_failed(frontend->set_resource_function(
            "/self-remove",
            FunctionHandler([](IFrontend* frontend, const Variant&) -> R<Variant>
            {
                RV result = frontend->remove_resource("/self-remove");
                if(failed(result)) return result.errcode();
                return Variant((i64)1);
            })));

        auto remove_result = frontend->invoke("/self-remove", Variant());
        lutest(remove_result.valid());
        lutest(remove_result.get().inum() == 1);
        lutest(frontend->get_resource_type("/self-remove") == ResourceType::null);

        lupanic_if_failed(frontend->set_resource_function(
            "/self-overwrite",
            FunctionHandler([](IFrontend* frontend, const Variant&) -> R<Variant>
            {
                RV result = frontend->set_resource_function(
                    "/self-overwrite",
                    FunctionHandler([](IFrontend*, const Variant&) -> R<Variant>
                    {
                        return Variant((i64)2);
                    }),
                    true);
                if(failed(result)) return result.errcode();
                return Variant((i64)1);
            })));

        auto first_result = frontend->invoke("/self-overwrite", Variant());
        lutest(first_result.valid());
        lutest(first_result.get().inum() == 1);
        auto second_result = frontend->invoke("/self-overwrite", Variant());
        lutest(second_result.valid());
        lutest(second_result.get().inum() == 2);
    }
}

int main()
{
    init();
    lupanic_if_failed(add_modules({module_frontend()}));
    lupanic_if_failed(init_modules());
    resource_data_test();
    resource_userdata_lifetime_test();
    invocation_test();
    nested_invocation_test();
    self_modifying_handler_test();
    close();
    return 0;
}
