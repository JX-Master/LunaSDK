/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Main.cpp
* @author JXMaster
* @date 2026/8/29
*/
#include "TestTypes.hpp"
#include <Luna/Asset/Asset.hpp>
#include <Luna/Runtime/Assert.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/PlatformDefines.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Serialization.hpp>
#include <Luna/Runtime/Signal.hpp>
#include <Luna/Runtime/String.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/Runtime/Variant.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/VariantUtils/VariantUtils.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/VFS/NativeFileSystem.hpp>
#include <cstdlib>
#include "AssetTest.meta.generated.hpp"

using namespace Luna;

namespace
{
    constexpr const c8* TEST_ASSET_TYPE = "AssetTest.Type";
    constexpr const c8* MAIN_LOADER = "AssetTest.MainLoader";
    constexpr const c8* AUXILIARY_LOADER = "AssetTest.AuxiliaryLoader";
    constexpr const c8* UNKNOWN_LOADER = "AssetTest.UnknownLoader";
    constexpr const c8* BLOCKING_LOADER = "AssetTest.BlockingLoader";
    constexpr const c8* EDITOR_UNIT = "Editor";
    constexpr const c8* STREAMING_UNIT = "Streaming";
    constexpr const c8* SHARED_UNIT_A = "SharedA";
    constexpr const c8* SHARED_UNIT_B = "SharedB";
    constexpr const c8* PARALLEL_UNIT_A = "ParallelA";
    constexpr const c8* PARALLEL_UNIT_B = "ParallelB";
    constexpr const c8* TEST_MOUNT_PATH = "/AssetTest";

    struct CallbackProbe
    {
        usize load_calls = 0;
        usize default_load_calls = 0;
        usize save_calls = 0;
        usize set_calls = 0;
        usize referred_assets_calls = 0;
        i32 next_data_value = 1;
        bool fail_next_load = false;
        Name last_load_unit;
        Name last_default_load_unit;
        Name last_save_unit;
        Name last_set_unit;
        Name last_referred_assets_unit;
        Path last_load_path;
        Path last_save_path;
        ObjRef last_saved_data;
        ObjRef last_set_data;
        ObjRef data_before_set;
        Asset::asset_t referred_asset;

        void reset_call_history()
        {
            load_calls = 0;
            default_load_calls = 0;
            save_calls = 0;
            set_calls = 0;
            referred_assets_calls = 0;
            fail_next_load = false;
            last_load_unit.reset();
            last_default_load_unit.reset();
            last_save_unit.reset();
            last_set_unit.reset();
            last_referred_assets_unit.reset();
            last_load_path.clear();
            last_save_path.clear();
            last_saved_data.reset();
            last_set_data.reset();
            data_before_set.reset();
            referred_asset.reset();
        }
    };

    CallbackProbe g_probe;

    struct BlockingLoadProbe
    {
        Ref<ISignal> unit_a_entered;
        Ref<ISignal> unit_b_entered;
        Ref<ISignal> release;
    };

    struct ConcurrentLoadContext
    {
        Asset::asset_t asset;
        Name data_unit;
        RV result;
    };

    BlockingLoadProbe g_blocking_probe;

    ObjRef new_test_data(i32 value)
    {
        Ref<AssetTestData> data = new_object<AssetTestData>(value);
        return ObjRef(data.object());
    }

    i32 get_test_data_value(const ObjRef& data)
    {
        luassert_always(data);
        AssetTestData* typed_data = cast_object<AssetTestData>(data.get());
        luassert_always(typed_data);
        return typed_data->value;
    }

    template <typename _Ty>
    void expect_error(const R<_Ty>& result, ResultCode expected_error)
    {
        luassert_always(failed(result));
        luassert_always(unwrap_errcode(result) == expected_error);
    }

    R<ObjRef> on_load_asset_data_unit(object_t userdata, Asset::asset_t asset,
        const Name& data_unit, const Path& path)
    {
        (void)userdata;
        (void)asset;
        ++g_probe.load_calls;
        g_probe.last_load_unit = data_unit;
        g_probe.last_load_path = path;
        if(g_probe.fail_next_load)
        {
            g_probe.fail_next_load = false;
            return E_BAD_DATA;
        }
        return new_test_data(g_probe.next_data_value++);
    }

    R<ObjRef> on_load_asset_data_unit_default_data(object_t userdata,
        Asset::asset_t asset, const Name& data_unit)
    {
        (void)userdata;
        (void)asset;
        ++g_probe.default_load_calls;
        g_probe.last_default_load_unit = data_unit;
        return new_test_data(g_probe.next_data_value++);
    }

