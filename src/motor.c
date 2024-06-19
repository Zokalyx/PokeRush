#include "motor.h"

#include "motor_input.h"
#include "motor_pantalla.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

#define T_DEBUG '\t'

// Se hace esto para que Ctrl-C cierre el juego
// de manera correcta, liberando toda memoria.
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
estado_t main_loop(juego_t *juego, void *estructura, pantalla_t *pantalla)
{
	uint64_t frames_transcurridos = 0;

	bool debug = false;

	estado_t estado;
	bool finalizar;
	while (true) {
		int input = leer_caracter();
		if (input == T_DEBUG)
			debug = !debug;

		finalizar = juego->procesar_eventos(estructura, input, &estado);
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
			pantalla_texto(pantalla, 0, 0, "Frames: %d",
				       frames_transcurridos);
		}

		pantalla_actualizar_frame(pantalla);
		frames_transcurridos++;
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
	deshabilitar_echo_terminal();

	estado_t finalizacion = main_loop(juego, juego_struct, pantalla);
	juego->finalizar(juego_struct);

	habilitar_echo_terminal();
	signal(SIGINT, SIG_DFL);

	pantalla_destruir(pantalla);

	return finalizacion;
}
