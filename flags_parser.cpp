#include <cstring> // strcmp

#include "flags_parser.hpp"

static inline constexpr PreprocessorFlags parse_flag(const char *arg) {
    switch (arg[0]) {
    case 'd':
        if (strcmp(++arg, "ebug") == 0) {
            return PreprocessorFlags::debug;
        }
        break;
    case 'o':
        if (strcmp(++arg, "verwrite") == 0) {
            return PreprocessorFlags::overwrite_file;
        }
        break;
    case 'v':
        if (strcmp(++arg, "erbose") == 0) {
            return PreprocessorFlags::verbose;
        }
        break;
    case 'c':
        if (strcmp(++arg, "ontinue_on_error") == 0) {
            return PreprocessorFlags::continue_on_error;
        }
        break;
    case 'a':
        if (strcmp(++arg, "ll_disabled") == 0) {
            return PreprocessorFlags::all_flags_disabled;
        }
        break;
    }

    return PreprocessorFlags::no_flags;
}

PreprocessorFlags parse_flags(const size_t argc, const char **const argv) {
    PreprocessorFlags flags = PreprocessorFlags::no_flags;

    for (size_t i = 0; i < argc; ++i) {
        const char *arg = argv[i];
        if (!(arg[0] == '-')) {
            continue;
        }

        flags |= parse_flag(++arg);
    }

    return flags;
}
