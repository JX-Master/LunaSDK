```c++
#include <Luna/Runtime/Time.hpp>
```

## High-resolution CPU timer

All modern CPUs contain high-resolution timers whose values will increase constantly and monotonically after CPU is powered or reset, usually once per several nanoseconds. The value of this timer can be used to measure time interval at a high resolution.

Use `get_ticks` to read the current value of the high-resolution CPU timer. The time value is an `u64` integer measured in CPU ticks, which is a platform-dependent small unit. The user can then call `get_ticks_per_second` to get the number of ticks per second on the current platform, and use this number to convert ticks to seconds. Remember to use `f64` instead of `f32` when performing high-resolution time calculation measured in seconds., since `f32` does not provide enough precision for representing such a tiny value.

## System time

Besides the high-resolution CPU timer, the underlying platform/OS also contains a timer that tracks the system time on the current platform, which can usually be changed by the user. Unlike CPU time, the system time is affected by the time zone and daylight saving time (DST) settings on the platform, so requires additional care when we're handling it.

In LunaSDK, the system time is represented by a `i64` UNIX timestamp (number of seconds from Jan 1st, 1970, UTC). The user can call `get_local_timestamp` to get the current system time shifted by the time zone and DST settings on the current platform, or call `get_utc_timestamp` to get the current system time in UTC. The local and UTC timestamp can be converted to each other by `local_timestamp_to_utc_timestamp` and `utc_timestamp_to_local_timestamp`. To convert one timestamp to one calendar form, the user can call `timestamp_to_datetime`, which returns one `DateTime` structure that contains the year, month, day, hour, minute, second and day of week of the timestamp. The user can also call `datetime_to_timestamp` to convert one `DateTime` to its corresponding timestamp.