#include "pokerush_menu_juego.h"
#include "pokerush_macros.h"

#include "motor_estado.h"
#include "motor_pantalla.h"
#include "motor_input.h"
#include "motor_animacion.h"

#include "hash.h"
#include <ctype.h>
#include <stdlib.h>

#define CANTIDAD_OPONENTES 4

#define M_OPCION_FACIL "[F] Brock (facil)"
#define T_FACIL 'F'
#define REINTENTOS_FACIL 6

#define M_OPCION_MEDIO "[M] Misty (medio)"
#define T_MEDIO 'M'
#define REINTENTOS_MEDIO 4

#define M_OPCION_DIFICIL "[D] Red (dificil)"
#define T_DIFICIL 'D'
#define REINTENTOS_DIFICIL 2

#define M_OPCION_IMPOSIBLE "[I] Cynthia (imposible)"
#define T_IMPOSIBLE 'I'
#define REINTENTOS_IMPOSIBLE 0

#define Y_OPCION_1 10
#define Y_SEPARACION 2
#define M_TITULO "OPONENTE"
#define P_SPRITE 50, 4

typedef struct opcion {
	const char *mensaje;
	enum pr_dificultad dificultad;
	void *sprite;
	unsigned reintentos;
} opcion_t;

typedef struct escenario {
	int seleccion;
	opcion_t opciones[CANTIDAD_OPONENTES];
} escenario_t;

void *pr_menu_juego_iniciar(struct pr_contexto *contexto)
{
	escenario_t *escenario = malloc(sizeof(escenario_t));
	if (escenario == NULL)
		return NULL;

	escenario->seleccion = 0;
	escenario->opciones[0] = (opcion_t){
		.dificultad = FACIL,
		.mensaje = M_OPCION_FACIL,
		.sprite = hash_obtener(contexto->sprites, M_FACIL),
		.reintentos = REINTENTOS_FACIL,
	};
	escenario->opciones[1] = (opcion_t){
		.dificultad = MEDIO,
		.mensaje = M_OPCION_MEDIO,
		.sprite = hash_obtener(contexto->sprites, M_MEDIO),
		.reintentos = REINTENTOS_MEDIO,
	};
	escenario->opciones[2] = (opcion_t){
		.dificultad = DIFICIL,
		.mensaje = M_OPCION_DIFICIL,
		.sprite = hash_obtener(contexto->sprites, M_DIFICIL),
		.reintentos = REINTENTOS_DIFICIL,
	};
	escenario->opciones[3] = (opcion_t){
		.dificultad = IMPOSIBLE,
		.mensaje = M_OPCION_IMPOSIBLE,
		.sprite = hash_obtener(contexto->sprites, M_IMPOSIBLE),
		.reintentos = REINTENTOS_IMPOSIBLE,
	};

	return escenario;
}

enum pr_nombre_escena pr_menu_juego_eventos(void *escenario_void, int input,
					    struct pr_contexto *contexto,
					    estado_t *estado)
{
	escenario_t *escenario = escenario_void;

	if (isalpha(input))
		input = toupper(input);

	switch (input) {
	case T_FACIL:
		contexto->dificultad = FACIL;
		contexto->es_reintento = false;
		contexto->intentos_restantes = REINTENTOS_FACIL;
		return POKERUSH_PREPARACION;

	case T_MEDIO:
		contexto->dificultad = MEDIO;
		contexto->es_reintento = false;
		contexto->intentos_restantes = REINTENTOS_MEDIO;
		return POKERUSH_PREPARACION;

	case T_DIFICIL:
		contexto->dificultad = DIFICIL;
		contexto->es_reintento = false;
		contexto->intentos_restantes = REINTENTOS_DIFICIL;
		return POKERUSH_PREPARACION;

	case T_IMPOSIBLE:
		contexto->dificultad = IMPOSIBLE;
		contexto->es_reintento = false;
		contexto->intentos_restantes = REINTENTOS_IMPOSIBLE;
		return POKERUSH_PREPARACION;

	case T_SALIR:
		return POKERUSH_MENU_PRINCIPAL;

	case FLECHA_ARRIBA:
		if (escenario->seleccion == -1)
			escenario->seleccion = CANTIDAD_OPONENTES - 1;
		else if (escenario->seleccion > 0)
			escenario->seleccion--;
		break;

	case FLECHA_ABAJO:
		if (escenario->seleccion == -1)
			escenario->seleccion = 0;
		else if (escenario->seleccion < CANTIDAD_OPONENTES - 1)
			escenario->seleccion++;
		break;

	case LINEFEED:
		if (escenario->seleccion != -1) {
			contexto->dificultad =
				escenario->opciones[escenario->seleccion]
					.dificultad;
			contexto->es_reintento = false;
			contexto->intentos_restantes =
				escenario->opciones[escenario->seleccion]
					.reintentos;
			return POKERUSH_PREPARACION;
		}
		break;
	}

	return POKERUSH_MENU_JUEGO;
}

