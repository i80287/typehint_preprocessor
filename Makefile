INCLUDE_DIR =../include

OBJDIR=obj
OBJ_FILES_LIST=main.o flags_parser.o preprocessor.o
OBJ_FILES=$(patsubst %,$(OBJDIR)/%,$(OBJ_FILES_LIST))

CPP_COMPILER=g++
COMPILER_FLAGS=-std=c++2b -O2 -Wall -Wextra -Wcast-align=strict -Wpedantic -Werror -pedantic-errors -I.

DEPENDENCIES=flags_parser.hpp preprocessor.hpp

OUTPUT_FILENAME=preprocessor.exe

# Windows
MKDIR_CHECKED=if not exist "$(OBJDIR)" mkdir $(OBJDIR)

# GNU/Linux
# MKDIR_CHECKED=mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.cpp $(DEPENDENCIES)
	$(MKDIR_CHECKED)
	$(CPP_COMPILER) -c -o $@ $< $(COMPILER_FLAGS)

preprocessor: $(OBJ_FILES)
	$(CPP_COMPILER) -o $(OUTPUT_FILENAME) $^ $(CFLAGS)

clean:
	rm -f $(OBJDIR)/*.o
