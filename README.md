# Vajra ⚡

A fast, accurate, and beautiful command-line benchmarking tool. Think `hyperfine` but with real-time progress tracking, better accuracy, and a gorgeous rainbow progress bar.

## Why Vajra?

Vajra (वज्र) means "thunderbolt" in Sanskrit. It's:

- Fast (like lightning)
- Powerful (accurate measurements)
- Sharp (precise timing)

Plus it sounds cooler than "benchmark-tool-v2" 😎

I got tired of switching between different tools for benchmarking. `time` is too basic, `perf` is overkill for simple tasks, and while `hyperfine` is great, I wanted something that shows real-time progress and gives me more control over the measurement process.

Vajra uses direct process execution (fork/exec) instead of spawning a shell for every run, which eliminates ~2-5ms of overhead per iteration. For fast commands, this makes a huge difference.

## Comparison

| Feature                     | Vajra                 | hyperfine | time | perf |
| --------------------------- | --------------------- | --------- | ---- | ---- |
| Real-time progress          | ✓ Rainbow!            | ✗         | ✗    | ✗    |
| ETA during benchmark        | ✓                     | ✗         | ✗    | ✗    |
| Direct execution (no shell) | ✓                     | ✗         | ✗    | ✓    |
| Warmup iterations           | ✓                     | ✓         | ✗    | ✗    |
| JSON output                 | ✓                     | ✓         | ✗    | ✗    |
| Statistical analysis        | ✓ (μ, σ, min, max, λ) | ✓         | ✗    | ✓    |
| Pretty colors               | ✓                     | ✓         | ✗    | ✗    |
| Easy to use                 | ✓                     | ✓         | ✓    | ✗    |
| CPU profiling               | ✗                     | ✗         | ✗    | ✓    |

**TL;DR:** Use Vajra when you want accurate timing with a nice UI. Use `perf` when you need CPU profiling. Use `hyperfine` if you don't care about real-time feedback.

## Installation

```bash
git clone https://github.com/navithecoderboi/vajra.git
cd vajra
mkdir build && cd build
cmake ..
ninja  # or make
sudo ninja install
```

## Quick Start

```bash
# Basic usage
vajra "sleep 0.1"

# More iterations for better accuracy
vajra --iterations 1000 "echo hello"

# Skip warmup, output JSON
vajra --warmup 0 --output json "ls -la"

# When you need shell features (pipes, redirects)
vajra --shell "ls | grep .cpp"
```

**Pro tip:** Always wrap your command in quotes, especially if it has spaces or arguments. Without quotes, each word becomes a separate argument to Vajra instead of your command.

## Output Explained

```
Benchmark: sleep 0.1
  μ=102.456 ms (mean)     σ=1.234 ms (std)
  ↓ 101.123 ms (min)      ↑ 105.789 ms (max)
  λ=9 ops/s (rate)    (100 iters)
```

- **μ (mu)**: Average time - what you usually care about
- **σ (sigma)**: Standard deviation - how consistent your timings are
- **↓**: Fastest run (best case)
- **↑**: Slowest run (worst case)
- **λ (lambda)**: Operations per second (throughput)

Lower σ = more consistent results = better benchmark.

## Options

### `--warmup <num>`

Number of times to run the command before measuring. Default: 5

Warmup runs help stabilize CPU frequency scaling, fill caches, and trigger JIT compilation. Skip warmup (`--warmup 0`) only if you specifically want to measure cold-start performance.

### `--iterations <num>`

How many times to run the command. Default: 100

**Rule of thumb:**

- Fast commands (< 1ms): 1000+ iterations
- Medium commands (< 100ms): 100-500 iterations
- Slow commands (> 1s): 10-50 iterations

### `--output <format>`

Output format: `text` (default) or `json`

JSON output is great for piping to other tools or saving results:

```bash
vajra --output json "sleep 0.1" > results.json
```

### `--shell`

Execute through shell (enables pipes, redirects, wildcards)

By default, Vajra uses direct execution for better accuracy. Use `--shell` when you need shell features:

```bash
vajra --shell "cat *.txt | wc -l"
```

**Warning:** Shell mode adds 2-5ms overhead per run.

### `--help [option]`

Get detailed help about a specific option:

```bash
vajra --help iterations
```

## Tips for Accurate Benchmarks

