#include "pokerush_carrera.h"
#include "pokerush_macros.h"

#include "motor_estado.h"
#include "motor_pantalla.h"
#include "motor_input.h"
#include "motor_animacion.h"

#include "hash.h"
#include "tp.h"
#include "split.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define D_TICK (500 / 30)
#define C_TEXTO 25, 25, 25
#define B_SUMA_TIEMPO 200, 100, 50
#define B_TIEMPO_NORMAL 255, 255, 255

#define TICKS_END 1

#define Y_POKE 4
#define X_POKE1 8
#define X_POKE2 55

#define X_PISTA1 X_POKE1 - X_POKEMON / 2
#define X_PISTA2 X_POKE2 - X_POKEMON / 2

#define Y_TIEMPO 2
#define X_TIEMPO1 X_POKE1
#define X_TIEMPO2 X_POKE2

#define FRAMES_PISTA 4
#define D_FRAME_PISTA (D_TICK / 4)

#define Y_OBSTACULO 6
#define Y_DELTA_OBSTACULO 18
#define X_DELTA_OBSTACULO 10
#define X_OBSTACULO1 11
#define X_OBSTACULO2 58

#define CANTIDAD_COUNTDOWN 3
#define TICKS_NUMERO_COUNTDOWN 2
#define X_COUNTDOWN ANCHO_PANTALLA / 2 - 16 / 2 + 2
#define Y_COUNTDOWN ALTO_PANTALLA / 2 - 8 / 2 - 2

#define P_EFECTIVO1 X_TIEMPO1 - 6, Y_TIEMPO + 2
#define P_EFECTIVO2 X_TIEMPO2 - 6, Y_TIEMPO + 2
#define C_EFECTIVO 100, 0, 100
#define M_EFECTIVO "Super efectivo!"

typedef struct jugador {
	// Sprite.
	void *pokemon;
	// Cuánto tarda el pokémon por cada obstáculo.
	// (sí, ya se sabe antes de siquiera comenzar la carrera)
	unsigned *tiempos;
	// Aumenta dinámicamente a medida que avanza la carrera.
	unsigned tiempo_total;
	// Obstáculos en formato string.
	char *obstaculos;
	size_t cantidad_obstaculos;
	// Comienza en -1 antes de estar en ningún obstáculo.
	int obstaculo_actual;
	// Mostrar la animación de movimiento
	bool corriendo;
	// Llegó a la meta.
	bool finalizo;
	// El TICK anterior estuvo en un obstáculo y aumentó el tiempo gastado.
	bool aumento_tiempo;
	// El TICK anterior atravesó un obstáculo con 0 tiempo utilizado.
	bool super_efectivo;
} jugador_t;

typedef struct escenario {
	// La carrera se maneja en TICKs, aproximadamente equivalente a
	// segundos.
	unsigned tick_actual;
	jugador_t *jugador1, *jugador2;
	// Sprites para animar el movimiento de la pista.
	void *pista[FRAMES_PISTA];
	// Sprites para animar el countdown al comienzo.
	void *countdown[CANTIDAD_COUNTDOWN];
	// Sprites varios.
	void *meta, *pared, *tunel_front, *tunel_back, *escombros_back,
		*puerta_cerrada, *puerta_abierta_front, *puerta_abierta_back,
		*escombros_front;
	// Ambos pokémon finalizaron.
	bool finalizado;
	// Valor auxiliar para manejar la animación final.
	uint64_t tiempo_final;
} escenario_t;

/**
 * Parsea los resultados del TDA TP para saber
 * los tiempos individuales de cáda obstáculo y
 * sus tipos.
*/
jugador_t *obtener_info_jugador(TP *tp, hash_t *sprites,
				enum TP_JUGADOR jugador)
{
	char *obstaculos = tp_obstaculos_pista(tp, jugador);
	if (obstaculos == NULL)
		return NULL;

	size_t cantidad = strlen(obstaculos);

	char *tiempos_csv = tp_tiempo_por_obstaculo(tp, jugador);
	if (tiempos_csv == NULL) {
		free(obstaculos);
		return NULL;
	}

	char **tiempos_split = split(tiempos_csv, ',');
	free(tiempos_csv);
	if (tiempos_split == NULL) {
		free(obstaculos);
		return NULL;
	}

