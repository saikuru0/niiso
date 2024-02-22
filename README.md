# Niiso

>Header only library for running a bot that follows the sockchat websocket subprotocol

## Building

~You don't need any dependencies to build a project with Niiso. After all,~ (now Niiso uses nlohmann's json library for its config) its interface is two string queues. I do recommend using `-Wall` and `-std=c++11` though.

Here's an example minimal Makefile for a single-file project using Niiso:

```Makefile
CC = g++
CFLAGS = -Wall -std=c++11 -Iwhatever/path/in/which/theres/json/include
LDFLAGS = 

SRC = main.cpp
LIB = niiso.hpp
TARGET = app

all: $(TARGET)

$(TARGET): $(SRC) $(LIB)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(TARGET) $(SRC)

clean: $(TARGET)
	rm $(TARGET)

.PHONY: all clean
```

## Usage

### Interface

Niiso uses two string queues, one for what gets put into the bot for parsing and another for what the bot spits out. The former can be changed by using `niiso.add(std::string packet)` and the latter by returning single elements using `niiso.next_msg()` *(name change to `next_packet()` pending)*. The class also has methods for specific Sockchat packets like `niiso.send(std::string msg)` for queueing a message to be sent to the chat or `niiso.ping()` for sending a ping packet to prevent timeout. To process all the packets in Niiso's input queue run `niiso.serve()`.

### Connection

Right now, the Niiso constructor takes ~the sockchat server `URI`,~ (removed it) the bot user `UID` and the bot user `authkey` (the data string part of the authkey, "Misuzu" is handled by Niiso). The `URI` is leftover code from previous versions because websocket communication is not in Niiso's scope - a dummy string can be safely used instead.

### Interaction

Niiso's chat commands start with `^` and right now are hardcoded inside the class (^help to view all of them). Although, they're already kept in a `std::map<std::string, Command>` where the string is what someone needs to send on the chat and `Command` is a custom struct containing a command's name, description and `std::function<void(const std::vector<std::string>&)>` handler, so I'm planning on moving them out of the class in the near future.