    RV on_save_asset_data_unit(object_t userdata, Asset::asset_t asset,
        const Name& data_unit, const Path& path, object_t data)
    {
        (void)userdata;
        (void)asset;
        ++g_probe.save_calls;
        g_probe.last_save_unit = data_unit;
        g_probe.last_save_path = path;
        g_probe.last_saved_data = data;
        return ok;
    }

    RV on_set_asset_data_unit(object_t userdata, Asset::asset_t asset,
        const Name& data_unit, object_t data)
    {
        (void)userdata;
        ++g_probe.set_calls;
        g_probe.last_set_unit = data_unit;
        g_probe.last_set_data = data;
        auto old_data = Asset::get_asset_data_unit_object(asset, data_unit);
        if(failed(old_data)) return old_data.errcode();
        g_probe.data_before_set = old_data.get();
        return ok;
    }

    void on_get_referred_assets(object_t userdata, Asset::asset_t asset,
        const Name& data_unit, Vector<Asset::asset_t>& referred_assets)
    {
        (void)userdata;
        (void)asset;
        ++g_probe.referred_assets_calls;
        g_probe.last_referred_assets_unit = data_unit;
        if(g_probe.referred_asset)
        {
            referred_assets.push_back(g_probe.referred_asset);
        }
    }

    R<ObjRef> on_blocking_load_asset_data_unit(object_t userdata,
        Asset::asset_t asset, const Name& data_unit, const Path& path)
    {
        (void)userdata;
        (void)asset;
        (void)path;
        if(data_unit == PARALLEL_UNIT_A)
        {
            g_blocking_probe.unit_a_entered->trigger();
        }
        else if(data_unit == PARALLEL_UNIT_B)
        {
            g_blocking_probe.unit_b_entered->trigger();
        }
        else
        {
            return E_BAD_ARGUMENTS;
        }
        g_blocking_probe.release->wait();
        return new_test_data(data_unit == PARALLEL_UNIT_A ? 300 : 301);
    }

    void concurrent_load_thread(void* params)
    {
        ConcurrentLoadContext* context = (ConcurrentLoadContext*)params;
        context->result = Asset::load_asset_data_unit(context->asset, context->data_unit);
    }

    bool wait_for_signal(ISignal* signal)
    {
        for(usize i = 0; i < 2000; ++i)
        {
            if(signal->try_wait()) return true;
            sleep(1);
        }
        return false;
    }

    void register_test_asset_type()
    {
        Asset::AssetLoaderDesc main_loader;
        main_loader.name = MAIN_LOADER;
        main_loader.on_load_asset_data_unit = on_load_asset_data_unit;
        main_loader.on_load_asset_data_unit_default_data = on_load_asset_data_unit_default_data;
        main_loader.on_save_asset_data_unit = on_save_asset_data_unit;
        main_loader.on_set_asset_data_unit = on_set_asset_data_unit;
        main_loader.on_get_referred_assets = on_get_referred_assets;
        Asset::register_asset_loader(main_loader);

        Asset::AssetLoaderDesc auxiliary_loader = main_loader;
        auxiliary_loader.name = AUXILIARY_LOADER;
        Asset::register_asset_loader(auxiliary_loader);

        Asset::AssetLoaderDesc blocking_loader;
        blocking_loader.name = BLOCKING_LOADER;
        blocking_loader.on_load_asset_data_unit = on_blocking_load_asset_data_unit;
        Asset::register_asset_loader(blocking_loader);

        Asset::AssetTypeDesc type;
        type.name = TEST_ASSET_TYPE;
        type.main_data_unit_loader = MAIN_LOADER;
        Asset::register_asset_type(type);
    }

    Asset::asset_t new_test_asset(const c8* path, bool save_meta = false)
    {
        auto result = Asset::new_asset(Path(path), TEST_ASSET_TYPE, save_meta);
        lupanic_if_failed(result);
        return result.get();
    }

    bool contains_data_unit(const Vector<Asset::AssetDataUnitDesc>& data_units,
        const Name& id, const Name& loader)
    {
        for(const auto& data_unit : data_units)
        {
            if(data_unit.id == id && data_unit.loader == loader) return true;
        }
        return false;
    }

    ObjRef get_data_unit_object(Asset::asset_t asset, const Name& data_unit)
    {
        auto result = Asset::get_asset_data_unit_object(asset, data_unit);
        lupanic_if_failed(result);
        return result.get();
    }

