#include "argparser.h"
#include "vajra.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

int executeCommand(const std::vector<std::string>& args) {
    pid_t pid{fork()};

    if (pid == -1) {
        return -1;
    } else if (pid == 0) {
        std::vector<char*> argv;
        argv.reserve(args.size() + 1);

        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }

        argv.push_back(nullptr);

        execvp(argv[0], argv.data());
        _exit(127);
    } else {
        int status;

        waitpid(pid, &status, 0);

        return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
}

std::vector<std::string> parseCommand(const std::string& command) {
    std::vector<std::string> args;
    std::string current;
    bool inQuote{false};

    for (size_t i{0}; i < command.size(); ++i) {
        char c{command[i]};

        if (c == '"' || c == '\'') {
            inQuote = !inQuote;
        } else if (c == ' ' && !inQuote) {
            if (!current.empty()) {
                args.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        args.push_back(current);
    }

    return args;
}

int main(int argc, char** argv) {
    ArgParser parser(argc, argv);

    if (parser.has("help") || argc == 1) {
        const auto& positional{parser.getPositional()};

        if (parser.has("help") && !positional.empty()) {
            parser.showOptionHelp(positional[0]);
        } else {
            parser.showHelp();
        }
        return 0;
    }

    if (!parser.validate()) {
        return 1;
    }

    int warmup, iterations;
    if (!parser.getIntSafe("warmup", warmup, 5) ||
        !parser.getIntSafe("iterations", iterations, 100)) {
        return 1;
    }
    std::string outputFormat{parser.get("output", "text")};
    bool useShell{parser.has("shell")};
    bool isJsonOutput{outputFormat == "json"};

    const auto& positionalArgs{parser.getPositional()};
    std::string command;
    for (size_t i{0}; i < positionalArgs.size(); ++i) {
        if (i > 0)
            command += " ";
        command += positionalArgs[i];
    }

    std::vector<std::string> cmdArgs;
    if (!useShell) {
        cmdArgs = parseCommand(command);
        if (cmdArgs.empty()) {
            std::cerr << Colors::BrightRed << "Error: " << Colors::Reset
                      << "Failed to parse command\n";
            return 1;
        }
    }

    if (!isJsonOutput) {
        std::cout << Colors::BrightCyan << "Running benchmark: " << Colors::BrightYellow << command
                  << Colors::Reset << "\n";
        std::cout << Colors::White << "Warmup: " << warmup << " | Iterations: " << iterations
                  << Colors::Reset << "\n\n";
    }

    if (warmup > 0) {
        if (!isJsonOutput) {
            std::cout << Colors::BrightMagenta << "Warming up..." << Colors::Reset << " "
                      << std::flush;
        }
        for (int i{0}; i < warmup; ++i) {
            if (useShell) {
                std::system(command.c_str());
            } else {
                executeCommand(cmdArgs);
            }
        }
        if (!isJsonOutput) {
            std::cout << Colors::BrightGreen << "✓" << Colors::Reset << "\n\n";
        }
    }

    if (!isJsonOutput) {
        std::cout << Colors::BrightGreen << "Benchmarking..." << Colors::Reset << "\n";
    }
    ProgressBar benchmarkBar(iterations);

    std::vector<double> timings{};
    timings.reserve(iterations);

    for (int i{0}; i < iterations; ++i) {
        Timer::Timer timer{};
        timer.start();
        if (useShell) {
            std::system(command.c_str());
        } else {
            executeCommand(cmdArgs);
        }
        timer.stop();
        timings.push_back(timer.elapsedMilliseconds());
        if (!isJsonOutput) {
            benchmarkBar.update(i + 1);
        }
    }
    if (!isJsonOutput) {
        benchmarkBar.finish();
        benchmarkBar.clear();

        int linesToClear{warmup > 0 ? 8 : 6};
        for (int i{0}; i < linesToClear; ++i) {
            std::cout << "\033[F\033[K";
        }
    }

    BenchmarkResults results{};
    results.command = command;
    results.mean = Statistics::mean(timings);
    results.stdDev = Statistics::stddev(timings);
    results.min = Statistics::min(timings);
    results.max = Statistics::max(timings);
    results.iterations = iterations;

    if (outputFormat == "json") {
        std::cout << results.toJson();
    } else {
        results.display();
    }

    return 0;
}
