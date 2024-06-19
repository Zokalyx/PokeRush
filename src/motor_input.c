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