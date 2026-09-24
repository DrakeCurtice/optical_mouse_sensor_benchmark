#include "BenchmarkAnalyzer.h"

#include <algorithm>
#include <cmath>
#include <numeric>

BenchmarkAnalyzer::BenchmarkAnalyzer(const BenchmarkConfig& config)
    : config_(config)
{
}

BenchmarkSummary BenchmarkAnalyzer::analyze(
    const std::vector<MouseSample>& samples,
    long long qpcFrequency,
    double captureSeconds) const
{
    BenchmarkSummary summary;
    summary.sampleCount = samples.size();
    summary.captureSeconds = captureSeconds;

    if (captureSeconds > 0.0)
    {
        summary.averageEventRateHz =
            static_cast<double>(samples.size()) /
            captureSeconds;
    }

    for (const MouseSample& sample : samples)
    {
        if (sample.dx != 0 || sample.dy != 0)
        {
            summary.motionSamples++;
        }
        else
        {
            summary.zeroMotionSamples++;
        }

        if (sample.noCoalesce)
        {
            summary.noCoalesceSamples++;
        }
    }

    if (samples.size() < 2 ||
        qpcFrequency <= 0 ||
        config_.pollingHz <= 0)
    {
        return summary;
    }

    const double expectedIntervalSeconds =
        1.0 /
        static_cast<double>(config_.pollingHz);

    std::vector<double> activeIntervalsSeconds;

    for (std::size_t index = 1;
         index < samples.size();
         ++index)
    {
        const double interval =
            ticksToSeconds(
                samples[index].ticks -
                samples[index - 1].ticks,
                qpcFrequency
            );

        if (interval <= 0.0)
        {
            continue;
        }

        // Slow movement can legitimately produce long intervals.
        // Those long intervals should not be treated as polling jitter.
        if (interval <= expectedIntervalSeconds * 2.5)
        {
            activeIntervalsSeconds.push_back(interval);
        }
    }

    if (!activeIntervalsSeconds.empty())
    {
        const double meanSeconds =
            std::accumulate(
                activeIntervalsSeconds.begin(),
                activeIntervalsSeconds.end(),
                0.0
            ) /
            static_cast<double>(
                activeIntervalsSeconds.size()
            );

        double varianceSecondsSquared = 0.0;

        for (double interval : activeIntervalsSeconds)
        {
            const double difference =
                interval - meanSeconds;

            varianceSecondsSquared +=
                difference * difference;
        }

        varianceSecondsSquared /=
            static_cast<double>(
                activeIntervalsSeconds.size()
            );

        const double medianSeconds =
            median(activeIntervalsSeconds);

        summary.meanActiveIntervalMs =
            meanSeconds * 1000.0;

        summary.medianActiveIntervalMs =
            medianSeconds * 1000.0;

        summary.intervalStdDevMs =
            std::sqrt(varianceSecondsSquared) *
            1000.0;

        summary.intervalVarianceMs2 =
            varianceSecondsSquared *
            1'000'000.0;

        if (medianSeconds > 0.0)
        {
            summary.estimatedPollingRateHz =
                1.0 / medianSeconds;

            summary.pollingRatePercent =
                summary.estimatedPollingRateHz /
                static_cast<double>(config_.pollingHz) *
                100.0;
        }
    }

    long long totalDx = 0;

    for (const MouseSample& sample : samples)
    {
        totalDx += sample.dx;
    }

    if (config_.distanceInches > 0.0)
    {
        summary.measuredDpi =
            std::abs(
                static_cast<double>(totalDx)
            ) /
            config_.distanceInches;

        summary.hasMeasuredDpi = true;
    }

    // UNDERSTAND SLIDING WINDOW RELATIVE CALCUATION HOW MUCH DISPLACEMENT IN 10 MS

    const double speedWindowSeconds =
        static_cast<double>(config_.speedWindowMs) /
        1000.0;

    std::vector<long long> cumulativeDx(
        samples.size() + 1,
        0
    );

    std::vector<long long> cumulativeDy(
        samples.size() + 1,
        0
    );

    for (std::size_t index = 0;
         index < samples.size();
         ++index)
    {
        cumulativeDx[index + 1] =
            cumulativeDx[index] +
            samples[index].dx;

        cumulativeDy[index + 1] =
            cumulativeDy[index] +
            samples[index].dy;
    }

    std::vector<double> rollingSpeedsIps(
        samples.size(),
        0.0
    );

    std::size_t speedStart = 0;

    for (std::size_t speedEnd = 1;
         speedEnd < samples.size();
         ++speedEnd)
    {
        while (
            speedStart + 1 < speedEnd &&
            ticksToSeconds(
                samples[speedEnd].ticks -
                samples[speedStart].ticks,
                qpcFrequency
            ) > speedWindowSeconds)
        {
            speedStart++;
        }

        const double duration =
            ticksToSeconds(
                samples[speedEnd].ticks -
                samples[speedStart].ticks,
                qpcFrequency
            );

        if (duration < speedWindowSeconds * 0.5)
        {
            continue;
        }

        const long long dx =
            cumulativeDx[speedEnd + 1] -
            cumulativeDx[speedStart + 1];

        const long long dy =
            cumulativeDy[speedEnd + 1] -
            cumulativeDy[speedStart + 1];

        const double counts =
            std::hypot(
                static_cast<double>(dx),
                static_cast<double>(dy)
            );

        const double inches =
            counts /
            static_cast<double>(config_.dpi);

        const double speedIps =
            inches / duration;

        rollingSpeedsIps[speedEnd] =
            speedIps;

        summary.peakObservedIps =
            std::max(
                summary.peakObservedIps,
                speedIps
            );
    }

    for (std::size_t index = 1;
         index < samples.size();
         ++index)
    {
        const double interval =
            ticksToSeconds(
                samples[index].ticks -
                samples[index - 1].ticks,
                qpcFrequency
            );

        if (interval <= 0.0)
        {
            continue;
        }

        const double intervalRatio =
            interval /
            expectedIntervalSeconds;

        const double previousMagnitude =
            sampleMagnitude(
                samples[index - 1]
            );

        const double currentMagnitude =
            sampleMagnitude(
                samples[index]
            );

        bool activeBefore = false;
        bool activeAfter = false;

        if (index >= 2)
        {
            const double beforeInterval =
                ticksToSeconds(
                    samples[index - 1].ticks -
                    samples[index - 2].ticks,
                    qpcFrequency
                );

            activeBefore =
                beforeInterval > 0.0 &&
                beforeInterval <=
                    expectedIntervalSeconds * 2.5;
        }

        if (index + 1 < samples.size())
        {
            const double afterInterval =
                ticksToSeconds(
                    samples[index + 1].ticks -
                    samples[index].ticks,
                    qpcFrequency
                );

            activeAfter =
                afterInterval > 0.0 &&
                afterInterval <=
                    expectedIntervalSeconds * 2.5;
        }

        const bool continuousMotion =
            previousMagnitude > 0.0 &&
            currentMagnitude > 0.0;

        if (continuousMotion &&
            activeBefore &&
            activeAfter &&
            intervalRatio >
                config_.gapMultiplier)
        {
            summary.rawInputGapCandidates++;
        }

        if (continuousMotion &&
            activeBefore &&
            activeAfter &&
            intervalRatio >
                config_.trackingGapMultiplier)
        {
            summary.trackingLossCandidates++;
        }

        if (index >= 3 &&
            previousMagnitude > 0.0 &&
            currentMagnitude > 0.0)
        {
            const std::size_t localStart =
                index > 8
                    ? index - 8
                    : 0;

            std::vector<double> recentMagnitudes;

            for (std::size_t earlier = localStart;
                 earlier < index;
                 ++earlier)
            {
                const double magnitude =
                    sampleMagnitude(
                        samples[earlier]
                    );

                if (magnitude > 0.0)
                {
                    recentMagnitudes.push_back(
                        magnitude
                    );
                }
            }

            if (!recentMagnitudes.empty())
            {
                const double localMedian =
                    median(recentMagnitudes);

                const double dotProduct =
                    static_cast<double>(
                        samples[index - 1].dx
                    ) *
                    static_cast<double>(
                        samples[index].dx
                    ) +
                    static_cast<double>(
                        samples[index - 1].dy
                    ) *
                    static_cast<double>(
                        samples[index].dy
                    );

                double cosine =
                    dotProduct /
                    (
                        previousMagnitude *
                        currentMagnitude
                    );

                cosine =
                    std::clamp(
                        cosine,
                        -1.0,
                        1.0
                    );

                const bool magnitudeSpike =
                    localMedian > 0.0 &&
                    currentMagnitude >
                        localMedian *
                        config_.spinoutMagnitudeMultiplier;

                const bool sharpReversal =
                    cosine < -0.5;

                if (magnitudeSpike &&
                    sharpReversal)
                {
                    summary.spinoutCandidates++;
                }
            }
        }

        // Heuristic only. A true negative-acceleration measurement
        // requires independent physical-speed ground truth.
        if (index >= 6 &&
            rollingSpeedsIps[index] > 0.0)
        {
            std::vector<double> priorSpeeds;

            const std::size_t speedHistoryStart =
                index > 6
                    ? index - 6
                    : 0;

            for (std::size_t earlier = speedHistoryStart;
                 earlier < index;
                 ++earlier)
            {
                if (rollingSpeedsIps[earlier] > 0.0)
                {
                    priorSpeeds.push_back(
                        rollingSpeedsIps[earlier]
                    );
                }
            }

            if (!priorSpeeds.empty())
            {
                const double priorMedianSpeed =
                    median(priorSpeeds);

                const bool fastBefore =
                    priorMedianSpeed >= 50.0;

                const bool largeReportedSpeedDrop =
                    rollingSpeedsIps[index] <
                    priorMedianSpeed *
                    config_.negativeAccelDropRatio;

                const bool stillActive =
                    interval <=
                    expectedIntervalSeconds * 2.5;

                if (fastBefore &&
                    largeReportedSpeedDrop &&
                    continuousMotion &&
                    stillActive)
                {
                    summary.negativeAccelerationCandidates++;
                }
            }
        }
    }

    return summary;
}

double BenchmarkAnalyzer::ticksToSeconds(
    long long ticks,
    long long qpcFrequency) const
{
    if (qpcFrequency <= 0)
    {
        return 0.0;
    }

    return static_cast<double>(ticks) /
           static_cast<double>(qpcFrequency);
}

double BenchmarkAnalyzer::sampleMagnitude(
    const MouseSample& sample) const
{
    return std::hypot(
        static_cast<double>(sample.dx),
        static_cast<double>(sample.dy)
    );
}

double BenchmarkAnalyzer::median(
    std::vector<double> values) const
{
    if (values.empty())
    {
        return 0.0;
    }

    std::sort(
        values.begin(),
        values.end()
    );

    const std::size_t middle =
        values.size() / 2;

    if (values.size() % 2 == 1)
    {
        return values[middle];
    }

    return (
        values[middle - 1] +
        values[middle]
    ) / 2.0;
}