	unsigned *tiempos = malloc(cantidad * sizeof(unsigned));
	if (tiempos == NULL) {
		free(obstaculos);
		free(tiempos_split);
		return NULL;
	}

	for (int i = 0; i < cantidad; i++) {
		tiempos[i] = (unsigned)atoi(tiempos_split[i]);
		free(tiempos_split[i]);
	}
	free(tiempos_split);

	jugador_t *jugador_struct = malloc(sizeof(jugador_t));
	if (jugador_struct == NULL) {
		free(obstaculos);
		free(tiempos);
		return NULL;
	}

	jugador_struct->obstaculos = obstaculos;
	jugador_struct->tiempos = tiempos;
	jugador_struct->pokemon = hash_obtener(
		sprites, tp_pokemon_seleccionado(tp, jugador)->nombre);
	jugador_struct->tiempo_total = 0;
	jugador_struct->corriendo = false;
	jugador_struct->obstaculo_actual = -1;
	jugador_struct->cantidad_obstaculos = cantidad;
	jugador_struct->finalizo = false;
	jugador_struct->aumento_tiempo = false;
	jugador_struct->super_efectivo = false;

	return jugador_struct;
}

/**
 * Actualiza el estado del jugador, tick a tick.
 * 
 * Si está en un obstáculo, le reduce su tiempo en 1.
 * 
 * Si llegó a 0, se pone a correr para llegar al siguiente
 * obstáculo.
 * 
 * Si ya estaba corriendo, se pone con el obstáculo.
 * 
 * Si finalizó, no hace nada.
*/
void tick_jugador(jugador_t *jugador)
{
	int i = jugador->obstaculo_actual;

	if (jugador->super_efectivo)
		jugador->super_efectivo = false;

	if (jugador->corriendo) {
		// Recién NO estuvo con un obstáculo,
		// así que ahora se topa con uno.
		jugador->aumento_tiempo = false;
		jugador->corriendo = false;

		// Verificar si en realidad podemos seguir de largo
		// gracias a una super efectividad.
		if (!jugador->finalizo && jugador->tiempos[i] == 0) {
			jugador->corriendo = true;
			jugador->super_efectivo = true;
			jugador->obstaculo_actual++;
			if (i == jugador->cantidad_obstaculos - 1)
				jugador->finalizo = true;
		}

		return;
	}

	if (jugador->finalizo)
		return;

	// Recien empieza
	if (i == -1) {
		jugador->obstaculo_actual++;
		jugador->corriendo = true;
		return;
	}

	// Si no estaba corriendo significa que
	// estaba con un obstáculo, y con tiempo
	// restante > 0
	jugador->aumento_tiempo = true;
	jugador->tiempo_total++;
	jugador->tiempos[i]--;

	if (jugador->tiempos[i] == 0) {
		// Terminó el obstáculo
		jugador->corriendo = true;
		jugador->obstaculo_actual++;
		if (i == jugador->cantidad_obstaculos - 1)
			jugador->finalizo = true;
	}
}

/**
 * Libera la memoria reservada por el jugador_t
*/
void liberar_jugador(jugador_t *jugador)
{
	free(jugador->obstaculos);
	free(jugador->tiempos);
	free(jugador);
}

void *pr_carrera_iniciar(struct pr_contexto *contexto)
{
	escenario_t *escenario = malloc(sizeof(escenario_t));
	if (escenario == NULL)
		return NULL;

	hash_t *sprites = contexto->sprites;

	escenario->jugador1 =
		obtener_info_jugador(contexto->tp, sprites, JUGADOR_1);
	if (escenario->jugador1 == NULL) {
		free(escenario);
		return NULL;
	}

	escenario->jugador2 =
		obtener_info_jugador(contexto->tp, sprites, JUGADOR_2);
	if (escenario->jugador2 == NULL) {
		liberar_jugador(escenario->jugador1);
		free(escenario);
		return NULL;
	}

	escenario->tick_actual = 0;
	escenario->finalizado = false;

	// SPRITES.

	escenario->pista[0] = hash_obtener(sprites, "pista1");
	escenario->pista[1] = hash_obtener(sprites, "pista2");
	escenario->pista[2] = hash_obtener(sprites, "pista3");
	escenario->pista[3] = hash_obtener(sprites, "pista4");

	escenario->countdown[0] = hash_obtener(sprites, "three");
	escenario->countdown[1] = hash_obtener(sprites, "two");
	escenario->countdown[2] = hash_obtener(sprites, "one");

	escenario->meta = hash_obtener(sprites, "meta");

	escenario->pared = hash_obtener(sprites, "pared");
	escenario->escombros_back = hash_obtener(sprites, "escombros_back");
	escenario->escombros_front = hash_obtener(sprites, "escombros_front");

	escenario->tunel_back = hash_obtener(sprites, "tunel_back");
	escenario->tunel_front = hash_obtener(sprites, "tunel_front");

	escenario->puerta_cerrada = hash_obtener(sprites, "puerta_cerrada");
	escenario->puerta_abierta_back =
		hash_obtener(sprites, "puerta_abierta_back");
	escenario->puerta_abierta_front =
		hash_obtener(sprites, "puerta_abierta_front");

	return escenario;
}

