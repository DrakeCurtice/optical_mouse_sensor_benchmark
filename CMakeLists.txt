cmake_minimum_required(VERSION 3.20)

project(MouseSensorBenchmark LANGUAGES CXX)

add_executable(mouse_benchmark
    main.cpp
    BenchmarkConfig.cpp
    BenchmarkApp.cpp
    BenchmarkAnalyzer.cpp
    RawInputCapture.cpp
    TcpClient.cpp
    HidInspector.cpp
)

target_compile_features(
    mouse_benchmark
    PRIVATE
    cxx_std_17
)

target_link_libraries(
    mouse_benchmark
    PRIVATE
    ws2_32
    user32
    gdi32
    hid
)