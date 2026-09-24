#include "BenchmarkApp.h"
#include "BenchmarkConfig.h"

int main(int argc, char* argv[])
{
    const BenchmarkConfig config =
        parseBenchmarkArguments(argc, argv);

    BenchmarkApp app(config);
    return app.run();
}

        // Every event is stored as a MouseSample sample;
        // Has fields like ticks, dx, and dy

        // We push the samples into a vector of MouseSamples

        // 10 seconds looks like somehting like 9548 ticks with tiemestamp and dx dy value

        // 9549 MouseSample objects


        // Why do we use a vector????

        // samples arrive in order
        // we mostly append
        // we know roughly how many you'll get
        // afterwards we scan them sequentially
        // we occasionally need indexing

        // O(1) amortized push_back
        // O(1) random access
        // contiguous memory
        // good cache locality
        // easy sequential iteration

        // Allocate memory in vector upfront because we know roughly how much memory we need 
        // so we allocate before timing sensitive part starts

        // After 10 seconds our capture is over and we call this:
        // BenchmarkAnalyzer analyzer(config_);

        // CAPTURE DATA
        // Do barely any work
        // STORE DATA
        // Finish capture and store
        // DO ANALYSIS ON DATA



        // CALCULATIONS!!!!!

        // Average HZ over entire 10 seconds is counts 9549 / 10 seconds = 954 HZ

        // Convert Ticks from QueryPerformanceCounter() to seconds by difference * frequency
        // Build a std::vector<double> activeIntervalsSeconds


        // FILTER INTERVALS
        // If interval <= expectedInterval * 2.5)
        //      activeIntervals.push_back(interval)

        // Dont include interval if there is more ms between them so we ignore some slow events that could distort our estimate

        // Threshold to separate normal active timing distribution from obvious long gaps



        // Take Median for HZ so the big gap doesnt mess with it. 


        

        // HOW DO WE DO IPS CALCULATION????


        // WHAT IS OUR SLIDING WINDOW???? 

        // HOW TO FIND TOTAL MOVEMENT INSDIE WINDOW ???? 

        // HOW TO CALCULATE 2D MOVEMENT DISPALCEMENT 


        // HOW TO TURN THIS TO INCHES ?????

        // HOW WE GET ACCURATE INCHES PER SECOND

            // Raw Input reports
            //         ↓
            //     (dx, dy) each ~1 ms
            //         ↓
            //     take ~10 ms worth
            //         ↓
            //     SUM dx and SUM dy
            //         ↓
            //     hypot(totalDx,totalDy)
            //         ↓
            //     net displacement in counts
            //         ↓
            //     divide by DPI
            //         ↓
            //     net inches moved
            //         ↓
            //     divide by elapsed seconds
            //         ↓
            //     IPS



        // WHAT DATASTRUCTURES 

        // WHY NOT COMPUTE METRICS IN REAL TIME IF ITS FAST
        
        // CONSIDER RING BUFFER AND ROLLING STUFF












        // 1. Register mouse for Raw Input.

        // 2. Windows puts WM_INPUT messages into my thread's queue.

        // 3. My PeekMessage loop removes and dispatches pending messages.

        // 4. For each WM_INPUT:
        //     QueryPerformanceCounter()
        //     GetRawInputData()
        //     extract dx/dy
        //     construct MouseSample
        //     push_back into vector

        // 5. After duration expires:
        //     stop capture.

        // 6. Analyzer reads vector<MouseSample> by const reference.

        // 7. For polling:
        //     adjacent timestamps
        //     → time intervals
        //     → filter active intervals
        //     → median interval
        //     → 1 / interval = estimated Hz

        // 8. For IPS:
        //     dx/dy samples
        //     → prefix sums
        //     → sliding 10 ms window
        //     → displacement counts
        //     → counts / DPI = inches
        //     → inches / seconds = IPS
        //     → retain maximum observed value


        