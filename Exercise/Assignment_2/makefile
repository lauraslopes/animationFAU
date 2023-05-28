CC = g++
CFLAGS = -w -g -I../Contrib/Eigen -I/usr/include
LDFLAGS = -L/usr/lib/x86_64-linux-gnu -lGL -lglut -lGLU -lGLEW -lX11 -lm

OBJ = camera.o light.o phongmaterial.o renderable.o renderer.o shaderprogram.o surface.o main.o

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

Application2: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LDFLAGS) -o Application2

.PHONY: clean
clean:
	rm -rf $(OBJ) Application2
