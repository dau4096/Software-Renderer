CC = g++
CFLAGS = -std=c++23 -O2 -ffast-math \
         -I/usr/local/include \
         -I/usr/local/include/GL \
         -I/usr/local/include/glm

LIBS = -lglfw -lGLEW -lGL -lpugixml -lm -ldl -pthread

SOURCES = main.cpp src/graphics.cpp src/utils.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: prgm

prgm: $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o prgm

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) prgm

