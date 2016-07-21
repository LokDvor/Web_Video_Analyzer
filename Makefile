CC              := g++
CFLAGS          := -I/usr/local/include/opencv -L/usr/local/lib
OBJECTS         := 
LIBRARIES       := -lopencv_core -lopencv_imgproc -lopencv_highgui -std=gnu++11 -lpthread

.PHONY: all clean

all: v_analyzer

v_analyzer: 
	$(CC) $(CFLAGS) -o v_analyzer v_analyzer.cpp $(LIBRARIES)
        
clean:
	rm -f *.o
