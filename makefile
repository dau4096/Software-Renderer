CC = g++
CFLAGS = -std=c++20 \
         -IC:/Users/User/Documents/code/.cpp/glew-2.1.0/include \
         -IC:/Users/User/Documents/code/.cpp/glm \
         -IC:/Users/User/Documents/code/.cpp/glfw-3.4.bin.WIN64/include \
         -IC:/Users/User/Documents/code/.cpp
LIBS = -LC:/Users/User/Documents/code/.cpp/glew-2.1.0/lib/Release/x64 \
       -LC:/Users/User/Documents/code/.cpp/glfw-3.4.bin.WIN64/lib-mingw-w64 \
       -lglew32 -lglfw3 -lopengl32 -lglu32 -luser32 -lgdi32
SOURCES = main.cpp src/render.cpp src/utils.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: app

app: $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o app

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del $(OBJECTS) app.exe
