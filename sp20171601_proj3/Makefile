OBJECTS = 20171601.o
ORCS = 20171601.c  20171601.h

cc = gcc
CFLAGS = -g -c -Wall -W

TARGET = 20171601.out

$(TARGET) : $(OBJECTS)
	$(cc) -o $(TARGET) $(OBJECTS)
clean :
	rm   -rf $(OBJECTS) $(TARGET)

main.o : 20171601.c
