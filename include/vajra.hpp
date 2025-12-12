#ifndef VAJRA_HPP
#define VAJRA_HPP

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

#ifdef __linux__
#include <fstream>
#include <sstream>
#include <sys/resource.h>
#endif

#ifdef _WIN32
// clang-format off
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <psapi.h>
// clang-format on
#endif

namespace Statistics {

/**
 * @brief Concept to constrain types to numeric types (integral and floating-point).
 * @tparam T The type to be checked.
 */
template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

/**
 * @brief Calculate the mean (average) of a vector of numeric values.
 * @tparam T The numeric type of the values.
 * @param values The vector of numeric values.
 * @return The mean of the values as a double.
 */
template <Numeric T>
inline double mean(const std::vector<T>& values)
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
{
    if (values.empty())
        return 0.0;

    const double sum{std::accumulate(values.begin(), values.end(), 0.0,
                                     [](double acc, T v) { return acc + static_cast<double>(v); })};

    return sum / static_cast<double>(values.size());
}

/**
 * @brief Calculate the median of a vector of numeric values.
 * @tparam T The numeric type of the values.
 * @param values The vector of numeric values.
 * @return The median of the values as a double.
 */
template <Numeric T>
inline double median(std::vector<T> values)
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
{
    if (values.empty())
        return 0.0;

    std::sort(values.begin(), values.end());

    const std::size_t count{values.size()};
    const std::size_t mid{count / 2};

    if ((count & 1) == 0) {
        return (static_cast<double>(values[mid - 1]) + static_cast<double>(values[mid])) * 0.5;
    }

    return static_cast<double>(values[mid]);
}

/**
 * @brief Calculate the variance of a vector of numeric values.
 * @tparam T The numeric type of the values.
 * @param values The vector of numeric values.
 * @return The variance of the values as a double.
 */
template <Numeric T>
inline double variance(const std::vector<T>& values)
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
{
    if (values.size() < 2)
        return 0.0;

    const double avg{mean(values)};
    double var{0.0};

    for (const T v : values) {
        const double diff{static_cast<double>(v) - avg};
        var += diff * diff;
    }

    return var / static_cast<double>(values.size());
}

/**
 * @brief Calculate the standard deviation of a vector of numeric values.
 * @tparam T The numeric type of the values.
 * @param values The vector of numeric values.
 * @return The standard deviation of the values as a double.
 */
template <Numeric T>
inline double stddev(const std::vector<T>& values)
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
{
    if (values.size() < 2)
        return 0.0;

    return std::sqrt(variance(values));
}

/**
 * @brief Find the minimum value in a vector of numeric values.
 * @tparam T The numeric type of the values.
 * @param values The vector of numeric values.
 * @return The minimum value in the vector.
 */
template <Numeric T> inline T min(const std::vector<T>& values) {
    if (values.empty())
        return T{};

    return *std::min_element(values.begin(), values.end());
}

/**
 * @brief Find the maximum value in a vector of numeric values.
 * @tparam T The numeric type of the values.
 * @param values The vector of numeric values.
 * @return The maximum value in the vector.
 */
template <Numeric T> inline T max(const std::vector<T>& values) {
    if (values.empty())
        return T{};

    return *std::max_element(values.begin(), values.end());
}

/**
 * @brief Calculate the percentile of a vector of numeric values.
 * @tparam T The numeric type of the values.
 * @param values The vector of numeric values.
 * @param p The percentile (0-100).
 * @return The percentile value as a double.
 */
template <Numeric T>
inline double percentile(std::vector<T> values, double p)
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
{
    if (values.empty())
        return 0.0;

    if (p < 0.0)
        p = 0.0;
    if (p > 100.0)
        p = 100.0;

    std::sort(values.begin(), values.end());

    const double index{(p / 100.0) * (values.size() - 1)};
    const size_t lower{static_cast<size_t>(std::floor(index))};
    const size_t upper{static_cast<size_t>(std::ceil(index))};

    if (lower == upper) {
        return static_cast<double>(values[lower]);
    }

    const double weight{index - lower};
    return static_cast<double>(values[lower]) * (1.0 - weight) +
           static_cast<double>(values[upper]) * weight;
}

/**
 * @brief Calculate the range of a vector of numeric values.
 * @tparam T The numeric type of the values.
 * @param values The vector of numeric values.
 * @return The range (max - min) of the values.
 */
template <Numeric T> inline T range(const std::vector<T>& values) {
    if (values.empty())
        return T{};

    return max(values) - min(values);
}

/**
 * @brief Calculate the sum of a vector of numeric values.
 * @tparam T The numeric type of the values.
 * @param values The vector of numeric values.
 * @return The sum of the values as a double.
 */
template <Numeric T>
inline double sum(const std::vector<T>& values)
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
{
    return std::accumulate(values.begin(), values.end(), 0.0,
                           [](double acc, T v) { return acc + static_cast<double>(v); });
}

} // namespace Statistics