    Asset::AssetDataUnitState get_data_unit_state(Asset::asset_t asset, const Name& data_unit)
    {
        auto result = Asset::get_asset_data_unit_state(asset, data_unit);
        lupanic_if_failed(result);
        return result.get();
    }

    void data_unit_lifecycle_test()
    {
        g_probe.reset_call_history();
        Asset::asset_t asset = new_test_asset("/AssetTest/Lifecycle.asset");

        Vector<Asset::AssetDataUnitDesc> data_units;
        lupanic_if_failed(Asset::get_asset_data_units(asset, data_units));
        luassert_always(data_units.size() == 1);
        luassert_always(contains_data_unit(data_units, Name(), MAIN_LOADER));

        Asset::AssetDataUnitDesc editor_unit;
        editor_unit.id = EDITOR_UNIT;
        editor_unit.loader = AUXILIARY_LOADER;
        lupanic_if_failed(Asset::add_asset_data_unit(asset, editor_unit));

        Asset::AssetDataUnitDesc streaming_unit;
        streaming_unit.id = STREAMING_UNIT;
        streaming_unit.loader = AUXILIARY_LOADER;
        lupanic_if_failed(Asset::add_asset_data_unit(asset, streaming_unit));

        data_units.clear();
        lupanic_if_failed(Asset::get_asset_data_units(asset, data_units));
        luassert_always(data_units.size() == 3);
        luassert_always(contains_data_unit(data_units, Name(), MAIN_LOADER));
        luassert_always(contains_data_unit(data_units, EDITOR_UNIT, AUXILIARY_LOADER));
        luassert_always(contains_data_unit(data_units, STREAMING_UNIT, AUXILIARY_LOADER));

        auto main_loader = Asset::get_asset_data_unit_loader(asset, Name());
        lupanic_if_failed(main_loader);
        luassert_always(main_loader.get() == MAIN_LOADER);
        auto editor_loader = Asset::get_asset_data_unit_loader(asset, EDITOR_UNIT);
        lupanic_if_failed(editor_loader);
        luassert_always(editor_loader.get() == AUXILIARY_LOADER);

        luassert_always(get_data_unit_state(asset, Name()) ==
            Asset::AssetDataUnitState::unloaded);
        luassert_always(get_data_unit_state(asset, EDITOR_UNIT) ==
            Asset::AssetDataUnitState::unloaded);
        expect_error(Asset::save_asset_data_unit(asset, Name()),
            Asset::E_ASSET_DATA_UNIT_NOT_LOADED);

        lupanic_if_failed(Asset::load_asset_data_unit(asset, Name()));
        luassert_always(g_probe.load_calls == 1);
        luassert_always(g_probe.last_load_unit.empty());
        luassert_always(g_probe.last_load_path == Path("/AssetTest/Lifecycle.asset"));
        luassert_always(get_data_unit_state(asset, Name()) ==
            Asset::AssetDataUnitState::loaded);
        luassert_always(get_data_unit_state(asset, EDITOR_UNIT) ==
            Asset::AssetDataUnitState::unloaded);
        ObjRef main_data = get_data_unit_object(asset, Name());
        luassert_always(main_data);
        auto typed_main_data = Asset::get_asset_data_unit_object<AssetTestData>(asset, Name());
        lupanic_if_failed(typed_main_data);
        luassert_always(typed_main_data.get().object() == main_data.get());

        lupanic_if_failed(Asset::load_asset_data_unit(asset, EDITOR_UNIT));
        luassert_always(g_probe.load_calls == 2);
        luassert_always(g_probe.last_load_unit == EDITOR_UNIT);
        luassert_always(get_data_unit_state(asset, EDITOR_UNIT) ==
            Asset::AssetDataUnitState::loaded);
        ObjRef editor_data = get_data_unit_object(asset, EDITOR_UNIT);
        luassert_always(editor_data);
        luassert_always(editor_data != main_data);

        lupanic_if_failed(Asset::load_asset_data_unit_default_data(asset, STREAMING_UNIT));
        luassert_always(g_probe.default_load_calls == 1);
        luassert_always(g_probe.last_default_load_unit == STREAMING_UNIT);
        luassert_always(get_data_unit_state(asset, STREAMING_UNIT) ==
            Asset::AssetDataUnitState::loaded);

        lupanic_if_failed(Asset::save_asset_data_unit(asset, Name()));
        luassert_always(g_probe.last_save_unit.empty());
        luassert_always(g_probe.last_saved_data == main_data);
        lupanic_if_failed(Asset::save_asset_data_unit(asset, EDITOR_UNIT));
        luassert_always(g_probe.last_save_unit == EDITOR_UNIT);
        luassert_always(g_probe.last_saved_data == editor_data);

        ObjRef replacement = new_test_data(100);
        lupanic_if_failed(Asset::set_asset_data_unit_object(asset, EDITOR_UNIT,
            replacement.get()));
        luassert_always(g_probe.last_set_unit == EDITOR_UNIT);
        luassert_always(g_probe.last_set_data == replacement);
        luassert_always(g_probe.data_before_set == editor_data);
        luassert_always(get_data_unit_object(asset, EDITOR_UNIT) == replacement);
        luassert_always(get_test_data_value(replacement) == 100);

        lupanic_if_failed(Asset::set_asset_data_unit_object(asset, EDITOR_UNIT, nullptr));
        luassert_always(g_probe.last_set_unit == EDITOR_UNIT);
        luassert_always(!g_probe.last_set_data);
        luassert_always(g_probe.data_before_set == replacement);
        luassert_always(get_data_unit_state(asset, EDITOR_UNIT) ==
            Asset::AssetDataUnitState::unloaded);
        luassert_always(get_data_unit_state(asset, Name()) ==
            Asset::AssetDataUnitState::loaded);

        g_probe.referred_asset = Asset::get_asset();
        Vector<Asset::asset_t> referred_assets;
        Asset::get_asset_data_unit_referred_assets(asset, STREAMING_UNIT,
            referred_assets);
        luassert_always(g_probe.referred_assets_calls == 1);
        luassert_always(g_probe.last_referred_assets_unit == STREAMING_UNIT);
        luassert_always(referred_assets.size() == 1);
        luassert_always(referred_assets[0] == g_probe.referred_asset);

        referred_assets.clear();
        Asset::get_asset_data_unit_referred_assets(asset, Name(), referred_assets);
        luassert_always(g_probe.referred_assets_calls == 2);
        luassert_always(g_probe.last_referred_assets_unit.empty());
        luassert_always(referred_assets.size() == 1);
        luassert_always(referred_assets[0] == g_probe.referred_asset);

        lupanic_if_failed(Asset::set_asset_data_unit_object(asset, Name(), nullptr));
        luassert_always(g_probe.last_set_unit.empty());
        luassert_always(get_data_unit_state(asset, Name()) ==
            Asset::AssetDataUnitState::unloaded);
        lupanic_if_failed(Asset::load_asset_data_unit_default_data(asset, Name()));
        luassert_always(g_probe.default_load_calls == 2);
        luassert_always(g_probe.last_default_load_unit.empty());
        luassert_always(get_data_unit_state(asset, Name()) ==
            Asset::AssetDataUnitState::loaded);
        lupanic_if_failed(Asset::set_asset_data_unit_object(asset, Name(), nullptr));
        lupanic_if_failed(Asset::set_asset_data_unit_object(asset, STREAMING_UNIT, nullptr));
    }

