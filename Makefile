TERMINAL = gnome-terminal

# Compiler selection
CC = gcc

# Compiler options
CCFLAGS = -O2 -Wall -Wextra -g

# FILES TO COMPILE
SRC = udpclient.c udpserver.c udpserver2.c

# TARGETS
EXEC = $(SRC:.c=)

# AutoCompiling
all: $(EXEC)
	@(echo "Terminé")

$(EXEC): $(SRC)

udpclient: udpclient.c
	$(CC) $(CCFLAGS) -o $@ $<

udpserver: udpserver.c
	$(CC) $(CCFLAGS) -o $@ $<

udpserver2: udpserver2.c
	$(CC) $(CCFLAGS) -o $@ $<

clean:
	rm -vf udpclient udpserver udpserver2

.PHONY: clean all
