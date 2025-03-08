INTERMEDIATES_DIR = build/intermediates
BUILD_DIR = build/Release
SRC_DIR = src
CXX = g++
CXXFLAGS = -std=c++20 -O2 -Ilibraries/include -DGLM_ENABLE_EXPERIMENTAL -Llibraries/lib
LDFLAGS = -lglfw -ldl -pthread

SRCS := $(shell find $(SRC_DIR) libraries -type f -name '*.cpp' -or -name '*.c')

clean:
	rm -rf build/

setup:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(INTERMEDIATES_DIR)
	mkdir -p $(INTERMEDIATES_DIR)/libs

build: setup $(BUILD_DIR)/game

run: $(BUILD_DIR)/game
	./$(BUILD_DIR)/game

$(INTERMEDIATES_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/game: $(SRCS:$(SRC_DIR)/%.cpp=$(INTERMEDIATES_DIR)/%.o)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	cp -r assets $(BUILD_DIR)/assets
	rm -f Game.zip
	cd build/Release && zip -r ../../Game.zip * && cd ../..