    void shared_data_and_error_test()
    {
        g_probe.reset_call_history();
        Asset::asset_t asset = new_test_asset("/AssetTest/Errors.asset");

        Asset::AssetDataUnitDesc shared_a;
        shared_a.id = SHARED_UNIT_A;
        shared_a.loader = AUXILIARY_LOADER;
        lupanic_if_failed(Asset::add_asset_data_unit(asset, shared_a));
        expect_error(Asset::add_asset_data_unit(asset, shared_a),
            Asset::E_ASSET_DATA_UNIT_ALREADY_EXISTS);

        Asset::AssetDataUnitDesc shared_b;
        shared_b.id = SHARED_UNIT_B;
        shared_b.loader = AUXILIARY_LOADER;
        lupanic_if_failed(Asset::add_asset_data_unit(asset, shared_b));

        ObjRef shared_data = new_test_data(200);
        lupanic_if_failed(Asset::set_asset_data_unit_object(asset, SHARED_UNIT_A,
            shared_data.get()));
        lupanic_if_failed(Asset::set_asset_data_unit_object(asset, SHARED_UNIT_B,
            shared_data.get()));
        luassert_always(get_data_unit_object(asset, SHARED_UNIT_A) == shared_data);
        luassert_always(get_data_unit_object(asset, SHARED_UNIT_B) == shared_data);

        expect_error(Asset::remove_asset_data_unit(asset, SHARED_UNIT_A),
            Asset::E_ASSET_DATA_UNIT_BUSY);
        lupanic_if_failed(Asset::set_asset_data_unit_object(asset, SHARED_UNIT_A, nullptr));
        lupanic_if_failed(Asset::remove_asset_data_unit(asset, SHARED_UNIT_A));
        expect_error(Asset::remove_asset_data_unit(asset, SHARED_UNIT_A),
            Asset::E_ASSET_DATA_UNIT_NOT_FOUND);
        expect_error(Asset::get_asset_data_unit_object(asset, SHARED_UNIT_A),
            Asset::E_ASSET_DATA_UNIT_NOT_FOUND);
        expect_error(Asset::get_asset_data_unit_state(asset, SHARED_UNIT_A),
            Asset::E_ASSET_DATA_UNIT_NOT_FOUND);
        expect_error(Asset::get_asset_data_unit_loader(asset, SHARED_UNIT_A),
            Asset::E_ASSET_DATA_UNIT_NOT_FOUND);

        Asset::AssetDataUnitDesc unknown;
        unknown.id = "Unknown";
        unknown.loader = UNKNOWN_LOADER;
        lupanic_if_failed(Asset::add_asset_data_unit(asset, unknown));
        auto unknown_loader = Asset::get_asset_data_unit_loader(asset, unknown.id);
        lupanic_if_failed(unknown_loader);
        luassert_always(unknown_loader.get() == UNKNOWN_LOADER);
        expect_error(Asset::load_asset_data_unit(asset, unknown.id),
            Asset::E_UNKNOWN_ASSET_LOADER);
        luassert_always(get_data_unit_state(asset, unknown.id) ==
            Asset::AssetDataUnitState::unloaded);

        Vector<Asset::asset_t> referred_assets;
        Asset::get_asset_data_unit_referred_assets(asset, unknown.id, referred_assets);
        luassert_always(referred_assets.empty());

        lupanic_if_failed(Asset::set_asset_data_unit_object(asset, SHARED_UNIT_B, nullptr));
        lupanic_if_failed(Asset::remove_asset_data_unit(asset, SHARED_UNIT_B));
        lupanic_if_failed(Asset::remove_asset_data_unit(asset, unknown.id));
    }

