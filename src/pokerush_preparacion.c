#include "pokerush_preparacion.h"
#include "pokerush_macros.h"

#include "motor_estado.h"
#include "motor_pantalla.h"
#include "motor_input.h"
#include "motor_animacion.h"

#include "hash.h"
#include "split.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define OBSTACULOS_FACIL 3
#define OBSTACULOS_MEDIO 5
#define OBSTACULOS_DIFICIL 7
#define OBSTACULOS_IMPOSIBLE 9
#define NUM_ATRIBUTOS 3

#define POKEMON_MISTERIO "?????"
#define Y_SELECCION 5
#define S_LEFT '<'
#define S_RIGHT '>'

typedef struct seleccion {
	size_t pokemon_idx;
	// Por simplicidad el arreglo tiene siempre OBSTACULOS_IMPOSIBLE
	// y usamos los primeros N elementos que necesitemos.
	size_t cantidad_obstaculos;
	enum TP_OBSTACULO obstaculos[OBSTACULOS_IMPOSIBLE];
	bool obstaculos_activados[OBSTACULOS_IMPOSIBLE];
	bool ocultos[OBSTACULOS_IMPOSIBLE];
} seleccion_t;

typedef struct atributo {
	char letra;
	color_t color;
} atributo_t;

typedef struct escenario {
	const char *nombre2;
	int x_nombre1, x_nombre2;
	seleccion_t seleccion1, seleccion2;
	// -1: Pokemon.
	// >0: Obstáculo.
	int modificando;
	atributo_t atributos[NUM_ATRIBUTOS];
} escenario_t;

/**
 * Elige un pokemon al azar y una pista de obstaculos
 * al azar.
*/
seleccion_t seleccion_azar(size_t cantidad_pokemones,
			   size_t cantidad_obstaculos, bool ocultar)
{
	seleccion_t seleccion;
	seleccion.pokemon_idx = (size_t)rand() % cantidad_pokemones;
	for (int i = 0; i < OBSTACULOS_IMPOSIBLE; i++) {
		switch (rand() % NUM_ATRIBUTOS) {
		case 0:
			seleccion.obstaculos[i] = OBSTACULO_FUERZA;
			break;
		case 1:
			seleccion.obstaculos[i] = OBSTACULO_INTELIGENCIA;
			break;
		case 2:
			seleccion.obstaculos[i] = OBSTACULO_DESTREZA;
			break;
		}

		seleccion.ocultos[i] = ocultar ? i % 2 != 0 : false;
		seleccion.obstaculos_activados[i] = i < cantidad_obstaculos;
	}
	seleccion.cantidad_obstaculos = cantidad_obstaculos;

	return seleccion;
}

/**
 * Carga lo seleccionado previamente en el TP.
*/
seleccion_t seleccion_reintento(TP *tp, const struct pokemon_info **pokemones,
				size_t cantidad_pokemones,
				enum TP_JUGADOR jugador, bool ocultar)
{
	seleccion_t seleccion;
	const struct pokemon_info *pokemon =
		tp_pokemon_seleccionado(tp, jugador);
	for (size_t i = 0; i < cantidad_pokemones; i++) {
		if (pokemon == pokemones[i])
			seleccion.pokemon_idx = i;
	}
	char *obstaculos = tp_obstaculos_pista(tp, jugador);
	if (obstaculos == NULL)
		// No crashear, solo utilizar una nueva configuracion.
		return seleccion_azar(cantidad_pokemones, 3, false);

	size_t cantidad_obstaculos = 0;
	for (size_t i = 0; obstaculos[i] != 0; i++) {
		switch (obstaculos[i]) {
		case IDENTIFICADOR_OBSTACULO_DESTREZA:
			seleccion.obstaculos[i] = OBSTACULO_DESTREZA;
			break;
		case IDENTIFICADOR_OBSTACULO_FUERZA:
			seleccion.obstaculos[i] = OBSTACULO_FUERZA;
			break;
		case IDENTIFICADOR_OBSTACULO_INTELIGENCIA:
			seleccion.obstaculos[i] = OBSTACULO_INTELIGENCIA;
			break;
		}
		seleccion.obstaculos_activados[i] = true;
		seleccion.ocultos[i] = ocultar ? i % 2 != 0 : false;
		cantidad_obstaculos++;
	}

	for (size_t i = cantidad_obstaculos; i < OBSTACULOS_IMPOSIBLE; i++) {
		seleccion.obstaculos_activados[i] = false;
		seleccion.ocultos[i] = ocultar ? i % 2 != 0 : false;
	}

	free(obstaculos);

	seleccion.cantidad_obstaculos = cantidad_obstaculos;

	return seleccion;
}

