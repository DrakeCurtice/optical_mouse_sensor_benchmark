CLEAN FILE MAP

Use these files in the benchmark executable:
- main.cpp
- BenchmarkTypes.h
- BenchmarkConfig.h
- BenchmarkConfig.cpp
- BenchmarkApp.h
- BenchmarkApp.cpp
- BenchmarkAnalyzer.h
- BenchmarkAnalyzer.cpp
- RawInputCapture.h
- RawInputCapture.cpp
- TcpClient.h
- TcpClient.cpp

Keep hid_inspector.cpp in a SEPARATE Visual Studio project or exclude it from the benchmark build because it has its own main().

WHAT YOUR OLD FILES ACTUALLY CONTAINED
- benchmark.cpp -> RawInputCapture.h
- BenchmarkAnalyzer.cpp (short class declaration) -> BenchmarkApp.h
- BenchmarkAnalyzer.h -> duplicate TcpClient.cpp
- BenchmarkApp.cpp -> BenchmarkAnalyzer.h
- BenchmarkApp.h -> BenchmarkConfig.h
- BenchmarkConfig.cpp -> hid_inspector.cpp
- BenchmarkConfig.h -> TcpClient.h
- BenchmarkTypes.h -> BenchmarkConfig.cpp
- RawInputCapture.cpp -> main.cpp
- RawInputCapture.h -> BenchmarkTypes.h
- TcpClient.cpp -> already TcpClient.cpp
- TcpClient.h -> BenchmarkApp.cpp
- uploaded 514-line file -> real BenchmarkAnalyzer.cpp

NOTES
- RawInputCapture.cpp was missing from the pasted set, so a clean implementation is included.
- The capture window is foreground-only (no RIDEV_INPUTSINK).
- Raw input byte storage is reused instead of allocating a fresh byte vector for every event.
- This version still uses WM_INPUT/GetRawInputData for per-message application timestamps. It does not claim hardware/USB timestamps.
