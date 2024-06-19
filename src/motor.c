#include "motor.h"

#include "motor_input.h"
#include "motor_tiempo.h"
#include "motor_pantalla.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

#define T_DEBUG '\t'

volatile bool interrupcion = false;
void handler_interrupcion(int numero_signal)
{
	interrupcion = true;
}

/**
 * Determina si todos los punteros del juego son válidos.
*/
bool juego_valido(juego_t *juego)
{
	return juego != NULL && juego->dibujar_graficos != NULL &&
	       juego->finalizar != NULL && juego->iniciar != NULL &&
	       juego->procesar_eventos != NULL && juego->config_motor != NULL;
}

/**
 * Ejecuta cada frame uno a uno en un loop hasta que deba frenar.
 * 
 * Devuelve el estado de finalización.
*/
estado_t main_loop(juego_t *juego, void *estructura, pantalla_t *pantalla,
		   unsigned frames_por_segundo)
{
	uint64_t ms_por_frame = 1000 / frames_por_segundo;

	uint64_t tiempo_frame = 0;
	uint64_t tiempo_antes, tiempo_muerto;

	bool debug = false;

	estado_t estado;
	bool finalizar;
	while (true) {
		tiempo_antes = ms_actuales();

		int input = leer_caracter();
		if (input == T_DEBUG)
			debug = !debug;

		finalizar = juego->procesar_eventos(estructura, input,
						    tiempo_frame, &estado);
		if (finalizar)
			break;

		if (interrupcion) {
			estado = SENIAL_INTERRUPCION;
			break;
		}

		juego->dibujar_graficos(estructura, pantalla);
		if (debug) {
			pantalla_color_texto(pantalla, 127, 127, 127, 0.5f);
			pantalla_color_fondo(pantalla, 0, 0, 0, 0.0f);
			pantalla_estilo_texto(pantalla, false, false, false);
			pantalla_texto(pantalla, 0, 0, "Tiempo frame: %d ms",
				       tiempo_frame);
		}
		pantalla_actualizar_frame(pantalla);

		tiempo_frame = ms_actuales() - tiempo_antes;

		if (tiempo_frame < ms_por_frame) {
			tiempo_muerto = ms_por_frame - tiempo_frame;
			sleep_ms(tiempo_muerto);
			tiempo_frame += tiempo_muerto;
		}
	}

	return estado;
}

estado_t motor_ejecutar_juego(juego_t *juego, void *config_juego)
{
	if (juego == NULL)
		return PUNTERO_NULL;

	if (!juego_valido(juego))
		return JUEGO_INVALIDO;

	motor_config_t config_motor = juego->config_motor();
	if (config_motor.frames_por_segundo == 0)
		return CONFIGURACION_INVALIDA;

	estado_t estado_juego;
	void *juego_struct = juego->iniciar(config_juego, &estado_juego);
	if (juego_struct == NULL) {
		return estado_juego;
	}

	estado_t estado_pantalla;
	pantalla_t *pantalla = pantalla_crear(config_motor.ancho_pantalla,
					      config_motor.alto_pantalla,
					      &estado_pantalla);
	if (pantalla == NULL) {
		juego->finalizar(juego_struct);
		return estado_pantalla;
	}

	signal(SIGINT, handler_interrupcion);

	srand((unsigned)ms_actuales());
	deshabilitar_echo_terminal();
	estado_t finalizacion = main_loop(juego, juego_struct, pantalla,
					  config_motor.frames_por_segundo);
	juego->finalizar(juego_struct);
	habilitar_echo_terminal();

	signal(SIGINT, SIG_DFL);

	pantalla_destruir(pantalla);

	return finalizacion;
}
