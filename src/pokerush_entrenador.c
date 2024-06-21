#include "pokerush_entrenador.h"
#include "pokerush_macros.h"

#include "motor_estado.h"
#include "motor_pantalla.h"
#include "motor_input.h"
#include "motor_animacion.h"

#include <stdlib.h>
#include <ctype.h>

#define D_CURSOR_TEXTO 500
#define M_TITULO "ENTRENADOR/A"
#define M_INSTRUCCION "Escribi tu nombre:"
#define Y_INSTRUCCION 10
#define Y_SEPARACION 2
#define P_SPRITE 50, 5
#define M_CURSOR "_"

typedef struct escenario {
	size_t letra_actual;
	void *sprite;
} escenario_t;

void *pr_entrenador_iniciar(struct pr_contexto *contexto)
{
	escenario_t *escenario = malloc(sizeof(escenario_t));
	if (escenario == NULL)
		return NULL;

	escenario->letra_actual = 0;
	escenario->sprite = hash_obtener(contexto->sprites, "jugador");

	return escenario;
}

enum pr_nombre_escena pr_entrenador_eventos(void *escenario_void, int input,
					    struct pr_contexto *contexto,
					    estado_t *estado)
{
	escenario_t *escenario = escenario_void;

	if (isalnum(input) && escenario->letra_actual < LONGITUD_NOMBRE - 1)
		contexto->nombre_entrenador[escenario->letra_actual++] =
			(char)input;

	else if (input == BACKSPACE && escenario->letra_actual > 0)
		contexto->nombre_entrenador[--escenario->letra_actual] = '\0';

	else if (input == LINEFEED && escenario->letra_actual > 0)
		return POKERUSH_MENU_PRINCIPAL;

	return POKERUSH_ENTRENADOR;
}

void pr_entrenador_graficos(void *escenario_void, pantalla_t *pantalla,
			    struct pr_contexto *contexto)
{
	escenario_t *escenario = escenario_void;

	uint64_t t = contexto->frames_escena * contexto->multiplicador_frames;

	// Fondo
	float opacidad_fondo =
		pulso(t, 0, D_TRANSICION_FONDO, OPACIDAD_FONDO, 100) / 100.0f;
	pantalla_color_fondo(pantalla, B_INICIAL, opacidad_fondo);
	pantalla_fondo(pantalla);

	// Título
	pantalla_color_fondo(pantalla, C_TRANSPARENTE);
	pantalla_color_texto(pantalla, C_TITULO, 1.0f);
	pantalla_estilo_texto(pantalla, E_TITULO);
	pantalla_texto(pantalla, P_TITULO, M_TITULO);
	pantalla_estilo_texto(pantalla, E_NORMAL);

	// Instrucción
	pantalla_color_texto(pantalla, C_NORMAL, 1.0f);
	pantalla_estilo_texto(pantalla, E_NORMAL);
	pantalla_texto(pantalla, X_MARGEN, Y_INSTRUCCION, M_INSTRUCCION);

	// Nombre
	pantalla_color_texto(pantalla, C_NOMBRE_1, 1.0f);
	pantalla_estilo_texto(pantalla, E_NOMBRE);
	pantalla_texto(pantalla, X_MARGEN, Y_INSTRUCCION + Y_SEPARACION,
		       contexto->nombre_entrenador);
	if (parpadeo(t, D_CURSOR_TEXTO, false, true))
		pantalla_texto(pantalla,
			       X_MARGEN + (int)escenario->letra_actual,
			       Y_INSTRUCCION + Y_SEPARACION, M_CURSOR);

	// Sprite
	pantalla_sprite(pantalla, P_SPRITE, escenario->sprite, 1.0f);

	// Controles
	if (escenario->letra_actual > 0) {
		pantalla_estilo_texto(pantalla, E_CONTROL);
		pantalla_color_texto(pantalla, C_CONTROL, 1.0f);
		pantalla_texto(pantalla, X_CONTROL_1, Y_CONTROL, M_ENTER);
	}
}

void pr_entrenador_destruir(void *escenario)
{
	free(escenario);
}

struct pr_escena pr_entrenador()
{
	return (struct pr_escena){
		.iniciar = pr_entrenador_iniciar,
		.procesar_eventos = pr_entrenador_eventos,
		.dibujar_graficos = pr_entrenador_graficos,
		.destruir = pr_entrenador_destruir,
	};
}
