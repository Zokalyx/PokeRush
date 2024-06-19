#ifndef __ANIMACION_H__
#define __ANIMACION_H__

#include <stdint.h>

/**
 * Movimiento lineal con comienzo y fin abruptos.
 * 
 * Resultado de punto flotante.
*/
float linear(uint64_t tiempo, uint64_t tiempo_comienzo, uint64_t tiempo_fin,
	     int inicio, int fin);

/**
 * Movimiento no lineal con comienzo y final suaves.
*/
int ease_in_out(uint64_t tiempo, uint64_t tiempo_comienzo, uint64_t tiempo_fin,
		int inicio, int fin);

/**
 * Onda "cuadrada", que oscila entre el valor máximo y mínimo.
*/
int parpadeo(uint64_t tiempo, uint64_t periodo, int inicio, int fin);

#endif // __ANIMACION_H__