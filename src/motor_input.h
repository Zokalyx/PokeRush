#ifndef __INPUT_H__
#define __INPUT_H__

#define LINEFEED 10
#define BACKSPACE 127
#define FLECHA_ARRIBA -'A'
#define FLECHA_ABAJO -'B'
#define FLECHA_DERECHA -'C'
#define FLECHA_IZQUIERDA -'D'

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
 * Reestablece la terminal a su estado canónico,
 * con ECHO habilitado.
 */
void habilitar_echo_terminal();

/**
 * Deshabilita el ECHO de caracteres
 * en la terminal y el modo canónico.
 * De esta manear no hay Line Buffering
 * y los caracteres pasan directamente
 * a stdin para ser leídos por el programa.
 */
void deshabilitar_echo_terminal();

#endif // __INPUT_H__