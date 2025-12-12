// #include "cucumber_cpp/library/TraceTime.hpp"
// #include <chrono>

// namespace cucumber_cpp::library
// {
//     void TraceTime::Start()
//     {
//         timeStart = std::chrono::high_resolution_clock::now();
//     }

//     void TraceTime::Stop()
//     {
//         timeStop = std::chrono::high_resolution_clock::now();
//     }

//     TraceTime::Duration TraceTime::Delta() const
//     {
//         if (timeStop != TimePoint{})
//             return timeStop - timeStart;

//         return std::chrono::high_resolution_clock::now() - timeStart;
//     }
// }
