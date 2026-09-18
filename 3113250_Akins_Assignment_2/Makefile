CC = gcc
CFLAGS = -Wall -Wextra -std=c11

CHAT_TARGET = chat
CLUADE_TARGET = cluade

.PHONY: all clean

all: $(CHAT_TARGET) $(CLUADE_TARGET)

$(CHAT_TARGET): Chat.c
	$(CC) $(CFLAGS) -o $@ $<

$(CLUADE_TARGET): Cluade.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(CHAT_TARGET) $(CLUADE_TARGET) $(CHAT_TARGET).exe $(CLUADE_TARGET).exe
