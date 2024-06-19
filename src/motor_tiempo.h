#ifndef __TIEMPO_H__
#define __TIEMPO_H__

#include <stdint.h>

/**
 * Pausa la ejecución del programa según la cantidad
 * de milisegundos pasados por parámetro
 */
void sleep_ms(uint64_t milisegundos);

/**
 * Devuelve el tiempo actual en milisegundos, a partir
 * de una referencia arbitraria (solo debe usarse para
 * calcular deltas de tiempo).
*/
uint64_t ms_actuales();

#endif // __TIEMPO_H__