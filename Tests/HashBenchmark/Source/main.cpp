/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file main.cpp
* @author JXMaster
* @date 2026/6/3
*/
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/RobinMap.hpp>
#include <Luna/Runtime/RobinSet.hpp>
#include <Luna/Runtime/UnorderedMap.hpp>
#include <Luna/Runtime/UnorderedSet.hpp>
#include <Luna/Runtime/Time.hpp>
#include <Luna/Runtime/Vector.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace Luna;

namespace
{
    volatile usize g_sink = 0;

    struct Config
    {
        usize size = 117964;
        usize lookup_ops = 471856;
        usize churn_ops = 58982;
        usize samples = 3;
    };

    struct Stats
    {
        usize size = 0;
        usize capacity = 0;
        usize buckets = 0;
        f32 load_factor = 0.0f;
    };

    struct Trial
    {
        f64 ms = 0.0;
        Stats stats;
        usize sink = 0;
    };

    struct Result
    {
        const c8* scenario = nullptr;
        const c8* container = nullptr;
        const c8* operation = nullptr;
        const c8* impl = nullptr;
        f64 ms = 0.0;
        f64 ns_per_op = 0.0;
        Stats stats;
        usize sink = 0;
    };

    u64 splitmix64(usize index)
    {
        u64 x = (u64)index + 0x9E3779B97F4A7C15ull;
        x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
        x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
        return x ^ (x >> 31);
    }

    usize parse_usize(const c8* text, usize fallback)
    {
        c8* end = nullptr;
        usize value = (usize)strtoull(text, &end, 10);
        return (end && *end == '\0' && value) ? value : fallback;
    }

    Config parse_config(int argc, char** argv)
    {
        Config cfg;
        for (int i = 1; i + 1 < argc; ++i)
        {
            if (!strcmp(argv[i], "--size"))
            {
                cfg.size = parse_usize(argv[++i], cfg.size);
            }
            else if (!strcmp(argv[i], "--lookups"))
            {
                cfg.lookup_ops = parse_usize(argv[++i], cfg.lookup_ops);
            }
            else if (!strcmp(argv[i], "--churn"))
            {
                cfg.churn_ops = parse_usize(argv[++i], cfg.churn_ops);
            }
            else if (!strcmp(argv[i], "--samples"))
            {
                cfg.samples = parse_usize(argv[++i], cfg.samples);
            }
        }
        return cfg;
    }

    template <typename _Container>
    Stats get_stats(const _Container& c)
    {
        Stats ret;
        ret.size = c.size();
        ret.capacity = c.capacity();
        ret.buckets = c.hash_table_size();
        ret.load_factor = c.load_factor();
        return ret;
    }

    template <typename _Kty, typename _Hash, typename _KeyEqual, typename _Alloc>
    Stats get_stats(const UnorderedSet<_Kty, _Hash, _KeyEqual, _Alloc>& c)
    {
        Stats ret;
        ret.size = c.size();
        ret.capacity = c.capacity();
        ret.buckets = c.bucket_count();
        ret.load_factor = c.load_factor();
        return ret;
    }

    template <typename _Kty, typename _Ty, typename _Hash, typename _KeyEqual, typename _Alloc>
    Stats get_stats(const UnorderedMap<_Kty, _Ty, _Hash, _KeyEqual, _Alloc>& c)
    {
        Stats ret;
        ret.size = c.size();
        ret.capacity = c.capacity();
        ret.buckets = c.bucket_count();
        ret.load_factor = c.load_factor();
        return ret;
    }

    f64 elapsed_ms(u64 start, u64 end)
    {
        return (f64)(end - start) * 1000.0 / get_ticks_per_second();
    }

    template <typename _Fn>
    Result run_best(const c8* scenario, const c8* container, const c8* operation, const c8* impl, usize op_count, usize samples, _Fn&& fn)
    {
        Result best;
        best.scenario = scenario;
        best.container = container;
        best.operation = operation;
        best.impl = impl;
        best.ms = 1.0e300;
        for (usize i = 0; i < samples; ++i)
        {
            Trial t = fn();
            g_sink ^= t.sink;
            if (t.ms < best.ms)
            {
                best.ms = t.ms;
                best.stats = t.stats;
                best.sink = t.sink;
            }
        }
        best.ns_per_op = best.ms * 1000000.0 / (f64)op_count;
        return best;
    }

