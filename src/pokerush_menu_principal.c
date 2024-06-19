#include "pokerush_menu_principal.h"
#include "pokerush_macros.h"

#include "motor_estado.h"
#include "motor_pantalla.h"
#include "motor_input.h"
#include "motor_animacion.h"

#include "hash.h"
#include <ctype.h>
#include <stdlib.h>

#define CANTIDAD_OPCIONES 5

#define T_JUGAR 'J'
#define M_JUGAR "[J] Jugar"

#define T_POKEDEX 'P'
#define M_POKEDEX "[P] Pokedex"

#define T_INFORMACION 'I'
#define M_INFORMACION "[I] Informacion"

#define T_REGLAS 'T'
#define M_REGLAS "[T] Tutorial"

#define X_OPCIONES 38
#define Y_OPCION_1 16
#define Y_SEPARACION 2

#define ARCHIVO_LOGO "logo_front"
#define ARCHIVO_LOGO_BACK "logo_back"
#define P_LOGO 7, 4

#define D_TRANSICION_COLOR_FONDO 1000

typedef struct opcion {
	const char *mensaje;
	enum pr_nombre_escena escena;
} opcion_t;

typedef struct escenario {
	void *logo;
	void *logo_back;
	opcion_t opciones[CANTIDAD_OPCIONES];
	int seleccion;
} escenario_t;

void *pr_menu_principal_iniciar(struct pr_contexto *contexto)
{
	escenario_t *escenario = malloc(sizeof(escenario_t));
	if (escenario == NULL)
		return NULL;

	escenario->seleccion = -1;
	opcion_t *opciones = escenario->opciones;
	opciones[0] = (opcion_t){
		.escena = POKERUSH_MENU_JUEGO,
		.mensaje = M_JUGAR,
	};
	opciones[1] = (opcion_t){
		.escena = POKERUSH_POKEDEX,
		.mensaje = M_POKEDEX,
	};
	opciones[2] = (opcion_t){
		.escena = POKERUSH_REGLAS,
		.mensaje = M_REGLAS,
	};
	opciones[3] = (opcion_t){
		.escena = POKERUSH_INFORMACION,
		.mensaje = M_INFORMACION,
	};
	opciones[4] = (opcion_t){
		.escena = POKERUSH_CERRAR,
		.mensaje = M_SALIR,
	};

	escenario->logo = hash_obtener(contexto->sprites, ARCHIVO_LOGO);
	escenario->logo_back =
		hash_obtener(contexto->sprites, ARCHIVO_LOGO_BACK);

	return escenario;
}

enum pr_nombre_escena pr_menu_principal_eventos(void *escenario_void, int input,
						struct pr_contexto *contexto,
						estado_t *estado)
{
	escenario_t *escenario = escenario_void;

	if (isalpha(input))
		input = toupper(input);

	switch (input) {
	case T_JUGAR:
		contexto->primera_vez_en_menu = false;
		return POKERUSH_MENU_JUEGO;

	case T_SALIR:
		*estado = FINALIZADO_POR_USUARIO;
		return POKERUSH_CERRAR;

	case T_POKEDEX:
		contexto->primera_vez_en_menu = false;
		return POKERUSH_POKEDEX;

	case T_INFORMACION:
		contexto->primera_vez_en_menu = false;
		return POKERUSH_INFORMACION;

	case T_REGLAS:
		contexto->primera_vez_en_menu = false;
		return POKERUSH_REGLAS;

	case FLECHA_ARRIBA:
		if (escenario->seleccion == -1)
			escenario->seleccion = CANTIDAD_OPCIONES - 1;
		else if (escenario->seleccion > 0)
			escenario->seleccion--;
		break;

	case FLECHA_ABAJO:
		if (escenario->seleccion == -1)
			escenario->seleccion = 0;
		else if (escenario->seleccion < CANTIDAD_OPCIONES - 1)
			escenario->seleccion++;
		break;

	case LINEFEED:
		if (escenario->seleccion != -1) {
			if (escenario->opciones[escenario->seleccion].escena ==
			    POKERUSH_CERRAR)
				*estado = FINALIZADO_POR_USUARIO;
			contexto->primera_vez_en_menu = false;
			return escenario->opciones[escenario->seleccion].escena;
		}
		break;
	}

	return POKERUSH_MENU_PRINCIPAL;
}

void pr_menu_principal_graficos(void *escenario_void, pantalla_t *pantalla,
				struct pr_contexto *contexto)
{
	escenario_t *escenario = escenario_void;

	// Transición suave
	float opacidad_fondo =
		(linear(contexto->tiempo_escena_ms, 0, D_TRANSICION_FONDO,
			OPACIDAD_FONDO, 100) -
		 linear(contexto->tiempo_escena_ms, D_TRANSICION_FONDO,
			2 * D_TRANSICION_FONDO, 0, (100 - OPACIDAD_FONDO))) /
		100.0f;

	// Fondo
	float progreso = contexto->primera_vez_en_menu ?
				 linear(contexto->tiempo_escena_ms, 0,
					D_TRANSICION_COLOR_FONDO, 0, 1) :
				 1.0f;
	color_t color_fondo = color_mezcla(color_crear(B_PRINCIPAL),
					   color_crear(B_INICIAL), progreso);
	pantalla_color_fondo(pantalla, color_fondo.r, color_fondo.g,
			     color_fondo.b, opacidad_fondo);
	pantalla_fondo(pantalla);
	pantalla_color_fondo(pantalla, C_TRANSPARENTE);

	// Opciones
	pantalla_color_texto(pantalla, C_NORMAL, 1.0f);
	pantalla_estilo_texto(pantalla, E_NORMAL);
	for (int i = 0; i < CANTIDAD_OPCIONES; i++) {
		if (escenario->seleccion == i) {
			pantalla_color_fondo(pantalla, B_SELECCION, 1.0f);
			pantalla_color_texto(pantalla, C_SELECCION, 1.0f);
		}

		pantalla_texto(pantalla, X_OPCIONES,
			       Y_OPCION_1 + Y_SEPARACION * i,
			       escenario->opciones[i].mensaje);

		if (escenario->seleccion == i) {
			pantalla_color_fondo(pantalla, C_TRANSPARENTE);

			pantalla_color_texto(pantalla, B_SELECCION, 1.0f);
			pantalla_texto(pantalla, X_OPCIONES - 2,
				       Y_OPCION_1 + Y_SEPARACION * i, "%c",
				       S_OPCION);
			pantalla_color_texto(pantalla, C_NORMAL, 1.0f);
		}
	}

	// Logo
	pantalla_sprite(pantalla, P_LOGO, escenario->logo_back, 1.0f);
	pantalla_sprite(pantalla, P_LOGO, escenario->logo, 1.0f);

	// Controles
	pantalla_color_texto(pantalla, C_CONTROL, 1.0f);
	pantalla_estilo_texto(pantalla, E_CONTROL);
	pantalla_texto(pantalla, X_CONTROL_1, Y_CONTROL, M_ARRIBA);
	pantalla_texto(pantalla, X_CONTROL_2, Y_CONTROL, M_ABAJO);
	if (escenario->seleccion != -1)
		pantalla_texto(pantalla, X_CONTROL_3, Y_CONTROL, M_ENTER);
}

void pr_menu_principal_destruir(void *escenario)
{
	free(escenario);
}

struct pr_escena pr_menu_principal()
{
	return (struct pr_escena){
		.iniciar = pr_menu_principal_iniciar,
		.procesar_eventos = pr_menu_principal_eventos,
		.dibujar_graficos = pr_menu_principal_graficos,
		.destruir = pr_menu_principal_destruir,
	};
}
