#ifndef __ANIMACION_H__
#define __ANIMACION_H__

#include <stdint.h>

/**
 * Movimiento lineal con comienzo y fin abruptos.
 * 
 * Resultado de punto flotante.
*/
float linear(uint64_t tiempo_ms, uint64_t comienzo_ms, uint64_t fin_ms,
	     int inicio, int fin);

/**
 * Movimiento no lineal con comienzo y final suaves.
*/
int ease_in_out(uint64_t tiempo_ms, uint64_t comienzo_ms, uint64_t fin_ms,
		int inicio, int fin);

/**
 * Onda "cuadrada", que oscila entre el valor máximo y mínimo.
*/
int parpadeo(uint64_t tiempo_ms, uint64_t periodo_ms, int inicio, int fin);

#endif // __ANIMACION_H__