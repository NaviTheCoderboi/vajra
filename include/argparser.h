#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace Colors {
const std::string Reset{"\033[0m"};
const std::string Bold{"\033[1m"};
const std::string Dim{"\033[2m"};
const std::string Red{"\033[31m"};
const std::string Green{"\033[32m"};
const std::string Yellow{"\033[33m"};
const std::string Blue{"\033[34m"};
const std::string Magenta{"\033[35m"};
const std::string Cyan{"\033[36m"};
const std::string White{"\033[37m"};
const std::string BrightRed{"\033[91m"};
const std::string BrightGreen{"\033[92m"};
const std::string BrightYellow{"\033[93m"};
const std::string BrightBlue{"\033[94m"};
const std::string BrightMagenta{"\033[95m"};
const std::string BrightCyan{"\033[96m"};
const std::string BrightWhite{"\033[97m"};
} // namespace Colors

class ProgressBar {
  private:
    int total;
    int current;
    int barWidth;
    std::vector<std::string> spinnerFrames;
    int spinnerIndex;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    bool started;

    std::string getRainbowColor(int position, int maxPos) const {
        float ratio{static_cast<float>(position) / maxPos};

        if (ratio < 0.16f)
            return Colors::BrightRed;
        else if (ratio < 0.33f)
            return Colors::BrightYellow;
        else if (ratio < 0.50f)
            return Colors::BrightGreen;
        else if (ratio < 0.66f)
            return Colors::BrightCyan;
        else if (ratio < 0.83f)
            return Colors::BrightBlue;
        else
            return Colors::BrightMagenta;
    }

    std::string formatTime(double seconds) const {
        std::ostringstream oss{};
        if (seconds < 60) {
            oss << std::fixed << std::setprecision(1) << seconds << "s";
        } else if (seconds < 3600) {
            int mins{static_cast<int>(seconds / 60)};
            int secs{static_cast<int>(seconds) % 60};
            oss << mins << "m " << secs << "s";
        } else {
            int hours{static_cast<int>(seconds / 3600)};
            int mins{static_cast<int>(seconds / 60) % 60};
            oss << hours << "h " << mins << "m";
        }
        return oss.str();
    }

  public:
    ProgressBar(int total, int barWidth = 50)
        : total(total), current(0), barWidth(barWidth), spinnerIndex(0), started(false) {
        spinnerFrames = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
    }

    void update(int value) {
        if (!started) {
            startTime = std::chrono::high_resolution_clock::now();
            started = true;
        }

        current = value;
        float progress{static_cast<float>(current) / total};
        int pos{static_cast<int>(barWidth * progress)};

        auto now{std::chrono::high_resolution_clock::now()};
        auto elapsed{
            std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() /
            1000.0};

        std::string estimate{""};
        if (current > 0 && current < total) {
            double avgTime{elapsed / current};
            double remaining{avgTime * (total - current)};
            estimate = formatTime(remaining);
        }

        std::string spinner{spinnerFrames[spinnerIndex % spinnerFrames.size()]};
        spinnerIndex++;

        std::cout << "\r" << Colors::BrightCyan << spinner << " " << Colors::Reset;

        if (!estimate.empty()) {
            std::cout << Colors::White << "ETA " << Colors::BrightWhite << estimate << Colors::Reset
                      << "  ";
        }

        std::cout << Colors::BrightCyan << "[";
        for (int i{0}; i < barWidth; ++i) {
            if (i < pos) {
                std::cout << getRainbowColor(i, barWidth) << "█";
            } else if (i == pos) {
                std::cout << getRainbowColor(i, barWidth) << "▓";
            } else {
                std::cout << Colors::White << "░";
            }
        }
        std::cout << Colors::BrightCyan << "] " << Colors::BrightWhite << std::setw(3)
                  << static_cast<int>(progress * 100.0) << "%" << Colors::White << " (" << current
                  << "/" << total << ")" << Colors::Reset << std::flush;
    }

    void finish() {
        update(total);
        std::cout << std::endl;
    }

    void clear() {
        std::cout << "\r" << std::string(barWidth + 60, ' ') << "\r" << std::flush;
    }
};

struct BenchmarkResults {
    std::string command;
    double mean;
    double stdDev;
    double min;
    double max;
    int iterations;

