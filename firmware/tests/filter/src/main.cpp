#include <zephyr/ztest.h>
#include "filter.hpp"
#include <chrono>

using namespace std::chrono_literals;

ZTEST(filter_tests, test_initial_reading)
{
    ReadingFilter filter(500ms, 0.5, 2.0, 1.0);
    SensorReading r1{
        .temperature = 20.0,
        .humidity = 50.0,
        .pressure = 1000.0,
        .address = 0x77
    };
    
    // First reading should always publish
    zassert_true(filter.should_publish(r1, 0ms), "Initial reading must be published");
}

ZTEST(filter_tests, test_time_threshold)
{
    ReadingFilter filter(500ms, 0.5, 2.0, 1.0);
    SensorReading r1{
        .temperature = 20.0,
        .humidity = 50.0,
        .pressure = 1000.0,
        .address = 0x77
    };
    
    filter.should_publish(r1, 0ms);

    // Identical reading before interval elapsed should NOT publish
    zassert_false(filter.should_publish(r1, 100ms), "Should not publish before interval");

    // Identical reading after interval elapsed SHOULD publish
    zassert_true(filter.should_publish(r1, 500ms), "Should publish after interval elapsed");
}

ZTEST(filter_tests, test_delta_temperature)
{
    ReadingFilter filter(500ms, 0.5, 2.0, 1.0);
    SensorReading r1{
        .temperature = 20.0,
        .humidity = 50.0,
        .pressure = 1000.0,
        .address = 0x77
    };
    
    filter.should_publish(r1, 0ms);

    // Temperature changes below delta (0.4 < 0.5) should NOT publish
    SensorReading r2{
        .temperature = 20.4,
        .humidity = 50.0,
        .pressure = 1000.0,
        .address = 0x77
    };
    zassert_false(filter.should_publish(r2, 100ms), "Should not publish below temp delta");

    // Temperature changes equal/above delta (0.5 >= 0.5) SHOULD publish
    SensorReading r3{
        .temperature = 20.5,
        .humidity = 50.0,
        .pressure = 1000.0,
        .address = 0x77
    };
    zassert_true(filter.should_publish(r3, 200ms), "Should publish at temp delta");
}

ZTEST_SUITE(filter_tests, NULL, NULL, NULL, NULL, NULL);
