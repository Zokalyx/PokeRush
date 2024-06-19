#ifndef __INPUT_H__
#define __INPUT_H__

#ifdef _WIN32
#define LINEFEED 13
#define BACKSPACE 8
#define FLECHA_ARRIBA -72
#define FLECHA_IZQUIERDA -75
#define FLECHA_DERECHA -77
#define FLECHA_ABAJO -80
#endif

#ifdef __unix__
#define LINEFEED 10
#define BACKSPACE 127
#define FLECHA_ARRIBA -'A'
#define FLECHA_ABAJO -'B'
#define FLECHA_DERECHA -'C'
#define FLECHA_IZQUIERDA -'D'
#endif

/**
 * Consume y devuelve el primer caracter
 * presente en `stdin`, o 0 si no hubiera ninguno.
 * No bloquea el programa.
 * 
 * Los caracteres que consisten en secuencias de escape
 * se devuelven con valor negativo.
*/
int leer_caracter();

/**
 * Habilita la visibilidad de caracteres
 * tipados (`stdin`) en la terminal.
*/
void habilitar_echo_terminal();

/**
 * Deshabilita la visibilidad de caracteres
 * tipados (`stdin`) en la terminal.
*/
void deshabilitar_echo_terminal();

#endif // __INPUT_H__