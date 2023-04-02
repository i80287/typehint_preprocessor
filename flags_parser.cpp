#include <cstring> // strcmp

#include "flags_parser.hpp"

static inline constexpr PreprocessorFlags parse_flag(const char *arg) {
    switch (arg[0]) {
    case 'd':
        return (strcmp(++arg, "ebug") == 0) ? PreprocessorFlags::debug : PreprocessorFlags::no_flags;
    case 'o':
        return (strcmp(++arg, "verwrite") == 0) ? PreprocessorFlags::debug : PreprocessorFlags::no_flags;
    case 'v':
        return (strcmp(++arg, "erbose") == 0) ? PreprocessorFlags::debug : PreprocessorFlags::no_flags;
    case 'c':
        return (strcmp(++arg, "ontinue_on_error") == 0) ? PreprocessorFlags::debug : PreprocessorFlags::no_flags;
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