enum pr_nombre_escena pr_carrera_eventos(void *escenario_void, int input,
					 struct pr_contexto *contexto,
					 estado_t *estado)
{
	escenario_t *escenario = escenario_void;

	uint64_t t = contexto->frames_escena;

	// Verificar si finalizamos.
	if (!escenario->finalizado && escenario->jugador1->finalizo &&
	    escenario->jugador2->finalizo && !escenario->jugador1->corriendo &&
	    !escenario->jugador2->corriendo) {
		escenario->finalizado = true;
		escenario->tiempo_final = t;

	} else if (escenario->finalizado &&
		   t - escenario->tiempo_final > D_TICK * TICKS_END) {
		// Verificar si terminó animación final.
		return POKERUSH_GANADOR;
	}

	// Countdown, no hacer nada.
	if (t < D_TICK * TICKS_NUMERO_COUNTDOWN * CANTIDAD_COUNTDOWN)
		return POKERUSH_CARRERA;

	// Verificar si estámos en un nuevo tick.
	unsigned tick_real =
		(unsigned)(t / D_TICK -
			   TICKS_NUMERO_COUNTDOWN * CANTIDAD_COUNTDOWN);
	if (tick_real == escenario->tick_actual)
		// Ya estamos actualizados.
		return POKERUSH_CARRERA;

	// Finalmente, actualizemos.
	escenario->tick_actual = tick_real;
	tick_jugador(escenario->jugador1);
	tick_jugador(escenario->jugador2);

	return POKERUSH_CARRERA;
}

/**
 * Dibuja los contadores de tiempo utilizado para
 * atravesar obstáculos, para un solo jugador
*/
void dibujar_timers(jugador_t *jugador, pantalla_t *pantalla, uint64_t tiempo,
		    int pos_x, float opacidad)
{
	float progreso;
	pantalla_color_texto(pantalla, C_TEXTO, opacidad);
	pantalla_estilo_texto(pantalla, true, false, false);

	if (jugador->aumento_tiempo)
		progreso = linear(tiempo % D_TICK, 0, D_TICK, 1, 0);
	else
		progreso = 0.0f;

	color_t color = color_mezcla(color_crear(B_SUMA_TIEMPO),
				     color_crear(B_TIEMPO_NORMAL), progreso);
	pantalla_color_fondo(pantalla, color.r, color.g, color.b, opacidad);
	pantalla_rectangulo(pantalla, pos_x - 2, Y_TIEMPO - 1,
			    5 + (jugador->tiempo_total >= 10 ? 1 : 0), 3, ' ');

	pantalla_color_fondo(pantalla, C_TRANSPARENTE);
	pantalla_texto(pantalla, pos_x, Y_TIEMPO, "%d", jugador->tiempo_total);
}

/**
 * Dibuja las pistas de los pokemones, con la animación de movimiento
 * incluida. Para un jugador solo.
*/
void dibujar_pista(void **pista, jugador_t *jugador, pantalla_t *pantalla,
		   uint64_t tiempo, int pos_x, float opacidad)
{
	size_t indice_frame;

	// Jugador 1
	if (jugador->corriendo)
		indice_frame = (tiempo / D_FRAME_PISTA) % FRAMES_PISTA;
	else
		indice_frame = 0;
	pantalla_sprite(pantalla, pos_x, 0, pista[indice_frame], opacidad);
}

