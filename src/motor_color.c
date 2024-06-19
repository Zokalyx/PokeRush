#include "motor_color.h"

color_t color_crear(uint8_t r, uint8_t g, uint8_t b)
{
	return (color_t){
		.r = r,
		.g = g,
		.b = b,
	};
}

color_t color_mezcla(color_t color_1, color_t color_2, float porcentaje)
{
	uint8_t r = (uint8_t)(color_2.r + porcentaje * (color_1.r - color_2.r));

	uint8_t g = (uint8_t)(color_2.g + porcentaje * (color_1.g - color_2.g));

	uint8_t b = (uint8_t)(color_2.b + porcentaje * (color_1.b - color_2.b));

	return color_crear(r, g, b);
}

bool color_iguales(color_t color_1, color_t color_2)
{
	return color_1.r == color_2.r && color_1.g == color_2.g &&
	       color_1.b == color_2.b;
}
