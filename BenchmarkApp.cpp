#include "BenchmarkApp.h"

#include "BenchmarkAnalyzer.h"
#include "RawInputCapture.h"
#include "TcpClient.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

BenchmarkApp::BenchmarkApp(BenchmarkConfig config)
    : config_(std::move(config))
{
}

int BenchmarkApp::run()
{
    printBanner();

    // Create local object of RawInputCapture class and give it the current config.
    RawInputCapture capture(config_);

    if (!capture.run())
    {
        std::cerr << "Capture failed.\n";
        return 1;
    }

    BenchmarkAnalyzer analyzer(config_);

    const BenchmarkSummary summary = analyzer.analyze(
        capture.samples(),
        capture.qpcFrequency(),
        capture.captureSeconds()
    );

    printSummary(summary);

    const std::string json =
        makeJson(summary, capture.devicePath());

    TcpClient tcpClient;

    if (tcpClient.sendJson(json))
    {
        std::cout
            << "\nSaved through TCP backend on 127.0.0.1:9000.\n";
    }
    else
    {
        std::cout
            << "\nBackend not running; benchmark completed locally.\n";
    }

    return 0;
}

void BenchmarkApp::printBanner() const
{
    std::cout
        << "Optical Mouse Sensor Benchmark\n"
        << "DPI: " << config_.dpi
        << " | Polling cap: " << config_.pollingHz << " Hz"
        << " | Duration: " << config_.durationSeconds << " s\n"
        << "IMPORTANT: keep the benchmark window focused.\n";

    if (config_.distanceInches > 0.0)
    {
        std::cout
            << "DPI test: move exactly "
            << config_.distanceInches
            << " inches horizontally in ONE direction.\n";
    }
    else
    {
        std::cout
            << "Polling test: use fast continuous left/right sweeps.\n";
    }
}

void BenchmarkApp::printSummary(
    const BenchmarkSummary& summary) const
{
    std::cout << std::fixed << std::setprecision(2);

    std::cout
        << "\nBENCHMARK SUMMARY\n"
        << "-----------------\n"
        << "Configured polling cap:       " << config_.pollingHz << " Hz\n"
        << "Samples captured:             " << summary.sampleCount << "\n"
        << "Motion samples:               " << summary.motionSamples << "\n"
        << "Zero-motion samples:          " << summary.zeroMotionSamples << "\n"
        << "Average observed event rate:  " << summary.averageEventRateHz << " Hz\n"
        << "Estimated active polling:     " << summary.estimatedPollingRateHz << " Hz\n"
        << "Estimated / configured cap:   " << summary.pollingRatePercent << " %\n"
        << "Mean active interval:         " << summary.meanActiveIntervalMs << " ms\n"
        << "Median active interval:       " << summary.medianActiveIntervalMs << " ms\n"
        << "Active interval stddev:       " << summary.intervalStdDevMs * 1000.0 << " us\n"
        << "Active interval variance:     " << summary.intervalVarianceMs2 << " ms^2\n";

    if (summary.hasMeasuredDpi)
        std::cout << "Measured DPI:                 " << summary.measuredDpi << "\n";
    else
        std::cout << "Measured DPI:                 not measured\n";

    std::cout
        << "Peak observed speed:          " << summary.peakObservedIps << " IPS\n"
        << "Raw Input gap candidates:     " << summary.rawInputGapCandidates << "\n"
        << "Tracking-loss candidates:     " << summary.trackingLossCandidates << "\n"
        << "Spinout candidates:           " << summary.spinoutCandidates << "\n"
        << "Negative-accel candidates:    " << summary.negativeAccelerationCandidates << "\n"
        << "Missing HID sequence gaps:    requires USB/report counter\n";
}

std::string BenchmarkApp::makeJson(
    const BenchmarkSummary& summary,
    const std::wstring& devicePath) const
{
    std::ostringstream json;
    json << std::fixed << std::setprecision(4);

    json
        << "{"
        << "\"device_path\":\"" << jsonEscape(toUtf8(devicePath)) << "\","
        << "\"configured_dpi\":" << config_.dpi << ","
        << "\"configured_polling_hz\":" << config_.pollingHz << ","
        << "\"sample_count\":" << summary.sampleCount << ","
        << "\"motion_sample_count\":" << summary.motionSamples << ","
        << "\"zero_motion_sample_count\":" << summary.zeroMotionSamples << ","
        << "\"average_event_rate_hz\":" << summary.averageEventRateHz << ","
        << "\"estimated_polling_rate_hz\":" << summary.estimatedPollingRateHz << ","
        << "\"polling_rate_percent\":" << summary.pollingRatePercent << ","
        << "\"mean_active_interval_ms\":" << summary.meanActiveIntervalMs << ","
        << "\"median_active_interval_ms\":" << summary.medianActiveIntervalMs << ","
        << "\"active_interval_stddev_ms\":" << summary.intervalStdDevMs << ","
        << "\"active_interval_variance_ms2\":" << summary.intervalVarianceMs2 << ",";

    if (summary.hasMeasuredDpi)
        json << "\"measured_dpi\":" << summary.measuredDpi << ",";
    else
        json << "\"measured_dpi\":null,";

    json
        << "\"peak_observed_ips\":" << summary.peakObservedIps << ","
        << "\"raw_input_gap_candidates\":" << summary.rawInputGapCandidates << ","
        << "\"tracking_loss_candidates\":" << summary.trackingLossCandidates << ","
        << "\"spinout_candidates\":" << summary.spinoutCandidates << ","
        << "\"negative_acceleration_candidates\":"
        << summary.negativeAccelerationCandidates << ","
        << "\"missing_hid_sequence_supported\":false"
        << "}";

    return json.str();
}

std::string BenchmarkApp::toUtf8(
    const std::wstring& value) const
{
    if (value.empty())
    {
        return "";
    }

    const int required = WideCharToMultiByte(
        CP_UTF8,
        0,
        value.data(),
        static_cast<int>(value.size()),
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (required <= 0)
    {
        return "";
    }

    std::string result(required, '\0');

    WideCharToMultiByte(
        CP_UTF8,
        0,
        value.data(),
        static_cast<int>(value.size()),
        result.data(),
        required,
        nullptr,
        nullptr
    );

    return result;
}

std::string BenchmarkApp::jsonEscape(
    const std::string& value) const
{
    std::ostringstream output;

    for (char character : value)
    {
        switch (character)
        {
            case '\\': output << "\\\\"; break;
            case '"':  output << "\\\""; break;
            case '\n': output << "\\n"; break;
            case '\r': output << "\\r"; break;
            case '\t': output << "\\t"; break;
            default:   output << character; break;
        }
    }

    return output.str();
}
