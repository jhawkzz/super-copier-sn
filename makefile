# Compiler
CC = g++

# Compiler flags
CFLAGS = -std=c++11 -Wall

# Include directories
INCLUDES = -I/path/to/include

# Libraries
# LIBS = -L/path/to/lib -lmylibrary
LIBS = -lgpiodcxx -lgpiod

# Source files
SRCS = cartPin.cpp gpioLine.cpp main.cpp snCartPins.cpp superCopierSN.cpp 

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
TARGET = superCopierSN

# Make rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS) $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)