1. **Quote your commands** - Always use `vajra "your command here"` instead of `vajra your command here`. This ensures Vajra treats it as a single command, not multiple arguments.

2. **Close background apps** - Discord, Spotify, etc. can add noise to your measurements

3. **Use more iterations for fast commands** - The faster your command, the more iterations you need to smooth out timing noise

4. **Always run warmup** - Unless you specifically want to measure cold-start performance

5. **Avoid shell mode** - Direct execution is more accurate. Only use `--shell` when you need pipes/redirects

6. **Check CPU frequency scaling** - On Linux:

    ```bash
    cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
    ```

    Set to "performance" for consistent results:

    ```bash
    sudo cpupower frequency-set -g performance
    ```

7. **Run benchmarks multiple times** - If you're publishing results, run the whole benchmark 3-5 times and report the median

8. **Compare similar workloads** - Comparing `sleep 0.1` vs actual work isn't meaningful

## Using Vajra in Your C++ Code

You don't have to use the CLI! Vajra comes with `vajra.hpp` for in-code benchmarking:

```cpp
#include "vajra.hpp"

int main() {
    // Simple timing
    Timer::Timer timer;
    timer.start();
    // ... your code ...
    timer.stop();
    std::cout << "Took: " << timer.elapsedMilliseconds() << "ms\n";

    // Benchmark with statistics
    Benchmark bench("My Function", 100, 10);
    auto times = bench.run([]() {
        // ... code to benchmark ...
    });
    bench.printStats(times);

    // Statistical functions
    std::vector<double> data = {1.2, 3.4, 2.1, 5.6};
    std::cout << "Mean: " << Statistics::mean(data) << "\n";
    std::cout << "StdDev: " << Statistics::stddev(data) << "\n";
    std::cout << "P95: " << Statistics::percentile(data, 95.0) << "\n";

    return 0;
}
```

This is super useful for:

- Benchmarking hot paths in your code
- A/B testing algorithm implementations
- Performance regression testing
- Profiling without external tools

Just include `vajra.hpp` and you get:

- `Timer` class for simple timing
- `Benchmark` class for statistical benchmarking
- `Statistics` namespace (mean, median, stddev, percentiles, etc.)
- `Memory` utilities for tracking memory usage
- `Profiler` for section-based profiling

## Examples

### Compare two implementations

```bash
# Method 1: grep
vajra --warmup 10 --iterations 500 "grep TODO src/*.cpp"

# Method 2: ripgrep
vajra --warmup 10 --iterations 500 "rg TODO src/"
```

### Test script performance

```bash
vajra --iterations 50 "python my_script.py"
vajra --iterations 50 "pypy my_script.py"
# Now you know which is faster!
```

### Measure cold start vs warm

```bash
# Cold start (no warmup)
vajra --warmup 0 --iterations 100 "./my_program"

# Warm start (with warmup)
vajra --warmup 50 --iterations 100 "./my_program"
```

### CI/CD Integration

```bash
# Fail if command takes > 100ms on average
result=$(vajra --output json --iterations 100 "./my_program")
mean=$(echo $result | jq -r '.mean_ms')
if (( $(echo "$mean > 100" | bc -l) )); then
    echo "Performance regression detected!"
    exit 1
fi
```

### Benchmark different compiler flags

```bash
g++ -O0 code.cpp -o prog_O0
g++ -O2 code.cpp -o prog_O2
g++ -O3 code.cpp -o prog_O3

vajra --iterations 1000 "./prog_O0"
vajra --iterations 1000 "./prog_O2"
vajra --iterations 1000 "./prog_O3"
```

## Real-World Example

Let's say you want to optimize a build script:

```bash
# Baseline
vajra --warmup 5 --iterations 10 "./build.sh" > baseline.txt

# After optimization 1
vajra --warmup 5 --iterations 10 "./build.sh" > opt1.txt

# After optimization 2
vajra --warmup 5 --iterations 10 "./build.sh" > opt2.txt

# Compare visually or parse JSON for automated comparisons
```

## Contributing

Found a bug? Want a feature? PRs welcome! Just:

1. Keep it fast
2. Keep it simple
3. Add tests if you're adding features

## Credits

Inspired by hyperfine, but built from scratch because I wanted to learn how to build a proper benchmarking tool.
