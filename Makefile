CC := gcc
CFLAGS := -g -Wall -Wextra -Werror

TARGET := sshell

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	rm -f $(TARGET) core