namespace Timer {

/**
 * @brief Type alias for high-resolution clock
 */
using Clock = std::chrono::high_resolution_clock;
/**
 * @brief Type alias for time points based on the high-resolution clock
 */
using TimePoint = std::chrono::time_point<Clock>;

/**
 * @brief A simple timer class for measuring elapsed time.
 */
class Timer {
  private:
    TimePoint startTime{};
    TimePoint endTime{};
    bool running{false};
    bool started{false};
    std::string name{"Timer"};

  public:
    /**
     * @brief Construct a new Timer object.
     * @param timerName The name of the timer (default: "Timer").
     */
    Timer(const std::string& timerName = "Timer") : name{timerName} {}

    /**
     * @brief Start the timer.
     */
    void start() {
        startTime = Clock::now();
        running = true;
        started = true;
    }

    /**
     * @brief Stop the timer.
     */
    void stop() {
        endTime = Clock::now();
        running = false;
    }

    /**
     * @brief Get the elapsed time in seconds.
     * @return Elapsed time in seconds.
     */
    double elapsedSeconds() const {
        if (!started)
            return 0.0;

        TimePoint end{running ? Clock::now() : endTime};
        auto duration{std::chrono::duration_cast<std::chrono::nanoseconds>(end - startTime)};
        return static_cast<double>(duration.count()) / 1e9;
    }

    /**
     * @brief Get the elapsed time in milliseconds.
     * @return Elapsed time in milliseconds.
     */
    double elapsedMilliseconds() const {
        return elapsedSeconds() * 1000.0;
    }

    /**
     * @brief Get the elapsed time in microseconds.
     * @return Elapsed time in microseconds.
     */
    double elapsedMicroseconds() const {
        return elapsedSeconds() * 1e6;
    }

    /**
     * @brief Get the elapsed time in nanoseconds.
     * @return Elapsed time in nanoseconds.
     */
    double elapsedNanoseconds() const {
        if (!started)
            return 0.0;

        TimePoint end{running ? Clock::now() : endTime};
        auto duration{std::chrono::duration_cast<std::chrono::nanoseconds>(end - startTime)};
        return static_cast<double>(duration.count());
    }

    /**
     * @brief Get the name of the timer.
     * @return The name of the timer as a string.
     */
    std::string getName() const {
        return name;
    }

    /**
     * @brief Reset the timer.
     */
    void reset() {
        running = false;
        started = false;
    }

    /**
     * @brief Check if the timer is currently running.
     * @return True if the timer is running, false otherwise.
     */
    bool isRunning() const {
        return running;
    }
};

/**
 * @brief Automatic timer that starts on construction and stops on destruction
 */
class ScopedTimer {
  private:
    Timer timer{};
    bool printOnDestroy{true};

