#pragma once

#include "BenchmarkTypes.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <string>
#include <vector>

class RawInputCapture
{
public:
    explicit RawInputCapture(const BenchmarkConfig& config);

    bool run();

    const std::vector<MouseSample>& samples() const;
    const std::wstring& devicePath() const;

    double captureSeconds() const;
    long long qpcFrequency() const;

private:
    static LRESULT CALLBACK windowProc(
        HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );

    LRESULT handleMessage(
        HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );

    bool createWindow();
    bool registerMouse();
    void captureInput(HRAWINPUT rawInputHandle);
    void paintWindow(HWND hwnd);

    std::wstring queryDevicePath(HANDLE deviceHandle) const;

    BenchmarkConfig config_;

    HWND hwnd_ = nullptr;
    HANDLE targetDevice_ = nullptr;

    LARGE_INTEGER qpcFrequency_{};
    long long startTicks_ = 0;
    long long endTicks_ = 0;

    std::wstring devicePath_;
    std::vector<MouseSample> samples_;

    // Reused so normal capture does not allocate a new byte buffer per event.
    std::vector<BYTE> rawInputBuffer_;
};
