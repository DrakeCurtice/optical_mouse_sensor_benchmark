#pragma once

#include "BenchmarkTypes.h"

#include <vector>

class BenchmarkAnalyzer
{
public:
    explicit BenchmarkAnalyzer(const BenchmarkConfig& config);

    BenchmarkSummary analyze(
        const std::vector<MouseSample>& samples,
        long long qpcFrequency,
        double captureSeconds
    ) const;

private:
    double ticksToSeconds(
        long long ticks,
        long long qpcFrequency
    ) const;

    double sampleMagnitude(
        const MouseSample& sample
    ) const;

    double median(
        std::vector<double> values
    ) const;

    BenchmarkConfig config_;
};
