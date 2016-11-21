EXECUTABLE=gb

CC=clang++-3.8
CFLAGS=-MMD -MP -g -std=c++14 -Wall -Wextra -Werror -Wconversion -Wsign-conversion -pedantic -I $(INCLUDE_DIR)
LFLAGS=

BUILD_DIR=build
SOURCE_DIR=source
INCLUDE_DIR=include
TEST_DIR=test

SOURCE:=$(wildcard $(SOURCE_DIR)/*.cpp)
OBJECTS:=$(patsubst $(SOURCE_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCE))

TEST_SOURCE:=$(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJECTS:=$(patsubst $(TEST_DIR)/%.cpp, $(BUILD_DIR)/%.test.o, $(TEST_SOURCE))

DEPENDENCIES:=$(OBJECTS:.o=.d)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) -o $@


$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

test: $(TEST_OBJECTS) $(OBJECTS)
	$(CC) $(TEST_OBJECTS) $(filter-out $(BUILD_DIR)/gb.o, $(OBJECTS)) $(LFLAGS) -o $(BUILD_DIR)/$@

$(BUILD_DIR)/%.test.o: $(TEST_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -r $(BUILD_DIR)

-include $(DEPENDENCIES)
