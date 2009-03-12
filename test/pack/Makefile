include   ../make.rules

CC	  = gcc
LD        = ld
TARGET	  = libmypack.so
CFLAGS    := -g -Wall -fPIC -I/usr/include/evms
OBJECTS   := ObjectStructs.o ReplacedStructs.o

%.o : %.c
	$(CC) $(CFLAGS) -c $^

all: $(TARGET) 

$(TARGET): $(OBJECTS)
	$(LD) -shared -o $@ $^ 
	rm $(LIB_DIR)$(TARGET) -rf
	cp $(TARGET) $(LIB_DIR)$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

