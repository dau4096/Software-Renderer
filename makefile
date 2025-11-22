CC = g++
CFLAGS = -std=c++23 -O2 -ffast-math \
         -I/usr/include \
         -I/usr/include/GL \
         -I/usr/include/glm

LIBS = -lglfw -lGLEW -lGL -lpugixml -lm -ldl -pthread

SOURCES = main.cpp src/graphics.cpp src/utils.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: app

app: $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o app

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) app

