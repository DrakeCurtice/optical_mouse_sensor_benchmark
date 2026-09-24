#include "RawInputCapture.h"

#include <algorithm>
#include <iostream>

namespace
{
constexpr wchar_t kWindowClassName[] = L"MouseBenchmarkRawInputWindow";
constexpr wchar_t kWindowTitle[] = L"Optical Mouse Sensor Benchmark";
}

RawInputCapture::RawInputCapture(const BenchmarkConfig& config)
    : config_(config)
{
}

bool RawInputCapture::run()
{
    if (!QueryPerformanceFrequency(&qpcFrequency_))
    {
        std::cerr << "QueryPerformanceFrequency failed.\n";
        return false;
    }

    if (!createWindow() || !registerMouse())
    {
        return false;
    }

    const std::size_t expectedSamples =
        static_cast<std::size_t>(config_.pollingHz) *
        static_cast<std::size_t>(config_.durationSeconds);

    // Give the sample vector room to grow without repeated reallocations.
    // I know approximately how much memory I'll need, so allocate it before the timing sensitive part starts

    samples_.reserve(expectedSamples * 2);

    LARGE_INTEGER start{};
    QueryPerformanceCounter(&start);
    startTicks_ = start.QuadPart;
    endTicks_ = startTicks_;

    ShowWindow(hwnd_, SW_SHOW);
    UpdateWindow(hwnd_);
    SetForegroundWindow(hwnd_);
    SetFocus(hwnd_);

    bool running = true;

    while (running)
    {
        // Wait for input/messages without busy-spinning.
        MsgWaitForMultipleObjectsEx(
            0,
            nullptr,
            1,
            QS_ALLINPUT,
            MWMO_INPUTAVAILABLE
        );

        MSG message{};

        // PM_REMOVE means to remove from the queue and continue getting messages untul queue is empty
        // NOT THE SAME AS GetRawInputBuffer()

        // GetRawInputBuffer() gives us [event, event, event, event, ...]
        // Good for high frequency devices because multiple events can accumulate between message loop iterators

        // Why not spam GetRawInputBuffer()?
        
        // We do GetRawInputData() BECAUSE
        // Get one WM_INPUT --> one RAW_INPUT
        // Easy to understand and associate processing with a timnestamp
        // BUT is more API/message processing overhead per report.

        // If we aren't fast enough the qeue gets bigger and latency increases and our time measurements are wrong

        // GetRawInputBuffer()
        // Meant to ffetch multiple accumulated events at once. Better throughput when input rate very high
        // DOWNSIDE it does not give original hardware timestamp for each event.
        // Timestamps would just show how quickly we process it BUT NOT WHEN THE MOUSE ACTUALLY PRODUCED TIMESTAMP


        //  In captureInput() we:
        // Use QueryPerformanceCounter() gives us a high resolution counter for measuring intervals, its our timestamp source

        // Returns in Ticks, we have a TicksToSeconds method. 

        while (PeekMessageW(
            &message,
            nullptr,
            0,
            0,
            PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                running = false;
                break;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        LARGE_INTEGER now{};

        // Explain why we use this
        QueryPerformanceCounter(&now);
        endTicks_ = now.QuadPart;

        const double elapsed =
            static_cast<double>(endTicks_ - startTicks_) /
            static_cast<double>(qpcFrequency_.QuadPart);

        if (elapsed >= static_cast<double>(config_.durationSeconds))
        {
            running = false;
        }
    }

    if (hwnd_ != nullptr && IsWindow(hwnd_))
    {
        DestroyWindow(hwnd_);
    }

    return !samples_.empty();
}

const std::vector<MouseSample>& RawInputCapture::samples() const
{
    return samples_;
}

const std::wstring& RawInputCapture::devicePath() const
{
    return devicePath_;
}

double RawInputCapture::captureSeconds() const
{
    if (qpcFrequency_.QuadPart <= 0 || endTicks_ <= startTicks_)
    {
        return 0.0;
    }

    return static_cast<double>(endTicks_ - startTicks_) /
           static_cast<double>(qpcFrequency_.QuadPart);
}

long long RawInputCapture::qpcFrequency() const
{
    return qpcFrequency_.QuadPart;
}

LRESULT CALLBACK RawInputCapture::windowProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    RawInputCapture* capture =
        reinterpret_cast<RawInputCapture*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA)
        );

    if (message == WM_NCCREATE)
    {
        const auto* create =
            reinterpret_cast<const CREATESTRUCTW*>(lParam);

        capture =
            static_cast<RawInputCapture*>(create->lpCreateParams);

        SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(capture)
        );

        capture->hwnd_ = hwnd;
    }

    if (capture != nullptr)
    {
        return capture->handleMessage(
            hwnd,
            message,
            wParam,
            lParam
        );
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT RawInputCapture::handleMessage(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
    {
        case WM_INPUT:
            captureInput(
                reinterpret_cast<HRAWINPUT>(lParam)
            );

            // Required cleanup path for foreground raw input.
            return DefWindowProcW(hwnd, message, wParam, lParam);

        case WM_PAINT:
            paintWindow(hwnd);
            return 0;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            hwnd_ = nullptr;
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}

bool RawInputCapture::createWindow()
{
    HINSTANCE instance = GetModuleHandleW(nullptr);

    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = RawInputCapture::windowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = kWindowClassName;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClassW(&windowClass))
    {
        const DWORD error = GetLastError();

        if (error != ERROR_CLASS_ALREADY_EXISTS)
        {
            std::cerr << "RegisterClassW failed: " << error << "\n";
            return false;
        }
    }

    hwnd_ = CreateWindowExW(
        0,
        kWindowClassName,
        kWindowTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        640,
        220,
        nullptr,
        nullptr,
        instance,
        this
    );

    if (hwnd_ == nullptr)
    {
        std::cerr
            << "CreateWindowExW failed: "
            << GetLastError()
            << "\n";
        return false;
    }

    return true;
}

bool RawInputCapture::registerMouse()
{
    RAWINPUTDEVICE device{};
    device.usUsagePage = 0x01; // Generic Desktop Controls
    device.usUsage = 0x02;     // Mouse
    device.dwFlags = 0;        // Foreground only on purpose
    device.hwndTarget = hwnd_;

    if (!RegisterRawInputDevices(
        &device,
        1,
        sizeof(device)))
    {
        std::cerr
            << "RegisterRawInputDevices failed: "
            << GetLastError()
            << "\n";
        return false;
    }

    return true;
}

void RawInputCapture::captureInput(HRAWINPUT rawInputHandle)
{
    // Timestamp as early as practical in our application-level handler.
    LARGE_INTEGER timestamp{};
    QueryPerformanceCounter(&timestamp);

    UINT size = 0;

    if (GetRawInputData(
        rawInputHandle,
        RID_INPUT,
        nullptr,
        &size,
        sizeof(RAWINPUTHEADER)) != 0)
    {
        return;
    }

    if (size == 0)
    {
        return;
    }

    if (rawInputBuffer_.size() < size)
    {
        rawInputBuffer_.resize(size);
    }

    const UINT bytesRead = GetRawInputData(
        rawInputHandle,
        RID_INPUT,
        rawInputBuffer_.data(),
        &size,
        sizeof(RAWINPUTHEADER)
    );

    if (bytesRead == static_cast<UINT>(-1))
    {
        return;
    }

    const RAWINPUT* input =
        reinterpret_cast<const RAWINPUT*>(
            rawInputBuffer_.data()
        );

    if (input->header.dwType != RIM_TYPEMOUSE)
    {
        return;
    }

    const RAWMOUSE& mouse = input->data.mouse;

    // This benchmark expects relative mouse motion.
    if ((mouse.usFlags & MOUSE_MOVE_ABSOLUTE) != 0)
    {
        return;
    }

    if (targetDevice_ == nullptr)
    {
        targetDevice_ = input->header.hDevice;
        devicePath_ = queryDevicePath(targetDevice_);
    }

    if (input->header.hDevice != targetDevice_)
    {
        return;
    }

    MouseSample sample;
    sample.ticks = timestamp.QuadPart;
    sample.dx = mouse.lLastX;
    sample.dy = mouse.lLastY;
    sample.mouseFlags = mouse.usFlags;
    sample.buttonFlags = mouse.usButtonFlags;
    sample.noCoalesce =
        (mouse.usFlags & MOUSE_MOVE_NOCOALESCE) != 0;

    samples_.push_back(sample);
}

void RawInputCapture::paintWindow(HWND hwnd)
{
    PAINTSTRUCT paint{};
    HDC dc = BeginPaint(hwnd, &paint);

    const wchar_t* line1 =
        L"Keep this window focused during the benchmark.";
    const wchar_t* line2 =
        L"Move the test mouse continuously until the capture ends.";

    TextOutW(
        dc,
        24,
        40,
        line1,
        static_cast<int>(wcslen(line1))
    );

    TextOutW(
        dc,
        24,
        75,
        line2,
        static_cast<int>(wcslen(line2))
    );

    EndPaint(hwnd, &paint);
}

std::wstring RawInputCapture::queryDevicePath(
    HANDLE deviceHandle) const
{
    UINT size = 0;

    GetRawInputDeviceInfoW(
        deviceHandle,
        RIDI_DEVICENAME,
        nullptr,
        &size
    );

    if (size == 0)
    {
        return L"";
    }

    std::vector<wchar_t> buffer(size + 1);

    const UINT result = GetRawInputDeviceInfoW(
        deviceHandle,
        RIDI_DEVICENAME,
        buffer.data(),
        &size
    );

    if (result == static_cast<UINT>(-1))
    {
        return L"";
    }

    return std::wstring(buffer.data());
}
