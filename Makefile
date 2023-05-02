INCLUDE_DIR =../include

OBJDIR=obj
OBJ_FILES_LIST=main.o flags_parser.o preprocessor.o
OBJ_FILES=$(patsubst %,$(OBJDIR)/%,$(OBJ_FILES_LIST))

CPP_COMPILER=g++
CCFLAGS=-std=c++2b -O2 -Wall -Wextra -Wcast-align=strict -Wpedantic -Werror -pedantic-errors -I.

DEPENDENCIES=flags_parser.hpp preprocessor.hpp

ifeq ($(OS),Windows_NT)
    CCFLAGS += -D WIN32
	MKDIR_CHECKED := if not exist "$(OBJDIR)" mkdir $(OBJDIR)
    OUTPUT_FILENAME := preprocessor.exe
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        CCFLAGS += -D AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            CCFLAGS += -D AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            CCFLAGS += -D IA32
        endif
    endif
else
    UNAME_S := $(shell uname -s)
	MKDIR_CHECKED := mkdir -p $(OBJDIR)
    OUTPUT_FILENAME := preprocessor.out
    ifeq ($(UNAME_S),Linux)
        CCFLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        CCFLAGS += -D OSX
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        CCFLAGS += -D AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        CCFLAGS += -D IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        CCFLAGS += -D ARM
    endif
endif


$(OBJDIR)/%.o: %.cpp $(DEPENDENCIES)
	$(MKDIR_CHECKED)
	$(CPP_COMPILER) -c -o $@ $< $(CCFLAGS)

preprocessor: $(OBJ_FILES)
	$(CPP_COMPILER) -o $(OUTPUT_FILENAME) $^ $(CCFLAGS)

clean:
	rm -f $(OBJDIR)/*.o