    void failed_force_reload_test()
    {
        g_probe.reset_call_history();
        Asset::asset_t asset = new_test_asset("/AssetTest/ForceReload.asset");
        Asset::AssetDataUnitDesc unit;
        unit.id = EDITOR_UNIT;
        unit.loader = AUXILIARY_LOADER;
        lupanic_if_failed(Asset::add_asset_data_unit(asset, unit));

        lupanic_if_failed(Asset::load_asset_data_unit(asset, EDITOR_UNIT));
        ObjRef original_data = get_data_unit_object(asset, EDITOR_UNIT);
        luassert_always(original_data);
        const i32 original_value = get_test_data_value(original_data);

        g_probe.fail_next_load = true;
        expect_error(Asset::load_asset_data_unit(asset, EDITOR_UNIT, true), E_BAD_DATA);
        ObjRef data_after_failure = get_data_unit_object(asset, EDITOR_UNIT);
        luassert_always(data_after_failure == original_data);
        luassert_always(get_test_data_value(data_after_failure) == original_value);
        luassert_always(get_data_unit_state(asset, EDITOR_UNIT) ==
            Asset::AssetDataUnitState::loaded);

        lupanic_if_failed(Asset::set_asset_data_unit_object(asset, EDITOR_UNIT, nullptr));
    }