/**
 * Determina si se puede proceder a la carrera.
 * 
 * - La cantidad de obstáculos debe ser mayor a 0.
*/
bool seleccion_valida(seleccion_t *seleccion)
{
	for (int i = 0; i < OBSTACULOS_IMPOSIBLE; i++) {
		if (seleccion->obstaculos_activados[i])
			return true;
	}

	return false;
}

void *pr_preparacion_iniciar(struct pr_contexto *contexto)
{
	// Solo se puede jugar con 2 pokemon o más
	if (contexto->cantidad_pokemones < 2)
		return NULL;

	escenario_t *escenario = malloc(sizeof(escenario_t));
	if (escenario == NULL)
		return NULL;

	escenario->x_nombre1 = ANCHO_PANTALLA / 4 -
			       (int)strlen(contexto->nombre_entrenador) / 2;
	escenario->x_nombre2 = ANCHO_PANTALLA * 3 / 4 - 2;

	size_t obstaculos_cpu = 0;
	switch (contexto->dificultad) {
	case FACIL:
		obstaculos_cpu = OBSTACULOS_FACIL;
		escenario->nombre2 = M_FACIL;
		break;
	case MEDIO:
		obstaculos_cpu = OBSTACULOS_MEDIO;
		escenario->nombre2 = M_MEDIO;
		break;
	case DIFICIL:
		obstaculos_cpu = OBSTACULOS_DIFICIL;
		escenario->nombre2 = M_DIFICIL;
		break;
	case IMPOSIBLE:
		obstaculos_cpu = OBSTACULOS_IMPOSIBLE;
		escenario->nombre2 = M_IMPOSIBLE;
		break;
	}

	escenario->atributos[0].color = color_crear(C_FUERZA);
	escenario->atributos[0].letra = IDENTIFICADOR_OBSTACULO_FUERZA;
	escenario->atributos[1].color = color_crear(C_DESTREZA);
	escenario->atributos[1].letra = IDENTIFICADOR_OBSTACULO_DESTREZA;
	escenario->atributos[2].color = color_crear(C_INTELIGENCIA);
	escenario->atributos[2].letra = IDENTIFICADOR_OBSTACULO_INTELIGENCIA;

	if (contexto->es_reintento)
		// Reutilizar la misma configuración
		escenario->seleccion2 = seleccion_reintento(
			contexto->tp, contexto->pokemones,
			contexto->cantidad_pokemones, JUGADOR_2, true);
	else
		escenario->seleccion2 = seleccion_azar(
			contexto->cantidad_pokemones, obstaculos_cpu, true);

	if (contexto->es_reintento)
		escenario->seleccion1 = seleccion_reintento(
			contexto->tp, contexto->pokemones,
			contexto->cantidad_pokemones, JUGADOR_1, false);
	else
		escenario->seleccion1 = seleccion_azar(
			contexto->cantidad_pokemones, obstaculos_cpu, false);

	escenario->modificando = contexto->es_reintento ? 0 : -1;

	// Evitar que sean el mismo pokemon
	if (escenario->seleccion1.pokemon_idx ==
	    escenario->seleccion2.pokemon_idx)
		escenario->seleccion1.pokemon_idx =
			(escenario->seleccion1.pokemon_idx + 1) %
			contexto->cantidad_pokemones;

	// Limpiar pista vieja si existe
	tp_limpiar_pista(contexto->tp, JUGADOR_1);
	tp_limpiar_pista(contexto->tp, JUGADOR_2);

	return escenario;
}

/**
 * Guarda la información del escenario en el TP.
 * 
 * TODO: Verificar si falla agregar obstáculos.
*/
void guardar_seleccion_en_tp(TP *tp, const struct pokemon_info **pokemones,
			     seleccion_t seleccion1, seleccion_t seleccion2)
{
	// Pokemones.
	tp_seleccionar_pokemon(tp, JUGADOR_1,
			       pokemones[seleccion1.pokemon_idx]->nombre);
	tp_seleccionar_pokemon(tp, JUGADOR_2,
			       pokemones[seleccion2.pokemon_idx]->nombre);

	// Obstáculos
	for (unsigned i = 0; i < OBSTACULOS_IMPOSIBLE; i++)
		if (seleccion1.obstaculos_activados[i])
			tp_agregar_obstaculo(tp, JUGADOR_1,
					     seleccion1.obstaculos[i], i);
	for (unsigned i = 0; i < seleccion2.cantidad_obstaculos; i++)
		tp_agregar_obstaculo(tp, JUGADOR_2, seleccion2.obstaculos[i],
				     i);
}

