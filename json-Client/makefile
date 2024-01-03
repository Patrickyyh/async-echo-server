# Makefile for SyncServer
UNAME := $(shell uname -s)

# Compiler to use
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++11

# Name of the executable
TARGET = main

# Source files
SRC = main.cpp

INCLUDES =

LFLAGS =

LIBS = -lpthread -ljsoncpp

# Object files
OBJ = $(SRC:.cpp=.o)

ifeq ($(UNAME), Linux)
	INCLUDES += -I.
	LFLAGS +=
endif

ifeq ($(UNAME), Darwin)
	INCLUDES = -I/opt/homebrew/opt/boost/include -I/opt/homebrew/opt/jsoncpp/include
	LFLAGS = -L/opt/homebrew/opt/boost/lib -L/opt/homebrew/opt/jsoncpp/lib
endif

# Rule to create the executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(OBJ) $(LFLAGS) $(LIBS)

# Rule to create object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES)  -c $<

# Clean up
clean:
	rm -f $(OBJ) $(TARGET)

# Phony targets
.PHONY: clean