/**
 * Dibuja los obstáculos por atravesar y el atravesado para un jugador.
 * Solo la capa de atrás.
*/
void dibujar_obstaculos_back(escenario_t *escenario, jugador_t *jugador,
			     pantalla_t *pantalla, uint64_t tiempo, int x_pos,
			     float opacidad)
{
	int obstaculo_actual = jugador->obstaculo_actual;
	int y_offset =
		(int)linear(tiempo % D_TICK, 0, D_TICK, Y_DELTA_OBSTACULO, 0);
	int x_offset =
		(int)linear(tiempo % D_TICK, 0, D_TICK, X_DELTA_OBSTACULO, 0);

	if (jugador->corriendo) {
		void *atravesado = NULL;

		if (obstaculo_actual == 0) {
			atravesado = escenario->meta;
		} else {
			switch (jugador->obstaculos[obstaculo_actual - 1]) {
			case IDENTIFICADOR_OBSTACULO_FUERZA:
				atravesado = escenario->escombros_back;
				break;
			case IDENTIFICADOR_OBSTACULO_DESTREZA:
				atravesado = escenario->tunel_back;
				break;
			case IDENTIFICADOR_OBSTACULO_INTELIGENCIA:
				atravesado = escenario->puerta_abierta_back;
			}
		}

		float opacidad_back = linear(tiempo % D_TICK, 0, D_TICK, 1, 0);

		pantalla_sprite(pantalla, x_pos - x_offset,
				Y_OBSTACULO + (y_offset - Y_DELTA_OBSTACULO),
				atravesado, opacidad_back);
	}

	void *actual = NULL;
	if (obstaculo_actual == -1 || jugador->finalizo)
		actual = escenario->meta;
	else
		switch (jugador->obstaculos[obstaculo_actual]) {
		case IDENTIFICADOR_OBSTACULO_FUERZA:
			actual = NULL;
			break;
		case IDENTIFICADOR_OBSTACULO_DESTREZA:
			actual = escenario->tunel_back;
			break;
		case IDENTIFICADOR_OBSTACULO_INTELIGENCIA:
			actual = NULL;
		}

	if (!jugador->corriendo) {
		y_offset = 0;
		x_offset = 0;
	}

	pantalla_sprite(pantalla, x_pos - x_offset - X_DELTA_OBSTACULO,
			Y_OBSTACULO + y_offset, actual, opacidad);
}

/**
 * Dibuja los obstáculos por atravesar y el atravesado para un jugador.
 * Solo la capa de adelante (en frente del pokémon)
*/
void dibujar_obstaculos_front(escenario_t *escenario, jugador_t *jugador,
			      pantalla_t *pantalla, uint64_t tiempo, int x_pos,
			      float opacidad)
{
	int obstaculo_actual = jugador->obstaculo_actual;
	int y_offset =
		(int)linear(tiempo % D_TICK, 0, D_TICK, Y_DELTA_OBSTACULO, 0);
	int x_offset =
		(int)linear(tiempo % D_TICK, 0, D_TICK, X_DELTA_OBSTACULO, 0);

	if (jugador->corriendo) {
		void *atravesado = NULL;

		if (obstaculo_actual == 0) {
			atravesado = NULL;
		} else {
			switch (jugador->obstaculos[obstaculo_actual - 1]) {
			case IDENTIFICADOR_OBSTACULO_FUERZA:
				atravesado = escenario->escombros_front;
				break;
			case IDENTIFICADOR_OBSTACULO_DESTREZA:
				atravesado = escenario->tunel_front;
				break;
			case IDENTIFICADOR_OBSTACULO_INTELIGENCIA:
				atravesado = escenario->puerta_abierta_front;
			}
		}

		float opacidad_front = linear(tiempo % D_TICK, 0, D_TICK, 1, 0);

		pantalla_sprite(pantalla, x_pos - x_offset,
				Y_OBSTACULO + (y_offset - Y_DELTA_OBSTACULO),
				atravesado, opacidad_front);
	}

	void *actual = NULL;
	if (obstaculo_actual == -1 || jugador->finalizo)
		actual = escenario->meta;
	else
		switch (jugador->obstaculos[obstaculo_actual]) {
		case IDENTIFICADOR_OBSTACULO_FUERZA:
			actual = escenario->pared;
			break;
		case IDENTIFICADOR_OBSTACULO_DESTREZA:
			actual = escenario->tunel_front;
			break;
		case IDENTIFICADOR_OBSTACULO_INTELIGENCIA:
			actual = escenario->puerta_cerrada;
		}

	if (!jugador->corriendo) {
		y_offset = 0;
		x_offset = 0;
	}

	pantalla_sprite(pantalla, x_pos - x_offset - X_DELTA_OBSTACULO,
			Y_OBSTACULO + y_offset, actual, opacidad);
}

