# typehint_preprocessor

Preprocessor to remove typehints from the .py file in order to decrease ram consumption during the runtime. (typehints are stored in the \__annotations__ dict)

Some steps for setting up a proprocessor
----------------------

Text files
----------------------

In order to use the preprocessor you need to create file `files.txt` and optionally `ignored_functions.txt`
    
`files.txt` should contains paths (or names) to the python files
As an example, see `files_example.txt`

If created, `ignored_functions.txt` should contains names of the Python functions that will be ignored by the preprocessor
Typehints for the arguments and return type of these functions will not be removed

Build preprocessor executable file
----------------------

To build executable file you can install `make` system and run the following in the terminal:

    make

This will run `make` that will read `Makefile` and build the executable file

Also you can manually compile `.cpp` files to the executable.
For example, following command will compile `.cpp` files to the Windows `.exe` via `g++` with using c++ 2023 standart

    g++ main.cpp flags_parser.cpp preprocessor.cpp -std=c++2b -O2 -Wall -Wextra -Wcast-align=strict -Wpedantic -Werror -pedantic-errors -I. -o preprocessor.exe

Usage and preprocessor flags
----------------------

After you made text file(s) and built preprocessor executable file, you can use it
Also you can add flags to the preprocessor:

- -overwrite Will force preprocessor to overwrite files.
This flag is turned off by default and thus preprocessor saves copies of the `.py` files as `tmp_OriginalFilname.py` instead of the overwritting

- -verbose Will turn on the basic logging of the processed files and will say if any errors occured during this process.
This flag is turned on by default

- -debug Will turn on -verbose flag and debug mode additionally. It will print out statistic about
each line and term processed by the preprocessor. The output can be quite cumbersome
This flag is turned off by default

- -continue_on_error Will force preprocessor to continue if any error occured. `This flag is not recommended to use`
This flag is turned off by default

- -all_disabled Will disable all flags
This flag is turned off by default
