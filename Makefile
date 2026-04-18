.PHONY: clean

CFLAGS  = -Wall -Werror
PROJECT = raytracing
LIBRARY = -lSDL3
SOURCES = main.c

$(PROJECT): $(SOURCES)
	gcc $(SOURCES) -o $(PROJECT) $(LIBRARY)

clean:
	rm -f *.o $(PROJECT)

run: $(PROJECT)
	./$(PROJECT)
