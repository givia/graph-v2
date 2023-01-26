#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>


// Intialize the console for outputting UTF-8 using cout
void init_console() {
#ifdef _MSC_VER
  SetConsoleOutputCP(CP_UTF8); // Change console from OEM to UTF-8 in Windows
#endif
  setvbuf(stdout, nullptr, _IOFBF, 1000); // avoid shearing multi-byte characters across buffer boundaries
}