  public:
    /**
     * @brief Construct a new Scoped Timer object and start timing.
     * @param name The name of the timer.
     * @param print Whether to print the elapsed time on destruction.
     */
    ScopedTimer(const std::string& name = "ScopedTimer", bool print = true)
        : timer{name}, printOnDestroy{print} {
        timer.start();
    }

    /**
     * @brief Destroy the Scoped Timer object and optionally print elapsed time.
     */
    ~ScopedTimer() {
        timer.stop();
        if (printOnDestroy) {
            std::cout << timer.getName() << ": " << std::fixed << std::setprecision(6)
                      << timer.elapsedSeconds() << "s" << std::endl;
        }
    }

    /**
     * @brief Get the elapsed time in seconds.
     * @return Elapsed time in seconds.
     */
    double elapsedSeconds() const {
        return timer.elapsedSeconds();
    }
};

} // namespace Timer

namespace Memory {

/**
 * @brief Memory usage information
 */
struct MemoryInfo {
    /**
     * @brief Peak Resident Set Size in Kilobytes
     */
    size_t peakRssKb{};
    /**
     * @brief Current Resident Set Size in Kilobytes
     */
    size_t currentRssKb{};

    /**
     * @brief Default constructor for MemoryInfo.
     */
    MemoryInfo() = default;
};

/**
 * @brief Get memory usage information
 * @return MemoryInfo struct containing peak and current RSS in KB
 */
inline MemoryInfo getMemoryInfo() {
    MemoryInfo info{};
#ifdef __linux__
    rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        info.peakRssKb = static_cast<size_t>(usage.ru_maxrss);
    }

    std::ifstream status{"/proc/self/status"};
    std::string line{};

    while (std::getline(status, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            std::istringstream iss{line.substr(6)};
            iss >> info.currentRssKb;
            break;
        }
    }
#elif defined(_WIN32)
    PROCESS_MEMORY_COUNTERS_EX pmc{};
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        info.peakRssKb = pmc.PeakWorkingSetSize / 1024;
        info.currentRssKb = pmc.WorkingSetSize / 1024;
    }
#endif
    return info;
}

/**
 * @brief Format memory size from kilobytes to a human-readable string.
 * @param kb Memory size in kilobytes.
 * @return Formatted memory size string.
 */
inline std::string formatMemory(size_t kb) {
    std::ostringstream oss{};
    oss << std::fixed << std::setprecision(2);

    if (kb < 1024) {
        oss << kb << " KB";
    } else if (kb < 1024 * 1024) {
        oss << (kb / 1024.0) << " MB";
    } else {
        oss << (kb / (1024.0 * 1024.0)) << " GB";
    }

    return oss.str();
}

} // namespace Memory

namespace Profiling {

/**
 * Performance measurement result
 */
struct PerfResult {
    /**
     * @brief Name of the measurement
     */
    std::string name{};

    /**
     * @brief Elapsed time in seconds
     */
    double elapsedSeconds{};
    /**
     * @brief Memory usage information
     */
    Memory::MemoryInfo memoryInfo{};
    /**
     * @brief Custom metrics collected during profiling
     */
    std::map<std::string, double> customMetrics{};

    /**
     * @brief Construct a new PerfResult object.
     * @param n The name of the measurement.
     */
    PerfResult(const std::string& n = "") : name{n}, elapsedSeconds{0.0} {}
};

/**
 * @brief Performance profiler for advanced profiling
 */
class Profiler {
  private:
    std::map<std::string, std::vector<double>> timingData{};
    std::map<std::string, Timer::Timer> activeTimers{};
    Memory::MemoryInfo initialMemory{};

  public:
    /**
     * @brief Construct a new Profiler object and capture initial memory state.
     */
    Profiler() {
        initialMemory = Memory::getMemoryInfo();
    }

    /**
     * @brief Start timing a section
     * @param sectionName Name of the section to time
     */
    void start(const std::string& sectionName) {
        activeTimers[sectionName] = Timer::Timer{sectionName};
        activeTimers[sectionName].start();
    }