void pr_menu_juego_graficos(void *escenario_void, pantalla_t *pantalla,
			    struct pr_contexto *contexto)
{
	escenario_t *escenario = escenario_void;

	uint64_t t = contexto->frames_escena * contexto->multiplicador_frames;

	// Fondo
	float opacidad_fondo =
		pulso(t, 0, D_TRANSICION_FONDO, OPACIDAD_FONDO, 100) / 100.0f;
	pantalla_color_fondo(pantalla, B_PRINCIPAL, opacidad_fondo);
	pantalla_fondo(pantalla);
	pantalla_color_fondo(pantalla, C_TRANSPARENTE);

	// Título
	pantalla_color_texto(pantalla, C_TITULO, 1.0f);
	pantalla_estilo_texto(pantalla, E_TITULO);
	pantalla_texto(pantalla, P_TITULO, M_TITULO);

	// Opciones
	pantalla_color_texto(pantalla, C_NORMAL, 1.0f);
	pantalla_estilo_texto(pantalla, E_NORMAL);
	for (int i = 0; i < CANTIDAD_OPONENTES; i++) {
		if (escenario->seleccion == i) {
			pantalla_color_fondo(pantalla, B_SELECCION, 1.0f);
			pantalla_color_texto(pantalla, C_SELECCION, 1.0f);
		}

		pantalla_texto(pantalla, X_MARGEN,
			       Y_OPCION_1 + Y_SEPARACION * i,
			       escenario->opciones[i].mensaje);

		if (escenario->seleccion == i) {
			pantalla_color_fondo(pantalla, C_TRANSPARENTE);

			pantalla_color_texto(pantalla, B_SELECCION, 1.0f);
			pantalla_texto(pantalla, X_MARGEN - 2,
				       Y_OPCION_1 + Y_SEPARACION * i, "%c",
				       S_OPCION);
			pantalla_color_texto(pantalla, C_NORMAL, 1.0f);
		}
	}

	// Sprite
	void *sprite = escenario->opciones[escenario->seleccion].sprite;
	pantalla_sprite(pantalla, P_SPRITE, sprite, 1.0f);

	// Controles
	pantalla_color_texto(pantalla, C_CONTROL, 1.0f);
	pantalla_estilo_texto(pantalla, E_CONTROL);
	pantalla_texto(pantalla, X_CONTROL_1, Y_CONTROL, M_ARRIBA);
	pantalla_texto(pantalla, X_CONTROL_2, Y_CONTROL, M_ABAJO);
	if (escenario->seleccion != -1)
		pantalla_texto(pantalla, X_CONTROL_3, Y_CONTROL, M_ENTER);

	pantalla_texto(pantalla, X_SALIR, Y_CONTROL, M_VOLVER);
}

void pr_menu_juego_destruir(void *escenario_void)
{
	escenario_t *escenario = escenario_void;

	free(escenario);
}

struct pr_escena pr_menu_juego()
{
	return (struct pr_escena){
		.iniciar = pr_menu_juego_iniciar,
		.procesar_eventos = pr_menu_juego_eventos,
		.dibujar_graficos = pr_menu_juego_graficos,
		.destruir = pr_menu_juego_destruir,
	};
}