    void print_result(const Result& r)
    {
        printf("%-15s %-8s %-14s %-10s %10.3f %12.2f %10llu %10llu %10llu %8.2f\n",
            r.scenario,
            r.container,
            r.operation,
            r.impl,
            r.ms,
            r.ns_per_op,
            (unsigned long long)r.stats.size,
            (unsigned long long)r.stats.capacity,
            (unsigned long long)r.stats.buckets,
            (double)r.stats.load_factor);
    }

    template <typename _Set>
    void build_set(_Set& set, const Vector<u64>& keys, f32 max_load_factor)
    {
        set.max_load_factor(max_load_factor);
        set.reserve(keys.size());
        for (usize i = 0; i < keys.size(); ++i)
        {
            set.insert(keys[i]);
        }
    }

    template <typename _Map>
    void build_map(_Map& map, const Vector<u64>& keys, f32 max_load_factor)
    {
        map.max_load_factor(max_load_factor);
        map.reserve(keys.size());
        for (usize i = 0; i < keys.size(); ++i)
        {
            map.insert(make_pair(keys[i], keys[i] ^ 0xD1B54A32D192ED03ull));
        }
    }

    template <typename _Set>
    Result bench_set_insert(const c8* scenario, const c8* impl, const Vector<u64>& keys, f32 max_load_factor, usize samples)
    {
        return run_best(scenario, "Set", "insert", impl, keys.size(), samples, [&]() {
            _Set set;
            set.max_load_factor(max_load_factor);
            set.reserve(keys.size());
            u64 start = get_ticks();
            for (usize i = 0; i < keys.size(); ++i)
            {
                set.insert(keys[i]);
            }
            u64 end = get_ticks();
            Trial ret;
            ret.ms = elapsed_ms(start, end);
            ret.stats = get_stats(set);
            ret.sink = ret.stats.size ^ ret.stats.buckets;
            return ret;
        });
    }

    template <typename _Set>
    Result bench_set_find_hit(const c8* scenario, const c8* impl, const Vector<u64>& keys, f32 max_load_factor, usize ops, usize samples)
    {
        return run_best(scenario, "Set", "find_hit", impl, ops, samples, [&]() {
            _Set set;
            build_set(set, keys, max_load_factor);
            usize sink = 0;
            u64 start = get_ticks();
            for (usize i = 0; i < ops; ++i)
            {
                auto iter = set.find(keys[i % keys.size()]);
                if (iter != set.end()) sink ^= (usize)*iter;
            }
            u64 end = get_ticks();
            Trial ret;
            ret.ms = elapsed_ms(start, end);
            ret.stats = get_stats(set);
            ret.sink = sink;
            return ret;
        });
    }

    template <typename _Set>
    Result bench_set_find_miss(const c8* scenario, const c8* impl, const Vector<u64>& keys, const Vector<u64>& miss_keys, f32 max_load_factor, usize ops, usize samples)
    {
        return run_best(scenario, "Set", "find_miss", impl, ops, samples, [&]() {
            _Set set;
            build_set(set, keys, max_load_factor);
            usize sink = 0;
            u64 start = get_ticks();
            for (usize i = 0; i < ops; ++i)
            {
                auto iter = set.find(miss_keys[i % miss_keys.size()]);
                sink ^= (iter == set.end()) ? 1 : (usize)*iter;
            }
            u64 end = get_ticks();
            Trial ret;
            ret.ms = elapsed_ms(start, end);
            ret.stats = get_stats(set);
            ret.sink = sink;
            return ret;
        });
    }

    template <typename _Set>
    Result bench_set_churn(const c8* scenario, const c8* impl, const Vector<u64>& keys, const Vector<u64>& churn_keys, f32 max_load_factor, usize ops, usize samples)
    {
        return run_best(scenario, "Set", "erase_insert", impl, ops * 2, samples, [&]() {
            _Set set;
            build_set(set, keys, max_load_factor);
            usize sink = 0;
            u64 start = get_ticks();
            for (usize i = 0; i < ops; ++i)
            {
                sink ^= set.erase(keys[i % keys.size()]);
                auto r = set.insert(churn_keys[i % churn_keys.size()]);
                sink ^= r.second ? 3 : 7;
            }
            u64 end = get_ticks();
            Trial ret;
            ret.ms = elapsed_ms(start, end);
            ret.stats = get_stats(set);
            ret.sink = sink ^ set.size();
            return ret;
        });
    }

