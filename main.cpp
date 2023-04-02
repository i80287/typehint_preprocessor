#include "preprocessor.hpp"

int main() {
    constexpr PreprocessorFlags flags = default_flags | PreprocessorFlags::debug;
    const std::unordered_set<std::string> filenames = {
        "example_file.py"
    };
    const std::unordered_set<std::string> ignored_functions {
        "function_to_ignore",
        "function_to_ignore_2",
        "ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_ignored_function"
    };
    process_files(filenames, ignored_functions, flags);
    return 0;
}
