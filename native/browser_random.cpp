// Preserve the frozen Windows restoration's MSVC CRT sequence in WebAssembly.
// libc rand() is platform-dependent; the gameplay consumes its low seven bits.
// This adapter is linked ONLY into the browser build, including its contracts.
#include <cstdint>
#include <cstdlib>
namespace { std::uint32_t state = 1; }
extern "C" void srand(unsigned int seed) { state = seed; }
extern "C" int rand() {
    state = state * 214013u + 2531011u;
    return int((state >> 16) & 0x7fffu);
}
