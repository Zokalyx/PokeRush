#ifndef __MOTOR_H__
#define __MOTOR_H__

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include "estado.h"
#include "pantalla.h"

/**
 * Estructura de configuración para el motor.
 * 
 * Debe ser proporcionada por el juego.
*/
typedef struct motor_config {
	unsigned ancho_pantalla;
	unsigned alto_pantalla;
} motor_config_t;

/**
 * Interface necesaria para que un juego
 * se pueda ejecutar con el motor.
*/
typedef struct juego {
	// Devuelve la configuración del motor a ser utilizada.
	motor_config_t (*config_motor)();
	// Creación e inicialización del juego
	// Debe retornar su estructura principal.
	// En caso de error retorna NULL y sobreescribe estado obligatoriamente.
	void *(*iniciar)(void *configuracion, estado_t *estado);
	// Procesamiento de eventos por input y por tiempo
	// Devuelve si hay que finalizar con el juego o no.
	// En caso de finalizar escribe en estado la razón de finalización.
	bool (*procesar_eventos)(void *estructura, int input, estado_t *estado);
	// Generación de la imagen en pantalla para el contexto y tiempo actual
	void (*dibujar_graficos)(void *estructura, pantalla_t *pantalla);
	// Liberar toda la memoria asociada al juego
	void (*finalizar)(void *estructura);
} juego_t;

/**
 * Carga y ejecuta el juego pasado por parámetro.
 * 
 * Se debe pasar la configuración del juego.
 * 
 * Retorna el estado de finalización.
 * 
 * Al finalizar libera los recursos del juego.
*/
estado_t motor_ejecutar_juego(juego_t *juego, void *config_juego);

#endif // __MOTOR_H_