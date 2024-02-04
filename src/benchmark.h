#ifndef QGI_BENCHMARK_H
#define QGI_BENCHMARK_H

#include <string>

namespace BENCH {
    void perf_t(std::string path, std::string format, unsigned char run_time);
    void benchmark(std::string sample_path);
}

#endif