    template <typename _Map>
    Result bench_map_insert(const c8* scenario, const c8* impl, const Vector<u64>& keys, f32 max_load_factor, usize samples)
    {
        return run_best(scenario, "Map", "insert", impl, keys.size(), samples, [&]() {
            _Map map;
            map.max_load_factor(max_load_factor);
            map.reserve(keys.size());
            u64 start = get_ticks();
            for (usize i = 0; i < keys.size(); ++i)
            {
                map.insert(make_pair(keys[i], keys[i] ^ 0xD1B54A32D192ED03ull));
            }
            u64 end = get_ticks();
            Trial ret;
            ret.ms = elapsed_ms(start, end);
            ret.stats = get_stats(map);
            ret.sink = ret.stats.size ^ ret.stats.buckets;
            return ret;
        });
    }

    template <typename _Map>
    Result bench_map_find_hit(const c8* scenario, const c8* impl, const Vector<u64>& keys, f32 max_load_factor, usize ops, usize samples)
    {
        return run_best(scenario, "Map", "find_hit", impl, ops, samples, [&]() {
            _Map map;
            build_map(map, keys, max_load_factor);
            usize sink = 0;
            u64 start = get_ticks();
            for (usize i = 0; i < ops; ++i)
            {
                auto iter = map.find(keys[i % keys.size()]);
                if (iter != map.end()) sink ^= (usize)iter->second;
            }
            u64 end = get_ticks();
            Trial ret;
            ret.ms = elapsed_ms(start, end);
            ret.stats = get_stats(map);
            ret.sink = sink;
            return ret;
        });
    }

    template <typename _Map>
    Result bench_map_find_miss(const c8* scenario, const c8* impl, const Vector<u64>& keys, const Vector<u64>& miss_keys, f32 max_load_factor, usize ops, usize samples)
    {
        return run_best(scenario, "Map", "find_miss", impl, ops, samples, [&]() {
            _Map map;
            build_map(map, keys, max_load_factor);
            usize sink = 0;
            u64 start = get_ticks();
            for (usize i = 0; i < ops; ++i)
            {
                auto iter = map.find(miss_keys[i % miss_keys.size()]);
                sink ^= (iter == map.end()) ? 1 : (usize)iter->second;
            }
            u64 end = get_ticks();
            Trial ret;
            ret.ms = elapsed_ms(start, end);
            ret.stats = get_stats(map);
            ret.sink = sink;
            return ret;
        });
    }

    template <typename _Map>
    Result bench_map_churn(const c8* scenario, const c8* impl, const Vector<u64>& keys, const Vector<u64>& churn_keys, f32 max_load_factor, usize ops, usize samples)
    {
        return run_best(scenario, "Map", "erase_insert", impl, ops * 2, samples, [&]() {
            _Map map;
            build_map(map, keys, max_load_factor);
            usize sink = 0;
            u64 start = get_ticks();
            for (usize i = 0; i < ops; ++i)
            {
                sink ^= map.erase(keys[i % keys.size()]);
                auto r = map.insert(make_pair(churn_keys[i % churn_keys.size()], churn_keys[i % churn_keys.size()] ^ 0xD1B54A32D192ED03ull));
                sink ^= r.second ? 3 : 7;
            }
            u64 end = get_ticks();
            Trial ret;
            ret.ms = elapsed_ms(start, end);
            ret.stats = get_stats(map);
            ret.sink = sink ^ map.size();
            return ret;
        });
    }

    void fill_keys(Vector<u64>& keys, usize count, usize start_index)
    {
        keys.reserve(count);
        for (usize i = 0; i < count; ++i)
        {
            keys.push_back(splitmix64(start_index + i));
        }
    }