void pr_carrera_graficos(void *escenario_void, pantalla_t *pantalla,
			 struct pr_contexto *contexto)
{
	escenario_t *escenario = escenario_void;

	float opacidad = linear(contexto->frames_escena, 0,
				D_TICK * TICKS_NUMERO_COUNTDOWN, 0, 1);

	// Fondo
	pantalla_color_fondo(pantalla, B_CARRERA, OPACIDAD_FONDO / 100.0f);
	pantalla_fondo(pantalla);

	// Pista
	dibujar_pista(escenario->pista, escenario->jugador1, pantalla,
		      contexto->frames_escena, X_PISTA1, opacidad);
	dibujar_pista(escenario->pista, escenario->jugador2, pantalla,
		      contexto->frames_escena, X_PISTA2, opacidad);

	// Obstáculos que se ven por atrás del pokemon
	dibujar_obstaculos_back(escenario, escenario->jugador1, pantalla,
				contexto->frames_escena, X_OBSTACULO1,
				opacidad);
	dibujar_obstaculos_back(escenario, escenario->jugador2, pantalla,
				contexto->frames_escena, X_OBSTACULO2,
				opacidad);

	// Pokemones
	pantalla_sprite(pantalla, X_POKE1, Y_POKE, escenario->jugador1->pokemon,
			opacidad);
	pantalla_sprite(pantalla, X_POKE2, Y_POKE, escenario->jugador2->pokemon,
			opacidad);

	// Obstáculos que se ven por en frente del pokemon
	dibujar_obstaculos_front(escenario, escenario->jugador1, pantalla,
				 contexto->frames_escena, X_OBSTACULO1,
				 opacidad);
	dibujar_obstaculos_front(escenario, escenario->jugador2, pantalla,
				 contexto->frames_escena, X_OBSTACULO2,
				 opacidad);

	// Countdown
	if (contexto->frames_escena <
	    D_TICK * TICKS_NUMERO_COUNTDOWN * CANTIDAD_COUNTDOWN) {
		size_t idx = (contexto->frames_escena / D_TICK) /
			     TICKS_NUMERO_COUNTDOWN;
		pantalla_sprite(pantalla, X_COUNTDOWN, Y_COUNTDOWN,
				escenario->countdown[idx], 1.0f);
	}

	// Timers
	dibujar_timers(escenario->jugador1, pantalla, contexto->frames_escena,
		       X_TIEMPO1, opacidad);
	dibujar_timers(escenario->jugador2, pantalla, contexto->frames_escena,
		       X_TIEMPO2, opacidad);

	float opacidad_super_efectivo =
		linear(contexto->frames_escena % D_TICK, 0, D_TICK, 1, 0);
	pantalla_color_texto(pantalla, C_EFECTIVO, opacidad_super_efectivo);
	if (escenario->jugador1->super_efectivo)
		pantalla_texto(pantalla, P_EFECTIVO1, M_EFECTIVO);
	if (escenario->jugador2->super_efectivo)
		pantalla_texto(pantalla, P_EFECTIVO2, M_EFECTIVO);

	// Transición final
	if (escenario->finalizado &&
	    contexto->frames_escena > escenario->tiempo_final) {
		int x_externo = ease_in_out(
			contexto->frames_escena - escenario->tiempo_final, 0,
			D_TICK * TICKS_END, 0, ANCHO_PANTALLA / 2);

		pantalla_color_fondo(pantalla, C_NORMAL, 1.0f);
		pantalla_rectangulo(pantalla, ANCHO_PANTALLA / 2 - x_externo, 0,
				    2 * (unsigned)x_externo, ALTO_PANTALLA,
				    ' ');
	}
}

void pr_carrera_destruir(void *escenario_void)
{
	escenario_t *escenario = escenario_void;

	liberar_jugador(escenario->jugador1);
	liberar_jugador(escenario->jugador2);
	free(escenario);
}

struct pr_escena pr_carrera()
{
	return (struct pr_escena){
		.iniciar = pr_carrera_iniciar,
		.procesar_eventos = pr_carrera_eventos,
		.dibujar_graficos = pr_carrera_graficos,
		.destruir = pr_carrera_destruir,
	};
}
