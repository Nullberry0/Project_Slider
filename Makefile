CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra

SDL_FLAGS = $(shell pkg-config --cflags --libs sdl3 sdl3-ttf sdl3-image)
JSON_FLAGS = -I$(shell brew --prefix nlohmann-json)/include

TARGET = slider
SRC = Main.cpp Renderer.cpp Editor.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(JSON_FLAGS) $(SRC) -o $(TARGET) $(SDL_FLAGS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)