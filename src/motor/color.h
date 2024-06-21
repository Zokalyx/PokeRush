#ifndef __COLOR_H__
#define __COLOR_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * Estructura para colores de 24-bit.
*/
typedef struct color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} color_t;

/**
 * Conveniencia para crear colores rápidamente.
*/
color_t color_crear(uint8_t r, uint8_t g, uint8_t b);

/**
 * Mezcla dos colores dado un parámetro entre 0 y 1.
*/
color_t color_mezcla(color_t color_1, color_t color_2, float porcentaje);

/**
 * Dice si dos colores son iguales o no.
*/
bool color_iguales(color_t color_1, color_t color_2);

#endif // __COLOR_H__