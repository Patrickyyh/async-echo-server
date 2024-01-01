# Makefile for SyncServer

# Compiler to use
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++11

# Name of the executable
TARGET = main

# Source files
SRC = SyncClient.cpp

# Object files
OBJ = $(SRC:.cpp=.o)

# Rule to create the executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

# Rule to create object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

# Clean up
clean:
	rm -f $(OBJ) $(TARGET)

# Phony targets
.PHONY: clean
