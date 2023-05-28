CC = g++
CFLAGS = -w -g -I../Contrib/Eigen -DEIGEN_NO_DEBUG
LDFLAGS = -lm

OBJ = main.o

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

Application1: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LDFLAGS) -o Application1

.PHONY: clean
clean:
	rm -rf Application1 $(OBJ)
