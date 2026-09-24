#include "BenchmarkConfig.h"

#include <cstdlib>
#include <iostream>
#include <string>

BenchmarkConfig parseBenchmarkArguments(int argc, char* argv[])
{
    BenchmarkConfig config;

    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];

        auto requireValue = [&](const std::string& flag)
        {
            if (index + 1 >= argc)
            {
                std::cerr << "Missing value for " << flag << "\n";
                std::exit(1);
            }

            return std::string(argv[++index]);
        };

        if (argument == "--dpi")
            config.dpi = std::stoi(requireValue(argument));
        else if (argument == "--polling")
            config.pollingHz = std::stoi(requireValue(argument));
        else if (argument == "--duration")
            config.durationSeconds = std::stoi(requireValue(argument));
        else if (argument == "--distance")
            config.distanceInches = std::stod(requireValue(argument));
        else if (argument == "--speed-window-ms")
            config.speedWindowMs = std::stoi(requireValue(argument));
        else if (argument == "--gap-multiplier")
            config.gapMultiplier = std::stod(requireValue(argument));
        else if (argument == "--tracking-gap-multiplier")
            config.trackingGapMultiplier = std::stod(requireValue(argument));
        else if (argument == "--spinout-multiplier")
            config.spinoutMagnitudeMultiplier = std::stod(requireValue(argument));
        else if (argument == "--negative-accel-drop-ratio")
            config.negativeAccelDropRatio = std::stod(requireValue(argument));
        else
        {
            std::cerr << "Unknown argument: " << argument << "\n";
            std::exit(1);
        }
    }

    if (config.dpi <= 0 ||
        config.pollingHz <= 0 ||
        config.durationSeconds <= 0 ||
        config.speedWindowMs <= 0)
    {
        std::cerr
            << "DPI, polling rate, duration, and speed window must be positive.\n";
        std::exit(1);
    }

    return config;
}