    void concurrent_data_unit_test()
    {
        Asset::asset_t asset = new_test_asset("/AssetTest/Concurrent.asset");
        Asset::AssetDataUnitDesc unit_a;
        unit_a.id = PARALLEL_UNIT_A;
        unit_a.loader = BLOCKING_LOADER;
        lupanic_if_failed(Asset::add_asset_data_unit(asset, unit_a));
        Asset::AssetDataUnitDesc unit_b;
        unit_b.id = PARALLEL_UNIT_B;
        unit_b.loader = BLOCKING_LOADER;
        lupanic_if_failed(Asset::add_asset_data_unit(asset, unit_b));

        g_blocking_probe.unit_a_entered = new_signal(true);
        g_blocking_probe.unit_b_entered = new_signal(true);
        g_blocking_probe.release = new_signal(true);
        ConcurrentLoadContext context_a;
        context_a.asset = asset;
        context_a.data_unit = PARALLEL_UNIT_A;
        ConcurrentLoadContext context_b;
        context_b.asset = asset;
        context_b.data_unit = PARALLEL_UNIT_B;
        auto thread_a_result = new_thread(concurrent_load_thread, &context_a);
        lupanic_if_failed(thread_a_result);
        Ref<IThread> thread_a = thread_a_result.get();
        const bool unit_a_entered = wait_for_signal(g_blocking_probe.unit_a_entered);
        auto thread_b_result = new_thread(concurrent_load_thread, &context_b);
        lupanic_if_failed(thread_b_result);
        Ref<IThread> thread_b = thread_b_result.get();
        const bool unit_b_entered = wait_for_signal(g_blocking_probe.unit_b_entered);
        auto duplicate_load = Asset::load_asset_data_unit(asset, PARALLEL_UNIT_A);
        g_blocking_probe.release->trigger();
        thread_a->wait();
        thread_b->wait();

        luassert_always(unit_a_entered);
        luassert_always(unit_b_entered);
        expect_error(duplicate_load, Asset::E_ASSET_DATA_UNIT_BUSY);
        lupanic_if_failed(context_a.result);
        lupanic_if_failed(context_b.result);
        luassert_always(get_data_unit_state(asset, PARALLEL_UNIT_A) ==
            Asset::AssetDataUnitState::loaded);
        luassert_always(get_data_unit_state(asset, PARALLEL_UNIT_B) ==
            Asset::AssetDataUnitState::loaded);
        lupanic_if_failed(Asset::set_asset_data_unit_object(asset, PARALLEL_UNIT_A,
            nullptr));
        lupanic_if_failed(Asset::set_asset_data_unit_object(asset, PARALLEL_UNIT_B,
            nullptr));
        g_blocking_probe = BlockingLoadProbe();
    }

    void create_empty_test_file(const Path& path)
    {
        auto file = VFS::open_file(path, FileOpenFlag::write,
            FileCreationMode::create_always);
        lupanic_if_failed(file);
    }

    bool test_file_exists(const Path& path)
    {
        return succeeded(VFS::get_file_attribute(path));
    }

    void asset_file_lifecycle_test()
    {
        const Path original_path("/AssetTest/FileOps.asset");
        const Path moved_path("/AssetTest/Moved.asset");
        const Path copied_path("/AssetTest/Copied.asset");
        Asset::asset_t asset = new_test_asset(original_path.encode().c_str(), true);
        Asset::AssetDataUnitDesc unit;
        unit.id = EDITOR_UNIT;
        unit.loader = AUXILIARY_LOADER;
        lupanic_if_failed(Asset::add_asset_data_unit(asset, unit));
        lupanic_if_failed(Asset::save_asset_meta(asset));
        create_empty_test_file("/AssetTest/FileOps.asset.payload");
        lupanic_if_failed(Asset::load_asset_data_unit(asset, EDITOR_UNIT));
        ObjRef loaded_data = get_data_unit_object(asset, EDITOR_UNIT);

        lupanic_if_failed(Asset::move_asset(asset, moved_path));
        luassert_always(Asset::get_asset_path(asset) == moved_path);
        luassert_always(get_data_unit_object(asset, EDITOR_UNIT) == loaded_data);
        luassert_always(!test_file_exists("/AssetTest/FileOps.asset.meta"));
        luassert_always(!test_file_exists("/AssetTest/FileOps.asset.payload"));
        luassert_always(test_file_exists("/AssetTest/Moved.asset.meta"));
        luassert_always(test_file_exists("/AssetTest/Moved.asset.payload"));

        lupanic_if_failed(Asset::set_asset_data_unit_object(asset, EDITOR_UNIT, nullptr));
        lupanic_if_failed(Asset::remove_asset_data_unit(asset, EDITOR_UNIT));
        lupanic_if_failed(Asset::load_asset_meta(asset));
        Vector<Asset::AssetDataUnitDesc> moved_units;
        lupanic_if_failed(Asset::get_asset_data_units(asset, moved_units));
        luassert_always(contains_data_unit(moved_units, EDITOR_UNIT, AUXILIARY_LOADER));

        auto copied_asset_result = Asset::copy_asset(asset, copied_path);
        lupanic_if_failed(copied_asset_result);
        Asset::asset_t copied_asset = copied_asset_result.get();
        Vector<Asset::AssetDataUnitDesc> copied_units;
        lupanic_if_failed(Asset::get_asset_data_units(copied_asset, copied_units));
        luassert_always(contains_data_unit(copied_units, EDITOR_UNIT, AUXILIARY_LOADER));
        luassert_always(get_data_unit_state(copied_asset, EDITOR_UNIT) ==
            Asset::AssetDataUnitState::unloaded);
        luassert_always(test_file_exists("/AssetTest/Copied.asset.meta"));
        luassert_always(test_file_exists("/AssetTest/Copied.asset.payload"));

        lupanic_if_failed(Asset::delete_asset(copied_asset));
        luassert_always(get_data_unit_state(copied_asset, Name()) ==
            Asset::AssetDataUnitState::unregistered);
        luassert_always(!test_file_exists("/AssetTest/Copied.asset.meta"));
        luassert_always(!test_file_exists("/AssetTest/Copied.asset.payload"));
        luassert_always(failed(Asset::get_asset_by_path(copied_path)));
        lupanic_if_failed(Asset::delete_asset(asset));
        luassert_always(!test_file_exists("/AssetTest/Moved.asset.meta"));
        luassert_always(!test_file_exists("/AssetTest/Moved.asset.payload"));
    }

