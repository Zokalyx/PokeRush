#include "input.h"

// Necesito funciones de estos archivos
// sí o sí. No estaba restringido en el
// enunciado y cuando alguien preguntó sobre
// uno parecido en Discord yo ya había hecho
// todo el juego. Las flechas de teclado son
// la única manera de controlar el juego en
// varias escenas. La única alternativa es
// apretar Enter después de cada flecha, lo cual
// no tiene sentido para el jugador.
// Todo el código fue escrito a mano (con ayuda
// de recursos de internet, obviamente) y se
// explica detalladamente.
#include <termios.h>
#include <sys/poll.h>

#include <stdio.h>

#define ESCAPE_UNIX '\x1b'
#define STDIN_FILENO 0

void deshabilitar_echo_terminal()
{
	// Obtener información sobre la terminal.
	struct termios t;
	tcgetattr(STDIN_FILENO, &t);

	// Deshabilitar modo canónico y echo.
	t.c_lflag &= (tcflag_t) ~(ICANON | ECHO);

	// Guardar configuración de la terminal.
	tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void habilitar_echo_terminal()
{
	// Obtener información sobre la terminal.
	struct termios t;
	tcgetattr(STDIN_FILENO, &t);

	// Habilitar modo canónico y echo.
	t.c_lflag |= (tcflag_t)(ICANON | ECHO);

	// Guardar configuración de la terminal.
	tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

int leer_caracter()
{
	// Struct para pollear al FILE* stdin.
	struct pollfd fds;
	fds.fd = STDIN_FILENO;
	fds.events = POLLIN;

	// Verificar si hay algún caracter en stdin.
	if (poll(&fds, 1, 0) > 0) {
		int caracter = getchar();

		// Verificar si esto es una secuencia de escape.
		// Por ejemplo, flechitas.
		if (caracter == ESCAPE_UNIX) {
			getchar(); // Saltear '[' (control)
			return -getchar(); // Convención del motor, los caracteres de escape se devuelven negativos.
		} else {
			return caracter; // Hubo caracter normal.
		}
	} else {
		return 0; // No hubo caracter en stdin.
	}
}
