INTERMEDIATES_DIR = build/intermediates
BUILD_DIR = build/Release
DEBUG_BUILD_DIR = build/Debug
SRC_DIR = src
CXX = g++
CXXFLAGS = -std=c++20 -O2 -Ilibraries/include -DGLM_ENABLE_EXPERIMENTAL -Llibraries/lib
DEBUG_FLAGS = -DGAME_DEBUG -g
LDFLAGS = -lglfw -ldl -pthread

SRCS := $(shell find $(SRC_DIR) libraries -type f -name '*.cpp' -or -name '*.c')

build: clean_current setup $(BUILD_DIR)/game
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: BUILD_DIR = $(DEBUG_BUILD_DIR)
debug: build

clean_current:
	rm -rf $(BUILD_DIR)
	rm -rf $(INTERMEDIATES_DIR)

clean:
	rm -rf build/

setup:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(INTERMEDIATES_DIR)
	mkdir -p $(INTERMEDIATES_DIR)/libs

run: build
	./$(BUILD_DIR)/game

$(INTERMEDIATES_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/game: $(SRCS:$(SRC_DIR)/%.cpp=$(INTERMEDIATES_DIR)/%.o)
	$(CXX) $(CXXFLAGS) $^ -o $(BUILD_DIR)/game $(LDFLAGS)
	cp -r assets $(BUILD_DIR)/assets
	mkdir $(BUILD_DIR)/worlds
	mkdir $(BUILD_DIR)/screenshots
	mkdir $(BUILD_DIR)/benchmarks
	rm -f build/Game.zip
	cd $(BUILD_DIR) && zip -r ../Game.zip * && cd ../..