    /**
     * @brief Stop timing a section
     * @param sectionName Name of the section to stop timing
     */
    void stop(const std::string& sectionName) {
        auto it{activeTimers.find(sectionName)};
        if (it != activeTimers.end()) {
            it->second.stop();
            timingData[sectionName].push_back(it->second.elapsedSeconds());
            activeTimers.erase(it);
        }
    }

    /**
     * @brief Add timing data for a section
     * @param sectionName Name of the section
     * @param seconds Elapsed time in seconds
     */
    void addTiming(const std::string& sectionName, double seconds) {
        timingData[sectionName].push_back(seconds);
    }

    /**
     * @brief Measure the performance of a function
     * @param name Name of the measurement
     * @param func Function to measure
     * @return PerfResult containing measurement results
     */
    PerfResult measure(const std::string& name, std::function<void()> func) {
        PerfResult result{name};
        Timer::Timer timer{name};

        timer.start();
        func();
        timer.stop();
        Memory::MemoryInfo memAfter{Memory::getMemoryInfo()};

        result.elapsedSeconds = timer.elapsedSeconds();
        result.memoryInfo = memAfter;

        return result;
    }

    /**
     * @brief Get the collected timing data
     * @return Map of section names to vectors of elapsed times
     */
    const std::map<std::string, std::vector<double>>& getTimingData() const {
        return timingData;
    }

    /**
     * @brief Clear all collected timing data
     */
    void clear() {
        timingData.clear();
        activeTimers.clear();
    }
};

} // namespace Profiling

/**
 * @brief Benchmark class for running multiple iterations
 */
class Benchmark {
  private:
    std::string name;
    size_t iterations;
    size_t warmupIterations;

  public:
    /**
     * @brief Construct a new Benchmark object.
     * @param benchName The name of the benchmark.
     * @param numIterations The number of iterations to run (default: 100).
     * @param warmup The number of warmup iterations (default: 10).
     */
    Benchmark(const std::string& benchName, size_t numIterations = 100, size_t warmup = 10)
        : name(benchName), iterations(numIterations), warmupIterations(warmup) {}

    /**
     * @brief Run the benchmark with the provided function.
     * @tparam Func The type of the function to benchmark.
     * @param func The function to benchmark.
     * @return Vector of elapsed times in seconds for each iteration.
     */
    template <typename Func> std::vector<double> run(Func func) {
        for (size_t i{0}; i < warmupIterations; ++i) {
            func();
        }

        std::vector<double> times;
        times.reserve(iterations);

        for (size_t i{0}; i < iterations; ++i) {
            Timer::Timer timer;
            timer.start();
            func();
            timer.stop();
            times.push_back(timer.elapsedSeconds());
        }

        return times;
    }

    /**
     * @brief Print statistical summary of benchmark results.
     * @param times Vector of elapsed times from benchmark runs.
     */
    void printStats(const std::vector<double>& times) const {
        if (times.empty()) {
            std::cout << "No timing data available" << std::endl;
            return;
        }

        std::cout << "\n=== " << name << " Results ===" << std::endl;
        std::cout << std::fixed << std::setprecision(6);
        std::cout << "Iterations: " << times.size() << std::endl;
        std::cout << "Mean:       " << Statistics::mean(times) << "s" << std::endl;
        std::cout << "Median:     " << Statistics::median(times) << "s" << std::endl;
        std::cout << "Min:        " << Statistics::min(times) << "s" << std::endl;
        std::cout << "Max:        " << Statistics::max(times) << "s" << std::endl;
        std::cout << "Std Dev:    " << Statistics::stddev(times) << "s" << std::endl;
        std::cout << "P95:        " << Statistics::percentile(times, 95.0) << "s" << std::endl;
        std::cout << "P99:        " << Statistics::percentile(times, 99.0) << "s" << std::endl;
    }
};

#endif // VAJRA_HPP