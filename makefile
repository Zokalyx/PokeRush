CFLAGS =-std=c99 -Wall -Wconversion -Wtype-limits -pedantic -Werror -O2 -g
CC = gcc

all: clean juego

juego: src/*.c juego.c
	$(CC) $(CFLAGS) src/*.c juego.c -o juego

clean:
	rm -f pruebas_alumno pruebas_chanutron juego