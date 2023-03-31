#include "preprocessor.hpp"

int main() {
    constexpr PreprocessorFlags flags = default_flags | PreprocessorFlags::debug;
    process_file("example_file.py", flags);
    return 0;
}
