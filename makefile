# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -g3 -D_WIN64 -I"C:/Program Files/Thorlabs/Scientific Imaging/DCx Camera Support/Develop/Source/uc480Acquire" -Iinclude

# Libraries
LIBS = -lws2_32 -L"C:/Program Files/Thorlabs/Scientific Imaging/DCx Camera Support/Develop/Source/uc480Acquire" -luc480_64

# Directories
SRC_DIR = C++
OBJ_DIR = obj
BIN_DIR = output

# Source files
SOURCES = $(SRC_DIR)/run_all.cpp $(SRC_DIR)/send_data.cpp $(SRC_DIR)/Initialize_camera.cpp

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Output executable
TARGET = $(BIN_DIR)/run_all.exe

# Default rule
all: $(TARGET)

# Rule to create the executable
$(TARGET): $(OBJECTS)
	if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET) $(LIBS)

# Rule to create object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean target
clean:
	rmdir /S /Q $(OBJ_DIR) $(BIN_DIR)

# Run target
run: all
	$(TARGET)
