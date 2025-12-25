/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TimeTest.cpp
* @author JXMaster
* @date 2020/9/25
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/Time.hpp>

namespace Luna
{
    void time_test()
    {
        // Get UNIX timestamp.
        i64 timestamp = get_local_timestamp();

        printf("Timestamp:%lld\n", (long long int)timestamp);

        // Get local time.
        DateTime local_time = timestamp_to_datetime(timestamp);
        
        // Get utc time.
        i64 utc_timestamp = local_timestamp_to_utc_timestamp(timestamp);
        DateTime utc_time = timestamp_to_datetime(utc_timestamp);

        lutest(utc_timestamp_to_local_timestamp(utc_timestamp) == timestamp);

        // Print local and utc time.
        printf("Local Time: %hu-%02hu-%02hu %02hu:%02hu:%02hu\n", 
            local_time.year, local_time.month, local_time.day, local_time.hour, local_time.minute, local_time.second);
        printf("UTC Time  : %hu-%02hu-%02hu %02hu:%02hu:%02hu\n",
            utc_time.year, utc_time.month, utc_time.day, utc_time.hour, utc_time.minute, utc_time.second);

        // Test epoch.
        i64 epoch = 86400;
        DateTime epoch_dt = timestamp_to_datetime(epoch);
        lutest(epoch_dt.year == 1970);
        lutest(epoch_dt.month == 1);
        lutest(epoch_dt.day == 2);
        lutest(epoch_dt.hour == 0);
        lutest(epoch_dt.minute == 0);
        lutest(epoch_dt.second == 0);
        lutest(datetime_to_timestamp(epoch_dt) == epoch);
    }
}