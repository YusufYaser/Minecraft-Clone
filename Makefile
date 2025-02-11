INTERMEDIATES_DIR = build/intermediates
BUILD_DIR = build/Release
SRC_DIR = src
CXX = g++
CXXFLAGS = -std=c++20 -O2 -Ilibraries/include -DGLM_ENABLE_EXPERIMENTAL -Llibraries/lib
LDFLAGS = -lglfw

SRCS := $(shell find $(SRC_DIR) libraries -type f -name '*.cpp' -or -name '*.c')

clean:
	rm -rf build/

all: $(BUILD_DIR)/game

$(INTERMEDIATES_DIR)/%.o: $(SRC_DIR)/%.cpp
	rm -f $@
	mkdir -p $@
	rmdir $@
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/game: $(SRCS:$(SRC_DIR)/%.cpp=$(INTERMEDIATES_DIR)/%.o)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
