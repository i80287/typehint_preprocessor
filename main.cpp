#include "preprocessor.hpp"

int main() {
    constexpr PreprocessorFlags flags = default_flags | PreprocessorFlags::debug;
    std::unordered_set<std::string> ignored_functions {
        "function_to_ignore",
        "function_to_ignore_2",
        "ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_function"
    };
    process_file("example_file.py", ignored_functions, flags);
    return 0;
}