    void display() const {
        std::cout << "\n"
                  << Colors::Bold << Colors::BrightWhite << "Benchmark: " << Colors::Reset
                  << command << Colors::Reset << "\n";

        std::cout << "  " << Colors::BrightGreen << "μ=" << std::fixed << std::setprecision(3)
                  << mean << " ms" << Colors::Dim << " (mean)" << Colors::Reset << "   "
                  << Colors::BrightMagenta << "σ=" << std::fixed << std::setprecision(3) << stdDev
                  << " ms" << Colors::Dim << " (std)" << Colors::Reset << "\n";

        std::cout << "  " << Colors::BrightBlue << "↓ " << std::fixed << std::setprecision(3) << min
                  << " ms" << Colors::Dim << " (min)" << Colors::Reset << "   " << Colors::BrightRed
                  << "↑ " << std::fixed << std::setprecision(3) << max << " ms" << Colors::Dim
                  << " (max)" << Colors::Reset << "\n";

        double opsPerSec{(mean > 0) ? (1000.0 / mean) : 0};
        std::cout << "  " << Colors::BrightYellow << "λ=" << std::fixed << std::setprecision(0)
                  << opsPerSec << " ops/s" << Colors::Dim << " (rate)" << Colors::Reset << "    "
                  << Colors::Dim << "(" << iterations << " iters)" << Colors::Reset << "\n\n";
    }

    std::string toJson() const {
        std::ostringstream json{};
        json << "{\n"
             << "  \"command\": \"" << command << "\",\n"
             << "  \"mean_ms\": " << std::fixed << std::setprecision(3) << mean << ",\n"
             << "  \"std_dev_ms\": " << stdDev << ",\n"
             << "  \"min_ms\": " << min << ",\n"
             << "  \"max_ms\": " << max << ",\n"
             << "  \"ops_per_sec\": " << std::fixed << std::setprecision(0) << (1000.0 / mean)
             << ",\n"
             << "  \"iterations\": " << iterations << "\n"
             << "}\n";
        return json.str();
    }
};

class ArgParser {
  private:
    std::map<std::string, std::string> arguments;
    std::vector<std::string> positionalArgs;
    std::string programName;

