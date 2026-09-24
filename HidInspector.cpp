#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <hidsdi.h>
#include <hidusage.h>
#include <hidpi.h>

#include "HidInspector.h"

#include <iostream>
#include <string>
#include <vector>


namespace
{

std::wstring getDevicePath(HANDLE deviceHandle)
{
    UINT characterCount = 0;

    GetRawInputDeviceInfoW(
        deviceHandle,
        RIDI_DEVICENAME,
        nullptr,
        &characterCount
    );

    if (characterCount == 0)
    {
        return L"";
    }

    std::vector<wchar_t> buffer(characterCount + 1, L'\0');

    UINT result = GetRawInputDeviceInfoW(
        deviceHandle,
        RIDI_DEVICENAME,
        buffer.data(),
        &characterCount
    );

    if (result == static_cast<UINT>(-1))
    {
        return L"";
    }

    return std::wstring(buffer.data());
}


void printValueCaps(
    PHIDP_PREPARSED_DATA preparsedData,
    const HIDP_CAPS& caps)
{
    USHORT count = caps.NumberInputValueCaps;

    if (count == 0)
    {
        std::cout << "\nNo input value fields.\n";
        return;
    }

    std::vector<HIDP_VALUE_CAPS> values(count);

    NTSTATUS status = HidP_GetValueCaps(
        HidP_Input,
        values.data(),
        &count,
        preparsedData
    );

    if (status != HIDP_STATUS_SUCCESS)
    {
        std::cout << "HidP_GetValueCaps failed.\n";
        return;
    }

    std::cout << "\nVALUE FIELDS\n";
    std::cout << "------------\n";

    for (USHORT index = 0; index < count; ++index)
    {
        const HIDP_VALUE_CAPS& value = values[index];

        std::cout
            << "Field " << index << "\n"

            << "  Report ID:     "
            << static_cast<int>(value.ReportID)
            << "\n"

            << "  Usage page:    0x"
            << std::hex
            << value.UsagePage
            << std::dec
            << "\n";

        if (value.IsRange)
        {
            std::cout
                << "  Usage range:   0x"
                << std::hex
                << value.Range.UsageMin
                << " - 0x"
                << value.Range.UsageMax
                << std::dec
                << "\n";
        }
        else
        {
            std::cout
                << "  Usage:         0x"
                << std::hex
                << value.NotRange.Usage
                << std::dec
                << "\n";
        }

        std::cout
            << "  Bit size:      "
            << value.BitSize
            << "\n"

            << "  Report count:  "
            << value.ReportCount
            << "\n"

            << "  Logical min:   "
            << value.LogicalMin
            << "\n"

            << "  Logical max:   "
            << value.LogicalMax
            << "\n"

            << "  Relative:      "
            << (value.IsAbsolute ? "No" : "Yes")
            << "\n\n";
    }
}


void printButtonCaps(
    PHIDP_PREPARSED_DATA preparsedData,
    const HIDP_CAPS& caps)
{
    USHORT count = caps.NumberInputButtonCaps;

    if (count == 0)
    {
        std::cout << "\nNo input button fields.\n";
        return;
    }

    std::vector<HIDP_BUTTON_CAPS> buttons(count);

    NTSTATUS status = HidP_GetButtonCaps(
        HidP_Input,
        buttons.data(),
        &count,
        preparsedData
    );

    if (status != HIDP_STATUS_SUCCESS)
    {
        std::cout << "HidP_GetButtonCaps failed.\n";
        return;
    }

    std::cout << "\nBUTTON FIELDS\n";
    std::cout << "-------------\n";

    for (USHORT index = 0; index < count; ++index)
    {
        const HIDP_BUTTON_CAPS& button = buttons[index];

        std::cout
            << "Field " << index << "\n"

            << "  Report ID:     "
            << static_cast<int>(button.ReportID)
            << "\n"

            << "  Usage page:    0x"
            << std::hex
            << button.UsagePage
            << std::dec
            << "\n";

        if (button.IsRange)
        {
            std::cout
                << "  Usage range:   "
                << button.Range.UsageMin
                << " - "
                << button.Range.UsageMax
                << "\n";
        }
        else
        {
            std::cout
                << "  Usage:         "
                << button.NotRange.Usage
                << "\n";
        }

        std::cout << "\n";
    }
}


void inspectMouse(HANDLE deviceHandle)
{
    const std::wstring devicePath =
        getDevicePath(deviceHandle);

    if (devicePath.empty())
    {
        std::cout << "Could not get device path.\n";
        return;
    }

    std::wcout
        << L"\n==================================================\n"
        << L"DEVICE PATH\n"
        << L"==================================================\n"
        << devicePath
        << L"\n";

    HANDLE hidHandle = CreateFileW(
        devicePath.c_str(),
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (hidHandle == INVALID_HANDLE_VALUE)
    {
        std::cout
            << "CreateFileW failed. Error: "
            << GetLastError()
            << "\n";

        return;
    }

    PHIDP_PREPARSED_DATA preparsedData = nullptr;

    if (!HidD_GetPreparsedData(
        hidHandle,
        &preparsedData))
    {
        std::cout
            << "HidD_GetPreparsedData failed. Error: "
            << GetLastError()
            << "\n";

        CloseHandle(hidHandle);
        return;
    }

    HIDP_CAPS caps{};

    NTSTATUS status =
        HidP_GetCaps(
            preparsedData,
            &caps
        );

    if (status != HIDP_STATUS_SUCCESS)
    {
        std::cout << "HidP_GetCaps failed.\n";

        HidD_FreePreparsedData(preparsedData);
        CloseHandle(hidHandle);
        return;
    }

    std::cout
        << "\nHID CAPABILITIES\n"
        << "----------------\n"

        << "Usage page:            0x"
        << std::hex
        << caps.UsagePage
        << std::dec
        << "\n"

        << "Usage:                 0x"
        << std::hex
        << caps.Usage
        << std::dec
        << "\n"

        << "Input report length:   "
        << caps.InputReportByteLength
        << " bytes\n"

        << "Output report length:  "
        << caps.OutputReportByteLength
        << " bytes\n"

        << "Feature report length: "
        << caps.FeatureReportByteLength
        << " bytes\n"

        << "Input value fields:    "
        << caps.NumberInputValueCaps
        << "\n"

        << "Input button fields:   "
        << caps.NumberInputButtonCaps
        << "\n";

    printValueCaps(preparsedData, caps);
    printButtonCaps(preparsedData, caps);

    HidD_FreePreparsedData(preparsedData);
    CloseHandle(hidHandle);
}

}


int runHidInspector()
{
    UINT deviceCount = 0;

    UINT result = GetRawInputDeviceList(
        nullptr,
        &deviceCount,
        sizeof(RAWINPUTDEVICELIST)
    );

    if (result == static_cast<UINT>(-1))
    {
        std::cerr
            << "Failed to query Raw Input device count.\n";
        return 1;
    }

    if (deviceCount == 0)
    {
        std::cout << "No Raw Input devices found.\n";
        return 0;
    }

    std::vector<RAWINPUTDEVICELIST> devices(deviceCount);

    result = GetRawInputDeviceList(
        devices.data(),
        &deviceCount,
        sizeof(RAWINPUTDEVICELIST)
    );

    if (result == static_cast<UINT>(-1))
    {
        std::cerr
            << "Failed to get Raw Input device list.\n";
        return 1;
    }

    std::cout
        << "Found "
        << deviceCount
        << " Raw Input devices.\n";

    int mouseCount = 0;

    for (const RAWINPUTDEVICELIST& device : devices)
    {
        if (device.dwType != RIM_TYPEMOUSE)
        {
            continue;
        }

        ++mouseCount;

        std::cout
            << "\nInspecting mouse "
            << mouseCount
            << "...\n";

        inspectMouse(device.hDevice);
    }

    std::cout
        << "\nMouse devices inspected: "
        << mouseCount
        << "\n";

    return 0;
}