enum pr_nombre_escena pr_preparacion_eventos(void *escenario_void, int input,
					     struct pr_contexto *contexto,
					     estado_t *estado)
{
	escenario_t *escenario = escenario_void;

	int modificando = escenario->modificando;

	switch (input) {
	case FLECHA_ABAJO:
		if (modificando < OBSTACULOS_IMPOSIBLE - 1)
			escenario->modificando++;
		break;

	case FLECHA_ARRIBA:
		if (modificando > -1) {
			escenario->modificando--;
			// No se puede cambiar pokemon en reintento
			if (contexto->es_reintento &&
			    escenario->modificando == -1)
				escenario->modificando = 0;
		}
		break;

	case FLECHA_DERECHA: {
		seleccion_t *seleccion = &escenario->seleccion1;
		if (modificando == -1) {
			// Cambiar pokemon (evitar que sean iguales)
			size_t step = (seleccion->pokemon_idx +
				       1) % contexto->cantidad_pokemones ==
						      escenario->seleccion2
							      .pokemon_idx ?
					      2 :
					      1;
			seleccion->pokemon_idx =
				(seleccion->pokemon_idx + step) %
				contexto->cantidad_pokemones;
			break;
		}

		if (!seleccion->obstaculos_activados[modificando]) {
			// Activar
			seleccion->obstaculos[modificando] = 0;
			seleccion->obstaculos_activados[modificando] = true;
			break;
		}

		if (seleccion->obstaculos[modificando] == NUM_ATRIBUTOS - 1) {
			// Desactivar
			seleccion->obstaculos_activados[modificando] = false;
			break;
		}

		// Ciclar
		seleccion->obstaculos[modificando]++;
		break;
	}

	case FLECHA_IZQUIERDA: {
		seleccion_t *seleccion = &escenario->seleccion1;
		if (modificando == -1) {
			// Cambiar pokemon
			size_t step =
				seleccion->pokemon_idx ==
						(escenario->seleccion2
							 .pokemon_idx +
						 1) % contexto->cantidad_pokemones ?
					2 :
					1;
			seleccion->pokemon_idx =
				(seleccion->pokemon_idx +
				 contexto->cantidad_pokemones - step) %
				contexto->cantidad_pokemones;
			break;
		}

		if (!seleccion->obstaculos_activados[modificando]) {
			// Activar
			seleccion->obstaculos[modificando] = NUM_ATRIBUTOS - 1;
			seleccion->obstaculos_activados[modificando] = true;
			break;
		}

		if (seleccion->obstaculos[modificando] == 0) {
			// Desactivar
			seleccion->obstaculos_activados[modificando] = false;
			break;
		}

		// Ciclar
		seleccion->obstaculos[modificando]--;
		break;
	}

	case LINEFEED:
		if (!seleccion_valida(&escenario->seleccion1))
			break;

		guardar_seleccion_en_tp(contexto->tp, contexto->pokemones,
					escenario->seleccion1,
					escenario->seleccion2);
		return POKERUSH_VERSUS;
	}

	return POKERUSH_PREPARACION;
}

void pr_preparacion_graficos(void *escenario_void, pantalla_t *pantalla,
			     struct pr_contexto *contexto)
{
	escenario_t *escenario = escenario_void;

	// Fondo
	float opacidad_fondo =
		(linear(contexto->tiempo_escena_ms, 0, D_TRANSICION_FONDO,
			OPACIDAD_FONDO, 100) -
		 linear(contexto->tiempo_escena_ms, D_TRANSICION_FONDO,
			2 * D_TRANSICION_FONDO, 0, (100 - OPACIDAD_FONDO))) /
		100.0f;
	pantalla_color_fondo(pantalla, B_PRINCIPAL, opacidad_fondo);
	pantalla_fondo(pantalla);

	// Nombres
	pantalla_estilo_texto(pantalla, E_NOMBRE);
	pantalla_color_texto(pantalla, C_NOMBRE_1, 1.0f);
	pantalla_texto(pantalla, escenario->x_nombre1, Y_NOMBRE_START,
		       contexto->nombre_entrenador);

	pantalla_color_texto(pantalla, C_NOMBRE_2, 1.0f);
	pantalla_texto(pantalla, escenario->x_nombre2, Y_NOMBRE_START,
		       escenario->nombre2);

	pantalla_estilo_texto(pantalla, E_NORMAL);
	pantalla_color_texto(pantalla, C_NORMAL, 1.0f);
	pantalla_texto(pantalla, X_VERSUS, Y_NOMBRE_START, M_VERSUS);

	// Pokemon
	const struct pokemon_info *pokemon =
		contexto->pokemones[escenario->seleccion1.pokemon_idx];
	if (escenario->modificando == -1) {
		pantalla_color_texto(pantalla, C_SELECCION, 1.0f);
		pantalla_color_fondo(pantalla, B_SELECCION, 1.0f);
	} else {
		pantalla_color_texto(pantalla, C_NORMAL, 1.0f);
	}