    void parseArgs(int argc, char** argv) {
        programName = std::string{argv[0]};

        for (int i{1}; i < argc; ++i) {
            std::string arg{argv[i]};

            if (arg.substr(0, 2) == "--") {
                std::string key{arg.substr(2)};

                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    arguments[key] = argv[i + 1];
                    ++i;
                } else {
                    arguments[key] = "";
                }
            } else {
                positionalArgs.push_back(arg);
            }
        }
    }

  public:
    ArgParser(int argc, char** argv) {
        parseArgs(argc, argv);
    }

    bool has(const std::string& key) const {
        return arguments.find(key) != arguments.end();
    }

    std::string get(const std::string& key, const std::string& defaultValue = "") const {
        auto it{arguments.find(key)};

        return (it != arguments.end()) ? it->second : defaultValue;
    }

    int getInt(const std::string& key, int defaultValue = 0) const {
        auto it{arguments.find(key)};
        if (it == arguments.end())
            return defaultValue;

        char* end;
        long value{std::strtol(it->second.c_str(), &end, 10)};
        if (*end != '\0' || end == it->second.c_str()) {
            return defaultValue;
        }

        return static_cast<int>(value);
    }

    bool getIntSafe(const std::string& key, int& outValue, int defaultValue = 0) const {
        auto it{arguments.find(key)};
        if (it == arguments.end()) {
            outValue = defaultValue;
            return true;
        }

        char* end;
        long value{std::strtol(it->second.c_str(), &end, 10)};

        if (*end != '\0' || end == it->second.c_str()) {
            std::cerr << Colors::BrightRed << "Error: " << Colors::Reset
                      << "Invalid integer value for --" << key << ": '" << it->second << "'\n";
            std::cerr << Colors::Dim << "Expected a number, e.g., --" << key << " 100"
                      << Colors::Reset << "\n";
            return false;
        }

        outValue = static_cast<int>(value);

        return true;
    }

    const std::vector<std::string>& getPositional() const {
        return positionalArgs;
    }

    bool validate() const {
        if (positionalArgs.empty()) {
            std::cerr << Colors::BrightRed << "Error: " << Colors::Reset
                      << "No command specified to benchmark\n\n";
            std::cerr << Colors::Dim << "Usage: " << programName << " [OPTIONS] <command>\n";
            std::cerr << "Example: " << programName << " sleep 0.1" << Colors::Reset << "\n\n";
            std::cerr << "Run '" << programName << " --help' for more information.\n";
            return false;
        }

        if (has("warmup")) {
            int warmup{};

            if (!getIntSafe("warmup", warmup)) {
                return false;
            }

            if (warmup < 0) {
                std::cerr << Colors::BrightRed << "Error: " << Colors::Reset
                          << "--warmup must be non-negative (got " << warmup << ")\n";
                std::cerr << Colors::Dim
                          << "Warmup iterations prepare the system before benchmarking."
                          << Colors::Reset << "\n";
                return false;
            }
        }

        if (has("iterations")) {
            int iterations{};

            if (!getIntSafe("iterations", iterations)) {
                return false;
            }

            if (iterations <= 0) {
                std::cerr << Colors::BrightRed << "Error: " << Colors::Reset
                          << "--iterations must be positive (got " << iterations << ")\n";
                std::cerr << Colors::Dim << "At least 1 iteration is required to benchmark."
                          << Colors::Reset << "\n";
                return false;
            }
        }

        if (has("output")) {
            std::string output{get("output")};

            if (output != "json" && output != "text") {
                std::cerr << Colors::BrightRed << "Error: " << Colors::Reset
                          << "--output must be either 'json' or 'text' (got '" << output << "')\n";
                std::cerr << Colors::Dim << "Available formats:\n";
                std::cerr << "  text - Colorful human-readable output (default)\n";
                std::cerr << "  json - Machine-readable JSON format" << Colors::Reset << "\n";
                return false;
            }
        }

        return true;
    }

    void showOptionHelp(const std::string& option) const {
        if (option == "warmup") {
            std::cout << Colors::Bold << Colors::BrightCyan << "--warmup <num>" << Colors::Reset
                      << "\n\n";
            std::cout << Colors::Bold << "Description:\n" << Colors::Reset;
            std::cout << "  Specifies the number of warmup iterations to run before the actual\n";
            std::cout << "  benchmark. Warmup iterations help ensure that caches, JIT compilers,\n";
            std::cout
                << "  and other dynamic optimizations are fully active before measurement.\n\n";
            std::cout << Colors::Bold << "Default:\n" << Colors::Reset << "  5\n\n";
            std::cout << Colors::Bold << "Valid Range:\n" << Colors::Reset << "  0 or greater\n\n";
            std::cout << Colors::Bold << "Examples:\n" << Colors::Reset;
            std::cout << "  " << programName << " --warmup 10 sleep 0.1    " << Colors::Dim
                      << "# Run 10 warmup iterations\n"
                      << Colors::Reset;
            std::cout << "  " << programName << " --warmup 0 ls           " << Colors::Dim
                      << "# Skip warmup\n"
                      << Colors::Reset;
        } else if (option == "iterations") {
            std::cout << Colors::Bold << Colors::BrightCyan << "--iterations <num>" << Colors::Reset
                      << "\n\n";
            std::cout << Colors::Bold << "Description:\n" << Colors::Reset;
            std::cout << "  Specifies how many times to run the command for benchmarking.\n";
            std::cout << "  More iterations provide more accurate results but take longer.\n";
            std::cout
                << "  Results include mean, standard deviation, min, max, and throughput.\n\n";
            std::cout << Colors::Bold << "Default:\n" << Colors::Reset << "  100\n\n";
            std::cout << Colors::Bold << "Valid Range:\n" << Colors::Reset << "  1 or greater\n\n";
            std::cout << Colors::Bold << "Recommendations:\n" << Colors::Reset;
            std::cout << "  Fast commands (< 1ms):    1000+ iterations\n";
            std::cout << "  Medium commands (< 100ms): 100-500 iterations\n";
            std::cout << "  Slow commands (> 1s):      10-50 iterations\n\n";
            std::cout << Colors::Bold << "Examples:\n" << Colors::Reset;
            std::cout << "  " << programName << " --iterations 1000 echo hello    " << Colors::Dim
                      << "# Fast command\n"
                      << Colors::Reset;
            std::cout << "  " << programName << " --iterations 20 sleep 0.5      " << Colors::Dim
                      << "# Slow command\n"
                      << Colors::Reset;
        } else if (option == "output") {
            std::cout << Colors::Bold << Colors::BrightCyan << "--output <format>" << Colors::Reset
                      << "\n\n";
            std::cout << Colors::Bold << "Description:\n" << Colors::Reset;
            std::cout << "  Controls the output format of benchmark results.\n\n";
            std::cout << Colors::Bold << "Default:\n" << Colors::Reset << "  text\n\n";
            std::cout << Colors::Bold << "Available Formats:\n" << Colors::Reset;
            std::cout << "  " << Colors::BrightGreen << "text" << Colors::Reset
                      << "  - Human-readable format with colors and Unicode symbols\n";
            std::cout << "         Shows mean (μ), std dev (σ), min (↓), max (↑), and rate (λ)\n";
            std::cout << "  " << Colors::BrightYellow << "json" << Colors::Reset
                      << "  - Machine-readable JSON format for scripting/automation\n";
            std::cout << "         Includes all metrics in a structured format\n\n";
            std::cout << Colors::Bold << "Examples:\n" << Colors::Reset;
            std::cout << "  " << programName << " --output text ls           " << Colors::Dim
                      << "# Colorful output\n"
                      << Colors::Reset;
            std::cout << "  " << programName << " --output json ls > out.json " << Colors::Dim
                      << "# Save JSON results\n"
                      << Colors::Reset;
        } else {
            std::cerr << Colors::BrightRed << "Error: " << Colors::Reset << "Unknown option '"
                      << option << "'\n\n";
            std::cerr << "Available options: warmup, iterations, output\n";
            std::cerr << "Run '" << programName << " --help' for general help.\n";
        }
    }

    void showHelp() const {
        std::cout << Colors::Bold << Colors::BrightCyan
                  << "Vajra - Command Line Benchmarking Tool\n"
                  << Colors::Reset << "\n";
        std::cout << "A fast, accurate, and beautiful benchmarking tool with real-time\n";
        std::cout << "progress tracking and detailed statistical analysis.\n\n";

        std::cout << Colors::Bold << "USAGE:\n"
                  << Colors::Reset << "  " << programName << " " << Colors::BrightYellow
                  << "[OPTIONS]" << Colors::Reset << " " << Colors::BrightGreen << "<command>\n"
                  << Colors::Reset;
        std::cout << "  " << programName << " " << Colors::BrightCyan << "--help" << Colors::Reset
                  << " " << Colors::Dim << "[option]" << Colors::Reset << "\n\n";

        std::cout << Colors::Bold << "OPTIONS:\n" << Colors::Reset;
        std::cout << "  " << Colors::BrightCyan << "--warmup <num>" << Colors::Reset
                  << "       Number of warmup iterations (default: 5)\n";
        std::cout << "  " << Colors::BrightCyan << "--iterations <num>" << Colors::Reset
                  << "   Number of benchmark iterations (default: 100)\n";
        std::cout << "  " << Colors::BrightCyan << "--output <format>" << Colors::Reset
                  << "    Output format: 'json' or 'text' (default: text)\n";
        std::cout << "  " << Colors::BrightCyan << "--shell" << Colors::Reset
                  << "              Execute command through shell (less accurate)\n";
        std::cout << "  " << Colors::BrightCyan << "--help" << Colors::Reset
                  << " [option]      Show help message (optionally for specific option)\n\n";

        std::cout << Colors::Bold << "ACCURACY TIPS:\n" << Colors::Reset;
        std::cout << "  " << Colors::BrightGreen << "✓" << Colors::Reset
                  << " Use more iterations for short-running commands\n";
        std::cout << "  " << Colors::BrightGreen << "✓" << Colors::Reset
                  << " Close unnecessary programs to reduce system noise\n";
        std::cout << "  " << Colors::BrightGreen << "✓" << Colors::Reset
                  << " Avoid shell features (pipes, redirects) for best accuracy\n";
        std::cout << "  " << Colors::BrightGreen << "✓" << Colors::Reset
                  << " Run warmup iterations to stabilize caches and CPU frequency\n\n";

        std::cout << Colors::Bold << "OUTPUT METRICS:\n" << Colors::Reset;
        std::cout << "  " << Colors::BrightGreen << "μ (mean)" << Colors::Reset
                  << "       Average execution time across all iterations\n";
        std::cout << "  " << Colors::BrightMagenta << "σ (std dev)" << Colors::Reset
                  << "    Standard deviation, measures consistency\n";
        std::cout << "  " << Colors::BrightBlue << "↓ (min)" << Colors::Reset
                  << "        Fastest execution time observed\n";
        std::cout << "  " << Colors::BrightRed << "↑ (max)" << Colors::Reset
                  << "        Slowest execution time observed\n";
        std::cout << "  " << Colors::BrightYellow << "λ (rate)" << Colors::Reset
                  << "       Operations per second (throughput)\n\n";

        std::cout << Colors::Bold << "EXAMPLES:\n" << Colors::Reset;
        std::cout << "  " << Colors::Dim << "# Basic usage\n" << Colors::Reset;
        std::cout << "  " << programName << " sleep 0.1\n\n";
        std::cout << "  " << Colors::Dim << "# More iterations for better accuracy\n"
                  << Colors::Reset;
        std::cout << "  " << programName << " --iterations 1000 ls -la\n\n";
        std::cout << "  " << Colors::Dim << "# Skip warmup and output JSON\n" << Colors::Reset;
        std::cout << "  " << programName << " --warmup 0 --output json python script.py\n\n";
        std::cout << "  " << Colors::Dim << "# Get detailed help for an option\n" << Colors::Reset;
        std::cout << "  " << programName << " --help iterations\n\n";

        std::cout << Colors::Dim << "For more information about a specific option, use:\n";
        std::cout << "  " << programName << " --help <option>" << Colors::Reset << "\n\n";
    }
};

#endif