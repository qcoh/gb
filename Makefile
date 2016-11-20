EXECUTABLE=gb

CC=clang++-3.8
CFLAGS=-MMD -MP -g -std=c++14 -Wall -Wextra -Werror -Wconversion -Wsign-conversion -pedantic -I $(INCLUDE_DIR)
LFLAGS=

BUILD_DIR=build
SOURCE_DIR=source
INCLUDE_DIR=include

SOURCE:=$(wildcard $(SOURCE_DIR)/*.cpp)
OBJECTS:=$(patsubst $(SOURCE_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCE))
DEPENDENCIES:=$(OBJECTS:.o=.d)

$(BUILD_DIR)/$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -r $(BUILD_DIR)

-include $(DEPENDENCIES)
