CC=clang
CFLAGS=-Wall -Wextra -std=c11
TARGET=MiniMacOS

all: $(TARGET)

$(TARGET): src/main.c
	$(CC) $(CFLAGS) src/main.c -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
