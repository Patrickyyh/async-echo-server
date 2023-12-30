# Define the C++ compiler to use
CXX = g++

# Define any compile-time flags
CXXFLAGS = -Wall -std=c++11 -g

# Define any directories containing header files
INCLUDES = -I.

# Define library paths in addition to /usr/lib
LFLAGS = 

# Define any libraries to link into executable
LIBS = -lboost_system -lpthread

# Define the C++ source files
SRCS = main.cpp Server.cpp Session.cpp

# Define the C++ object files
OBJS = $(SRCS:.cpp=.o)

# Define the executable file 
MAIN = server_app

.PHONY: depend clean valgrind

all:    $(MAIN)
	@echo Simple compiler named server_app has been compiled

$(MAIN): $(OBJS) 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

# This is a suffix replacement rule for building .o's from .cpp's
# It uses automatic variables $<: the name of the prerequisite of
# the rule(a .cpp file) and $@: the name of the target of the rule (a .o file)
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

valgrind:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose ./$(MAIN)

# DO NOT DELETE THIS LINE -- make depend needs it
