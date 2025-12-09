CC = gcc
CFLAGS = -Wall -g
TARGET = filesystem
OBJS = main.o libDisk.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c libDisk.h error.h
	$(CC) $(CFLAGS) -c main.c

libDisk.o: libDisk.c libDisk.h error.h
	$(CC) $(CFLAGS) -c libDisk.c

clean:
	rm -f $(OBJS) $(TARGET) Release/step_*.bin

run: $(TARGET)
	mkdir -p Release
	./$(TARGET)