    void run_scenario(const c8* name, const Vector<u64>& keys, const Vector<u64>& miss_keys, const Vector<u64>& churn_keys, f32 sparse_load, f32 robin_load, f32 unordered_load, const Config& cfg)
    {
        print_result(bench_set_insert<HashSet<u64>>(name, "Hash", keys, sparse_load, cfg.samples));
        print_result(bench_set_insert<RobinSet<u64>>(name, "Robin", keys, robin_load, cfg.samples));
        print_result(bench_set_insert<UnorderedSet<u64>>(name, "Unord", keys, unordered_load, cfg.samples));
        print_result(bench_set_find_hit<HashSet<u64>>(name, "Hash", keys, sparse_load, cfg.lookup_ops, cfg.samples));
        print_result(bench_set_find_hit<RobinSet<u64>>(name, "Robin", keys, robin_load, cfg.lookup_ops, cfg.samples));
        print_result(bench_set_find_hit<UnorderedSet<u64>>(name, "Unord", keys, unordered_load, cfg.lookup_ops, cfg.samples));
        print_result(bench_set_find_miss<HashSet<u64>>(name, "Hash", keys, miss_keys, sparse_load, cfg.lookup_ops, cfg.samples));
        print_result(bench_set_find_miss<RobinSet<u64>>(name, "Robin", keys, miss_keys, robin_load, cfg.lookup_ops, cfg.samples));
        print_result(bench_set_find_miss<UnorderedSet<u64>>(name, "Unord", keys, miss_keys, unordered_load, cfg.lookup_ops, cfg.samples));
        print_result(bench_set_churn<HashSet<u64>>(name, "Hash", keys, churn_keys, sparse_load, cfg.churn_ops, cfg.samples));
        print_result(bench_set_churn<RobinSet<u64>>(name, "Robin", keys, churn_keys, robin_load, cfg.churn_ops, cfg.samples));
        print_result(bench_set_churn<UnorderedSet<u64>>(name, "Unord", keys, churn_keys, unordered_load, cfg.churn_ops, cfg.samples));

        print_result(bench_map_insert<HashMap<u64, u64>>(name, "Hash", keys, sparse_load, cfg.samples));
        print_result(bench_map_insert<RobinMap<u64, u64>>(name, "Robin", keys, robin_load, cfg.samples));
        print_result(bench_map_insert<UnorderedMap<u64, u64>>(name, "Unord", keys, unordered_load, cfg.samples));
        print_result(bench_map_find_hit<HashMap<u64, u64>>(name, "Hash", keys, sparse_load, cfg.lookup_ops, cfg.samples));
        print_result(bench_map_find_hit<RobinMap<u64, u64>>(name, "Robin", keys, robin_load, cfg.lookup_ops, cfg.samples));
        print_result(bench_map_find_hit<UnorderedMap<u64, u64>>(name, "Unord", keys, unordered_load, cfg.lookup_ops, cfg.samples));
        print_result(bench_map_find_miss<HashMap<u64, u64>>(name, "Hash", keys, miss_keys, sparse_load, cfg.lookup_ops, cfg.samples));
        print_result(bench_map_find_miss<RobinMap<u64, u64>>(name, "Robin", keys, miss_keys, robin_load, cfg.lookup_ops, cfg.samples));
        print_result(bench_map_find_miss<UnorderedMap<u64, u64>>(name, "Unord", keys, miss_keys, unordered_load, cfg.lookup_ops, cfg.samples));
        print_result(bench_map_churn<HashMap<u64, u64>>(name, "Hash", keys, churn_keys, sparse_load, cfg.churn_ops, cfg.samples));
        print_result(bench_map_churn<RobinMap<u64, u64>>(name, "Robin", keys, churn_keys, robin_load, cfg.churn_ops, cfg.samples));
        print_result(bench_map_churn<UnorderedMap<u64, u64>>(name, "Unord", keys, churn_keys, unordered_load, cfg.churn_ops, cfg.samples));
    }
}

int main(int argc, char** argv)
{
    lupanic_if_failed(init());
    Config cfg = parse_config(argc, argv);
    if (cfg.churn_ops > cfg.size) cfg.churn_ops = cfg.size;

    Vector<u64> keys;
    Vector<u64> miss_keys;
    Vector<u64> churn_keys;
    fill_keys(keys, cfg.size, 0);
    fill_keys(miss_keys, cfg.size, cfg.size);
    fill_keys(churn_keys, cfg.churn_ops, cfg.size * 2);

    printf("HashBenchmark\n");
    printf("size=%llu lookups=%llu churn=%llu samples=%llu\n",
        (unsigned long long)cfg.size,
        (unsigned long long)cfg.lookup_ops,
        (unsigned long long)cfg.churn_ops,
        (unsigned long long)cfg.samples);
    printf("%-15s %-8s %-14s %-10s %10s %12s %10s %10s %10s %8s\n",
        "scenario", "kind", "operation", "impl", "ms", "ns/op", "size", "capacity", "buckets", "load");

    run_scenario("target-lf09", keys, miss_keys, churn_keys, 0.9f, 0.9f, 0.9f, cfg);
    run_scenario("sparse-default", keys, miss_keys, churn_keys, 2.0f, 0.9f, 2.0f, cfg);

    printf("sink=%llu\n", (unsigned long long)g_sink);
    close();
    return 0;
}
