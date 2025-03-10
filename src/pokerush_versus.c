#include "pokerush_versus.h"
#include "pokerush_macros.h"

#include "motor_estado.h"
#include "motor_pantalla.h"
#include "motor_input.h"
#include "motor_animacion.h"

#include "hash.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define D_VERSUS 1000
#define D_FAST1 1000
#define D_SLOW 2000
#define D_FAST2 1500
#define D_RECT 1000

typedef struct escenario {
	int x_nombre1, x_nombre2;
	const char *nombre2;
	void *sprite_pokemon1, *sprite_pokemon2;
	void *sprite_entrenador1, *sprite_entrenador2;
} escenario_t;

void *pr_versus_iniciar(struct pr_contexto *contexto)
{
	escenario_t *escenario = malloc(sizeof(escenario_t));
	if (escenario == NULL)
		return NULL;

	escenario->x_nombre1 = ANCHO_PANTALLA / 4 -
			       (int)strlen(contexto->nombre_entrenador) / 2;
	escenario->x_nombre2 = ANCHO_PANTALLA * 3 / 4 - 2;

	switch (contexto->dificultad) {
	case FACIL:
		escenario->nombre2 = M_FACIL;
		escenario->sprite_entrenador2 =
			hash_obtener(contexto->sprites, M_FACIL);
		break;
	case MEDIO:
		escenario->nombre2 = M_MEDIO;
		escenario->sprite_entrenador2 =
			hash_obtener(contexto->sprites, M_MEDIO);
		break;
	case DIFICIL:
		escenario->nombre2 = M_DIFICIL;
		escenario->sprite_entrenador2 =
			hash_obtener(contexto->sprites, M_DIFICIL);
		break;
	case IMPOSIBLE:
		escenario->nombre2 = M_IMPOSIBLE;
		escenario->sprite_entrenador2 =
			hash_obtener(contexto->sprites, M_IMPOSIBLE);
		break;
	}

	escenario->sprite_entrenador1 =
		hash_obtener(contexto->sprites, "jugador");

	const struct pokemon_info *pokemon1 =
		tp_pokemon_seleccionado(contexto->tp, JUGADOR_1);
	escenario->sprite_pokemon1 =
		hash_obtener(contexto->sprites, pokemon1->nombre);

	const struct pokemon_info *pokemon2 =
		tp_pokemon_seleccionado(contexto->tp, JUGADOR_2);
	escenario->sprite_pokemon2 =
		hash_obtener(contexto->sprites, pokemon2->nombre);

	return escenario;
}

enum pr_nombre_escena pr_versus_eventos(void *escenario_void, int input,
					struct pr_contexto *contexto,
					estado_t *estado)
{
	if (contexto->tiempo_escena_ms > D_FAST1 + D_FAST2 + D_SLOW + D_RECT)
		return POKERUSH_CARRERA;

	return POKERUSH_VERSUS;
}

void pr_versus_graficos(void *escenario_void, pantalla_t *pantalla,
			struct pr_contexto *contexto)
{
	escenario_t *escenario = escenario_void;

	uint64_t t = contexto->tiempo_escena_ms;

	// Fondo
	float opacidad_fondo =
		(linear(contexto->tiempo_escena_ms, 0, D_TRANSICION_FONDO,
			OPACIDAD_FONDO, 100) -
		 linear(contexto->tiempo_escena_ms, D_TRANSICION_FONDO,
			2 * D_TRANSICION_FONDO, 0,
			(100 - OPACIDAD_FONDO) / 2)) /
		100.0f;
	if (t > D_FAST1 + D_SLOW + (D_FAST2 + D_RECT) / 2)
		pantalla_color_fondo(pantalla, B_CARRERA, opacidad_fondo);
	else
		pantalla_color_fondo(pantalla, B_PRINCIPAL, opacidad_fondo);
	pantalla_fondo(pantalla);

