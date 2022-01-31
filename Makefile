EXECUTABLE = iceflac
SRC = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, build/%.o, $(SRC))
DEPENDS = $(patsubst src/%.c, build/%.d, $(SRC))

CC = gcc
CLIBS = -logg -lFLAC -lmxml
CFLAGS = -I./include -Wall -Wextra

.PHONY: all debug run clean

all: $(EXECUTABLE)
	cp ./settings/$(EXECUTABLE).xml ./build/$(EXECUTABLE).xml

debug: CFLAGS += -ggdb -DDEBUG
debug: all

-include $(DEPENDS)

$(OBJS): build/%.o : src/%.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(EXECUTABLE): $(OBJS)
	$(CC) -o build/$(EXECUTABLE) $(OBJS) $(CLIBS)

run: build/$(EXECUTABLE)
	cd build; ./$(EXECUTABLE)

clean:
	rm -f build/*.d
	rm -f build/*.o
	rm -f build/$(EXECUTABLE).out
	rm -f build/$(EXECUTABLE).xml
