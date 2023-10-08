CFLAGS = -fsanitize=address -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE

SRC_DIR = source
LIB_DIR = ../lib
BUILD_DIR = build

# List of source files in SRC_DIR
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# List of additional .o files from the LIB_DIR
LIB_OBJS = $(wildcard $(LIB_DIR)/*.o)

# List of all .o files including both from SRC_DIR and LIB_DIR
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS)) $(LIB_OBJS)

EXECUTABLE = Assembler

all: $(BUILD_DIR) $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	g++ $^ $(CFLAGS) -o $(BUILD_DIR)/$@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	g++ $^ $(CFLAGS) -c -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)