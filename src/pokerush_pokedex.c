#include "pokerush_pokedex.h"
#include "pokerush_macros.h"

#include "motor_estado.h"
#include "motor_pantalla.h"
#include "motor_input.h"
#include "motor_animacion.h"

#include "hash.h"
#include "tp.h"
#include "split.h"
#include <ctype.h>
#include <stdlib.h>

#define M_TITULO "POKEDEX"
#define Y_NOMBRE_1 6
#define X_SPRITE 50
#define Y_SEPARACION_ATRIBUTO 2
#define Y_SEPARACION_LISTA 2
#define Y_SPRITE 2

typedef struct escenario {
	size_t seleccion;
	const void **sprites;
} escenario_t;

void *pr_pokedex_iniciar(struct pr_contexto *contexto)
{
	escenario_t *escenario = malloc(sizeof(escenario_t));
	if (escenario == NULL)
		return NULL;

	const void **sprites =
		malloc(contexto->cantidad_pokemones * sizeof(void *));
	if (sprites == NULL) {
		free(escenario);
		return NULL;
	}

	for (size_t i = 0; i < contexto->cantidad_pokemones; i++) {
		sprites[i] = hash_obtener(contexto->sprites,
					  contexto->pokemones[i]->nombre);
		if (sprites[i] == NULL)
			sprites[i] = hash_obtener(contexto->sprites,
						  SPRITE_POKEMON_DEFAULT);
	}

	escenario->sprites = sprites;
	escenario->seleccion = 0;

	return escenario;
}

enum pr_nombre_escena pr_pokedex_eventos(void *escenario_void, int input,
					 struct pr_contexto *contexto,
					 estado_t *estado)
{
	escenario_t *escenario = escenario_void;

	if (isalpha(input))
		input = toupper(input);

	switch (input) {
	case T_SALIR:
		return POKERUSH_MENU_PRINCIPAL;

	case FLECHA_ARRIBA:
		if (escenario->seleccion > 0)
			escenario->seleccion--;
		break;

	case FLECHA_ABAJO:
		if (escenario->seleccion < contexto->cantidad_pokemones - 1)
			escenario->seleccion++;
		break;
	}

	return POKERUSH_POKEDEX;
}

void pr_pokedex_graficos(void *escenario_void, pantalla_t *pantalla,
			 struct pr_contexto *contexto)
{
	escenario_t *escenario = escenario_void;

	// Fondo
	float opacidad_fondo =
		(linear(contexto->frames_escena, 0, D_TRANSICION_FONDO,
			OPACIDAD_FONDO, 100) -
		 linear(contexto->frames_escena, D_TRANSICION_FONDO,
			2 * D_TRANSICION_FONDO, 0, (100 - OPACIDAD_FONDO))) /
		100.0f;
	pantalla_color_fondo(pantalla, B_PRINCIPAL, opacidad_fondo);
	pantalla_fondo(pantalla);

	// Título
	pantalla_color_texto(pantalla, C_TITULO, 1.0f);
	pantalla_estilo_texto(pantalla, E_TITULO);
	pantalla_texto(pantalla, P_TITULO, M_TITULO);

	// Nombres
	pantalla_color_texto(pantalla, C_NORMAL, 1.0f);
	pantalla_estilo_texto(pantalla, E_NORMAL);
	for (int i = 0; i < contexto->cantidad_pokemones; i++) {
		if (escenario->seleccion == i) {
			pantalla_color_fondo(pantalla, B_SELECCION, 1.0f);
			pantalla_color_texto(pantalla, C_SELECCION, 1.0f);
		}

		int y_nombre = Y_NOMBRE_1 + Y_SEPARACION_LISTA * i;
		pantalla_texto(pantalla, X_MARGEN, y_nombre,
			       contexto->pokemones[i]->nombre);

		if (escenario->seleccion == i) {
			pantalla_color_fondo(pantalla, B_PRINCIPAL, 1.0f);

			pantalla_color_texto(pantalla, B_SELECCION, 1.0f);
			pantalla_texto(pantalla, X_MARGEN - 2, y_nombre, "%c",
				       S_OPCION);
			pantalla_color_texto(pantalla, C_NORMAL, 1.0f);
		}
	}

	// Atributos
	const struct pokemon_info *pokemon =
		contexto->pokemones[escenario->seleccion];
	pantalla_color_texto(pantalla, C_FUERZA, 1.0f);
	int y = Y_SPRITE + Y_POKEMON + Y_SEPARACION_ATRIBUTO;
	int x_left = X_SPRITE + 2;
	int x_right = X_SPRITE + X_POKEMON - 4;
	pantalla_texto(pantalla, x_left, y, M_FUERZA);
	pantalla_texto(pantalla, x_right, y, "%d", pokemon->fuerza);

	y += Y_SEPARACION_ATRIBUTO;
	pantalla_color_texto(pantalla, C_DESTREZA, 1.0f);
	pantalla_texto(pantalla, x_left, y, M_DESTREZA);
	pantalla_texto(pantalla, x_right, y, "%d", pokemon->destreza);

	y += Y_SEPARACION_ATRIBUTO;
	pantalla_color_texto(pantalla, C_INTELIGENCIA, 1.0f);
	pantalla_texto(pantalla, x_left, y, M_INTELIGENCIA);
	pantalla_texto(pantalla, x_right, y, "%d", pokemon->inteligencia);

	// Sprite
	const void *sprite = escenario->sprites[escenario->seleccion];
	pantalla_sprite(pantalla, X_SPRITE, Y_SPRITE, sprite, 1.0f);

	// Controles
	pantalla_color_texto(pantalla, C_CONTROL, 1.0f);
	pantalla_estilo_texto(pantalla, E_CONTROL);
	pantalla_texto(pantalla, X_CONTROL_1, Y_CONTROL, M_ARRIBA);
	pantalla_texto(pantalla, X_CONTROL_2, Y_CONTROL, M_ABAJO);

	pantalla_texto(pantalla, X_SALIR, Y_CONTROL, M_VOLVER);
}

void pr_pokedex_destruir(void *escenario_void)
{
	escenario_t *escenario = escenario_void;

	free(escenario->sprites);
	free(escenario);
}

struct pr_escena pr_pokedex()
{
	return (struct pr_escena){
		.iniciar = pr_pokedex_iniciar,
		.procesar_eventos = pr_pokedex_eventos,
		.dibujar_graficos = pr_pokedex_graficos,
		.destruir = pr_pokedex_destruir,
	};
}
