#ifndef __SPRITE_H__
#define __SPRITE_H__

#include "motor_color.h"
#include "motor_estado.h"
#include <stdio.h>

/**
 * Estructura que representa un sprite a ser dibujado
 * por la pantalla.
*/
typedef struct sprite {
	unsigned ancho, alto;
	bool *mascara;
	color_t *color_fondo;
} sprite_t;

/**
 * Crea un sprite a partir de un BMP de colores 24bit.
 * 
 * El color verde (0, 255, 0) se usa como pantalla verde
 * y se elimina de la máscara.
 * 
 * Devuelve NULL en caso de error, escribiendo la razón en `estado`
 * (obligatorio pasarlo).
*/
sprite_t *sprite_crear(FILE *archivo, estado_t *estado);

/**
 * Libera la memoria reservada por el sprite.
*/
void sprite_destruir(sprite_t *sprite);

#endif // __SPRITE_H__