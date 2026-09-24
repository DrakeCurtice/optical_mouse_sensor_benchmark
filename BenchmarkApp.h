#pragma once

#include "BenchmarkTypes.h"

#include <string>

class BenchmarkApp
{
public:
    explicit BenchmarkApp(BenchmarkConfig config);

    int run();

private:
    void printBanner() const;
    void printSummary(const BenchmarkSummary& summary) const;

    std::string makeJson(
        const BenchmarkSummary& summary,
        const std::wstring& devicePath
    ) const;

    std::string toUtf8(const std::wstring& value) const;
    std::string jsonEscape(const std::string& value) const;

    BenchmarkConfig config_;
};