	// Nombres
	int y_versus =
		ease_in_out(t, 0, D_FAST1, 0, (Y_NOMBRE_END - Y_NOMBRE_START));

	int x_nombre_extra;
	if (t < D_FAST1)
		x_nombre_extra = (int)linear(t, 0, D_FAST1, 0, 15);
	else if (t < D_FAST1 + D_SLOW)
		x_nombre_extra =
			(int)linear(t, D_FAST1, D_FAST1 + D_SLOW, 15, 30);
	else
		x_nombre_extra = ease_in_out(t, D_FAST1 + D_SLOW,
					     D_FAST1 + D_SLOW + D_FAST2, 30,
					     100);

	pantalla_estilo_texto(pantalla, E_NOMBRE);
	pantalla_color_fondo(pantalla, C_TRANSPARENTE);

	pantalla_color_texto(pantalla, C_NOMBRE_1, 1.0f);
	pantalla_texto(pantalla, escenario->x_nombre1 + x_nombre_extra,
		       Y_NOMBRE_START, contexto->nombre_entrenador);

	pantalla_color_texto(pantalla, C_NOMBRE_2, 1.0f);
	pantalla_texto(pantalla, escenario->x_nombre2 - x_nombre_extra,
		       Y_NOMBRE_START + 2 * y_versus, escenario->nombre2);

	float opacidad = linear(t, 0, D_FAST1 / 2, 1, 0);

	pantalla_estilo_texto(pantalla, E_NORMAL);
	pantalla_color_texto(pantalla, C_NORMAL, opacidad);
	pantalla_texto(pantalla, X_VERSUS, Y_NOMBRE_START + y_versus, M_VERSUS);

	if (t > D_FAST1 + D_FAST2 + D_SLOW)
		opacidad = linear(t, D_FAST1 + D_SLOW + D_FAST2,
				  D_FAST1 + D_SLOW + D_FAST2 + D_RECT, 1, 0);
	else
		opacidad = linear(t, D_FAST1, D_FAST1 + D_SLOW, 0, 1);

	// Pokemones y entrenadores
	if (t > D_FAST1 && t < D_FAST1 + D_FAST2 + D_SLOW) {
		pantalla_sprite(pantalla,
				escenario->x_nombre1 + 3 * x_nombre_extra -
					X_POKEMON - 15,
				-1, escenario->sprite_entrenador1,
				opacidad / 4);
		pantalla_sprite(pantalla,
				escenario->x_nombre1 + 3 * x_nombre_extra -
					X_POKEMON - 60,
				-1, escenario->sprite_pokemon1, opacidad / 4);

		pantalla_sprite(pantalla,
				escenario->x_nombre2 - 3 * x_nombre_extra + 75,
				ALTO_PANTALLA / 2, escenario->sprite_pokemon2,
				opacidad / 4);
		pantalla_sprite(pantalla,
				escenario->x_nombre2 - 3 * x_nombre_extra + 30,
				ALTO_PANTALLA / 2,
				escenario->sprite_entrenador2, opacidad / 4);
	}

	// Rectángulos cool 😎
	int pendiente = (int)linear(t, D_FAST1 + D_SLOW,
				    D_FAST1 + D_SLOW + D_FAST2 + D_RECT,
				    ANCHO_PANTALLA, -ANCHO_PANTALLA);
	pantalla_color_fondo(pantalla, C_NORMAL, opacidad);
	for (int i = 0; i < ALTO_PANTALLA; i++)
		pantalla_rect(pantalla, pendiente * (i - Y_NOMBRE_END), i,
			      ANCHO_PANTALLA, 1, ' ');
}

void pr_versus_destruir(void *escenario_void)
{
	escenario_t *escenario = escenario_void;

	free(escenario);
}

struct pr_escena pr_versus()
{
	return (struct pr_escena){
		.iniciar = pr_versus_iniciar,
		.procesar_eventos = pr_versus_eventos,
		.dibujar_graficos = pr_versus_graficos,
		.destruir = pr_versus_destruir,
	};
}
