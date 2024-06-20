

#include "motor_input.h"

#ifdef _WIN32
#include <conio.h>
#endif

#ifdef __unix__
#include <termios.h>
#include <unistd.h>
#include <sys/poll.h>
#include <stdio.h>
#endif

#define ESCAPE_WINDOWS_1 0
#define ESCAPE_WINDOWS_2 224
#define ESCAPE_UNIX '\x1b'

void deshabilitar_echo_terminal()
{
#ifdef _WIN32
	return;
#endif

#ifdef __unix__
	struct termios t;
	tcgetattr(STDIN_FILENO, &t);
	t.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &t);
#endif
}

void habilitar_echo_terminal()
{
#ifdef _WIN32
	return;
#endif

#ifdef __unix__
	struct termios t;
	tcgetattr(STDIN_FILENO, &t);
	t.c_lflag |= (tcflag_t)(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &t);
#endif
}

int leer_caracter()
{
#ifdef _WIN32
	if (_kbhit()) {
		int caracter = _getch();
		if (caracter == ESCAPE_WINDOWS_1 ||
		    caracter == ESCAPE_WINDOWS_2)
			return -_getch();
		else
			return caracter;
	} else {
		return 0;
	}
#endif

#ifdef __unix__
	struct pollfd fds;
	fds.fd = STDIN_FILENO;
	fds.events = POLLIN;
	if (poll(&fds, 1, 0) > 0) {
		int caracter = getchar();
		if (caracter == ESCAPE_UNIX) {
			getchar(); // Saltear '['
			return -getchar();
		} else {
			return caracter;
		}
	} else {
		return 0;
	}
#endif
}

// #include "motor_input.h"

// // Necesito funciones de estos archivos
// // sí o sí. No estaba restringido en el
// // enunciado y cuando alguien preguntó sobre
// // uno parecido en Discord yo ya había hecho
// // todo el juego. Las flechas de teclado son
// // la única manera de controlar el juego en
// // varias escenas. La única alternativa es
// // apretar Enter después de cada flecha, lo cual
// // no tiene sentido para el jugador.
// // Todo el código fue escrito a mano (con ayuda
// // de recursos de internet, obviamente) y se
// // explica detalladamente.
// #include <termios.h>
// #include <sys/poll.h>

// #include <stdio.h>

// #define ESCAPE_UNIX '\x1b'
// #define STDIN_FILENO 0

// void deshabilitar_echo_terminal()
// {
// 	// Obtener información sobre la terminal.
// 	struct termios t;
// 	tcgetattr(STDIN_FILENO, &t);

// 	// Deshabilitar modo canónico y echo.
// 	t.c_lflag &= (tcflag_t) ~(ICANON | ECHO);

// 	// Guardar configuración de la terminal.
// 	tcsetattr(STDIN_FILENO, TCSANOW, &t);
// }

// void habilitar_echo_terminal()
// {
// 	// Obtener información sobre la terminal.
// 	struct termios t;
// 	tcgetattr(STDIN_FILENO, &t);

// 	// Habilitar modo canónico y echo.
// 	t.c_lflag |= (tcflag_t)(ICANON | ECHO);

// 	// Guardar configuración de la terminal.
// 	tcsetattr(STDIN_FILENO, TCSANOW, &t);
// }

// int leer_caracter()
// {
// 	// Struct para pollear al FILE* stdin.
// 	struct pollfd fds;
// 	fds.fd = STDIN_FILENO;
// 	fds.events = POLLIN;

// 	// Verificar si hay algún caracter en stdin.
// 	if (poll(&fds, 1, 0) > 0) {
// 		int caracter = getchar();

// 		// Verificar si esto es una secuencia de escape.
// 		// Por ejemplo, flechitas.
// 		if (caracter == ESCAPE_UNIX) {
// 			getchar(); // Saltear '[' (control)
// 			return -getchar(); // Convención del motor, los caracteres de escape se devuelven negativos.
// 		} else {
// 			return caracter; // Hubo caracter normal.
// 		}
// 	} else {
// 		return 0; // No hubo caracter en stdin.
// 	}
// }
