#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstddef>

struct BenchmarkConfig
{
    int dpi = 1600;
    int pollingHz = 1000;
    int durationSeconds = 10;
    double distanceInches = 0.0;

    int speedWindowMs = 10;

    double gapMultiplier = 4.0;
    double trackingGapMultiplier = 8.0;
    double spinoutMagnitudeMultiplier = 8.0;
    double negativeAccelDropRatio = 0.35;
};

struct MouseSample
{
    long long ticks = 0;
    LONG dx = 0;
    LONG dy = 0;
    USHORT mouseFlags = 0;
    USHORT buttonFlags = 0;
    bool noCoalesce = false;
};

struct BenchmarkSummary
{
    std::size_t sampleCount = 0;
    std::size_t motionSamples = 0;
    std::size_t zeroMotionSamples = 0;
    std::size_t noCoalesceSamples = 0;

    double captureSeconds = 0.0;
    double averageEventRateHz = 0.0;

    double estimatedPollingRateHz = 0.0;
    double pollingRatePercent = 0.0;

    double meanActiveIntervalMs = 0.0;
    double medianActiveIntervalMs = 0.0;
    double intervalStdDevMs = 0.0;
    double intervalVarianceMs2 = 0.0;

    bool hasMeasuredDpi = false;
    double measuredDpi = 0.0;

    double peakObservedIps = 0.0;

    int rawInputGapCandidates = 0;
    int trackingLossCandidates = 0;
    int spinoutCandidates = 0;
    int negativeAccelerationCandidates = 0;
};