    void write_legacy_meta_file(const Path& asset_path, const Guid& guid)
    {
        Variant metadata(VariantType::object);
        auto serialized_guid = serialize(guid);
        lupanic_if_failed(serialized_guid);
        metadata["guid"] = serialized_guid.get();
        auto serialized_type = serialize(Name(TEST_ASSET_TYPE));
        lupanic_if_failed(serialized_type);
        metadata["type"] = serialized_type.get();

        Path meta_path = asset_path;
        meta_path.append_extension("meta");
        auto file = VFS::open_file(meta_path, FileOpenFlag::write,
            FileCreationMode::create_always);
        lupanic_if_failed(file);
        lupanic_if_failed(VariantUtils::write_json(file.get().get(), metadata));
    }

    void metadata_compatibility_test()
    {
        Asset::asset_t round_trip_asset = new_test_asset(
            "/AssetTest/RoundTrip.asset", true);
        Asset::AssetDataUnitDesc zeta;
        zeta.id = "Zeta";
        zeta.loader = AUXILIARY_LOADER;
        lupanic_if_failed(Asset::add_asset_data_unit(round_trip_asset, zeta));
        Asset::AssetDataUnitDesc alpha;
        alpha.id = "Alpha";
        alpha.loader = MAIN_LOADER;
        lupanic_if_failed(Asset::add_asset_data_unit(round_trip_asset, alpha));
        Asset::AssetDataUnitDesc delayed;
        delayed.id = "Delayed";
        delayed.loader = UNKNOWN_LOADER;
        lupanic_if_failed(Asset::add_asset_data_unit(round_trip_asset, delayed));
        lupanic_if_failed(Asset::save_asset_meta(round_trip_asset));

        lupanic_if_failed(Asset::remove_asset_data_unit(round_trip_asset, zeta.id));
        lupanic_if_failed(Asset::remove_asset_data_unit(round_trip_asset, alpha.id));
        lupanic_if_failed(Asset::remove_asset_data_unit(round_trip_asset, delayed.id));
        lupanic_if_failed(Asset::load_asset_meta(round_trip_asset));
        Vector<Asset::AssetDataUnitDesc> round_trip_units;
        lupanic_if_failed(Asset::get_asset_data_units(round_trip_asset, round_trip_units));
        luassert_always(round_trip_units.size() == 4);
        luassert_always(contains_data_unit(round_trip_units, Name(), MAIN_LOADER));
        luassert_always(contains_data_unit(round_trip_units, alpha.id, alpha.loader));
        luassert_always(contains_data_unit(round_trip_units, delayed.id, delayed.loader));
        luassert_always(contains_data_unit(round_trip_units, zeta.id, zeta.loader));
        luassert_always(round_trip_units[0].id.empty());
        luassert_always(round_trip_units[1].id == alpha.id);
        luassert_always(round_trip_units[2].id == delayed.id);
        luassert_always(round_trip_units[3].id == zeta.id);
        expect_error(Asset::load_asset_data_unit(round_trip_asset, delayed.id),
            Asset::E_UNKNOWN_ASSET_LOADER);
        Asset::AssetLoaderDesc delayed_loader;
        delayed_loader.name = UNKNOWN_LOADER;
        delayed_loader.on_load_asset_data_unit = on_load_asset_data_unit;
        delayed_loader.on_load_asset_data_unit_default_data =
            on_load_asset_data_unit_default_data;
        delayed_loader.on_save_asset_data_unit = on_save_asset_data_unit;
        delayed_loader.on_set_asset_data_unit = on_set_asset_data_unit;
        delayed_loader.on_get_referred_assets = on_get_referred_assets;
        Asset::register_asset_loader(delayed_loader);
        lupanic_if_failed(Asset::load_asset_data_unit(round_trip_asset, delayed.id));
        lupanic_if_failed(Asset::set_asset_data_unit_object(round_trip_asset,
            delayed.id, nullptr));
        lupanic_if_failed(Asset::load_asset_data_unit(round_trip_asset, alpha.id));
        ObjRef loaded_alpha = get_data_unit_object(round_trip_asset, alpha.id);
        lupanic_if_failed(Asset::load_asset_meta(round_trip_asset));
        luassert_always(get_data_unit_object(round_trip_asset, alpha.id) == loaded_alpha);
        lupanic_if_failed(Asset::set_asset_data_unit_object(round_trip_asset,
            alpha.id, nullptr));

        const Path legacy_path("/AssetTest/Legacy.asset");
        Guid legacy_guid = random_guid();
        write_legacy_meta_file(legacy_path, legacy_guid);
        lupanic_if_failed(Asset::load_assets_meta(legacy_path));
        auto legacy_asset_result = Asset::get_asset_by_path(legacy_path);
        lupanic_if_failed(legacy_asset_result);
        Asset::asset_t legacy_asset = legacy_asset_result.get();
        luassert_always(Asset::get_asset_guid(legacy_asset) == legacy_guid);
        luassert_always(Asset::get_asset_type(legacy_asset) == TEST_ASSET_TYPE);
        Vector<Asset::AssetDataUnitDesc> legacy_units;
        lupanic_if_failed(Asset::get_asset_data_units(legacy_asset, legacy_units));
        luassert_always(legacy_units.size() == 1);
        luassert_always(contains_data_unit(legacy_units, Name(), MAIN_LOADER));
        lupanic_if_failed(Asset::save_asset_meta(legacy_asset));
        Path legacy_meta_path = legacy_path;
        legacy_meta_path.append_extension("meta");
        auto legacy_meta_file = VFS::open_file(legacy_meta_path,
            FileOpenFlag::read, FileCreationMode::open_existing);
        lupanic_if_failed(legacy_meta_file);
        auto upgraded_metadata = VariantUtils::read_json(legacy_meta_file.get().get());
        lupanic_if_failed(upgraded_metadata);
        luassert_always(upgraded_metadata.get().contains("format_version"));
        luassert_always(upgraded_metadata.get()["format_version"].unum() == 2);
        luassert_always(upgraded_metadata.get().contains("data_units"));
        luassert_always(upgraded_metadata.get()["data_units"].type() ==
            VariantType::array);
        luassert_always(upgraded_metadata.get()["data_units"].size() == 0);
    }