	pantalla_texto(pantalla, escenario->x_nombre1, Y_SELECCION,
		       pokemon->nombre);
	if (escenario->modificando == -1) {
		pantalla_color_fondo(pantalla, C_TRANSPARENTE);
		pantalla_color_texto(pantalla, B_SELECCION, 1.0f);
		pantalla_texto(pantalla, escenario->x_nombre1 - 5, Y_SELECCION,
			       "%c  %c", S_LEFT, S_RIGHT);
	}

	// Obstaculos del jugador
	for (int i = 0; i < OBSTACULOS_IMPOSIBLE; i++) {
		bool activado = escenario->seleccion1.obstaculos_activados[i];
		float opacidad = escenario->modificando == i ? 1.0f : 0.3f;

		for (int j = 0; j < NUM_ATRIBUTOS; j++) {
			color_t c = escenario->atributos[j].color;

			if (activado &&
			    (int)escenario->seleccion1.obstaculos[i] == j) {
				pantalla_color_fondo(pantalla, c.r, c.g, c.b,
						     1.0f);
				pantalla_color_texto(pantalla, C_SELECCION,
						     1.0f);
			} else {
				pantalla_color_fondo(pantalla, C_TRANSPARENTE);
				pantalla_color_texto(pantalla, c.r, c.g, c.b,
						     opacidad);
			}
			pantalla_texto(pantalla, escenario->x_nombre1 + 2 * j,
				       Y_SELECCION + 2 + 2 * i, "%c",
				       escenario->atributos[j].letra);
		}

		if (escenario->modificando == i) {
			pantalla_color_fondo(pantalla, C_TRANSPARENTE);
			pantalla_color_texto(pantalla, B_SELECCION, 1.0f);
			pantalla_texto(pantalla, escenario->x_nombre1 - 2,
				       Y_SELECCION + 2 + 2 * i, "%c", '<');
			pantalla_texto(pantalla, escenario->x_nombre1 + 6,
				       Y_SELECCION + 2 + 2 * i, "%c", '>');
		}
	}

	// Aclaración
	pantalla_estilo_texto(pantalla, E_NORMAL);
	pantalla_color_fondo(pantalla, C_TRANSPARENTE);
	pantalla_color_texto(pantalla, C_NORMAL, 1.0f);
	pantalla_texto(
		pantalla, 10, 25,
		"Recorda que el objetivo es que los pokemones terminen al mismo tiempo");

	// Selección oponente
	pantalla_texto(
		pantalla, escenario->x_nombre2, Y_SELECCION,
		contexto->pokemones[escenario->seleccion2.pokemon_idx]->nombre);

	for (int i = 0; i < escenario->seleccion2.cantidad_obstaculos; i++) {
		if (escenario->seleccion2.ocultos[i]) {
			pantalla_color_texto(pantalla, C_NORMAL, 1.0f);
			pantalla_texto(pantalla, escenario->x_nombre2 + 2,
				       Y_SELECCION + 2 + 2 * i, "?");
		} else {
			size_t atributo =
				(size_t)escenario->seleccion2.obstaculos[i];
			color_t color = escenario->atributos[atributo].color;
			pantalla_color_texto(pantalla, color.r, color.g,
					     color.b, 1.0f);
			pantalla_texto(pantalla, escenario->x_nombre2 + 2,
				       Y_SELECCION + 2 + 2 * i, "%c",
				       escenario->atributos[atributo].letra);
		}
	}

	// Controles
	pantalla_color_texto(pantalla, C_CONTROL, 1.0f);
	pantalla_texto(pantalla, X_CONTROL_1, Y_CONTROL, M_ARRIBA);
	pantalla_texto(pantalla, X_CONTROL_2, Y_CONTROL, M_ABAJO);
	pantalla_texto(pantalla, X_CONTROL_2, Y_CONTROL, M_ABAJO);
	pantalla_texto(pantalla, X_CONTROL_3, Y_CONTROL, M_IZQUIERDA);
	pantalla_texto(pantalla, X_CONTROL_4, Y_CONTROL, M_DERECHA);
	if (seleccion_valida(&escenario->seleccion1))
		pantalla_texto(pantalla, X_CONTROL_5, Y_CONTROL, M_ENTER);
}

void pr_preparacion_destruir(void *escenario_void)
{
	escenario_t *escenario = escenario_void;

	free(escenario);
}

struct pr_escena pr_preparacion()
{
	return (struct pr_escena){
		.iniciar = pr_preparacion_iniciar,
		.procesar_eventos = pr_preparacion_eventos,
		.dibujar_graficos = pr_preparacion_graficos,
		.destruir = pr_preparacion_destruir,
	};
}
