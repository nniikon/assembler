CFLAGS = -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE

SRC_DIR = source ../common/CPU_parseArgs
BUILD_DIR = build
EXECUTABLE = Ass

SRCS = $(wildcard $(addsuffix /*.cpp, $(SRC_DIR)))
OBJS = $(patsubst $(addsuffix /%.cpp, $(SRC_DIR)), $(BUILD_DIR)/%.o, $(SRCS))

all: $(BUILD_DIR) $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	g++ $^ $(CFLAGS) -o $(BUILD_DIR)/$@

$(BUILD_DIR)/%.o: $(addsuffix /%.cpp, $(SRC_DIR))
	g++ $^ $(CFLAGS) -c -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