    String create_native_test_directory()
    {
#if defined(LUNA_PLATFORM_WINDOWS)
        const c8* temporary_root = std::getenv("TEMP");
#else
        const c8* temporary_root = std::getenv("TMPDIR");
#endif
        if(!temporary_root || !temporary_root[0])
        {
#if defined(LUNA_PLATFORM_WINDOWS)
            temporary_root = ".";
#else
            temporary_root = "/tmp";
#endif
        }
        String path;
        strprintf(path, "%s/LunaAssetTest-%016llx", temporary_root,
            (unsigned long long)random_u64());
        lupanic_if_failed(create_dir(path.c_str()));
        return path;
    }

    void remove_test_file(const Path& path)
    {
        auto result = VFS::delete_file(path);
        if(failed(result))
        {
            luassert_always(unwrap_errcode(result) == E_NOT_FOUND);
        }
    }
}

int main()
{
    lupanic_if_failed(Luna::init());
    lupanic_if_failed(add_modules({module_variant_utils(), module_vfs(), module_asset()}));
    lupanic_if_failed(init_modules());
    Meta::register_AssetTest_types();

    String native_test_directory = create_native_test_directory();
    {
        auto file_system = VFS::new_native_file_system(native_test_directory.c_str());
        lupanic_if_failed(file_system);
        lupanic_if_failed(VFS::mount(file_system.get(), TEST_MOUNT_PATH));
    }

    register_test_asset_type();
    data_unit_lifecycle_test();
    shared_data_and_error_test();
    failed_force_reload_test();
    concurrent_data_unit_test();
    asset_file_lifecycle_test();
    metadata_compatibility_test();

    remove_test_file("/AssetTest/RoundTrip.asset.meta");
    remove_test_file("/AssetTest/Legacy.asset.meta");
    lupanic_if_failed(VFS::unmount(TEST_MOUNT_PATH));
    lupanic_if_failed(delete_file(native_test_directory.c_str()));
    g_probe = CallbackProbe();
    g_blocking_probe = BlockingLoadProbe();
    Luna::close();
    return 0;
}
