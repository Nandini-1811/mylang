CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

SRC = src/scanner.c src/main.c
TARGET = lang

$(TARGET) : $(SRC) 
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

