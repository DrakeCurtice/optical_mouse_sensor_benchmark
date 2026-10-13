#pragma once

#include <chrono>
#include <cstdint>

using Clock = std::chrono::steady_clock;
using Timestamp = Clock::time_point;

struct BoolAlert {
    bool value;
    Timestamp timestamp;
    double confidence;
};

struct SensorReading {
    int32_t dx;
    int32_t dy;
    Timestamp timestamp;
};


// Signal<BoolAlert> alertSignal;
// Signal<SensorReading> sensorSignal;
// Timestamp now = Clock::now();