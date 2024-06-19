#include "pokerush_informacion.h"
#include "pokerush_macros.h"

#include "motor_estado.h"
#include "motor_pantalla.h"
#include "motor_input.h"
#include "motor_animacion.h"

#include "hash.h"
#include <ctype.h>

#define M_TITULO "INFORMACION"

#define Y_LINEA_1 7

#define M_LINEA_1 \
	"Este juego fue realizado enteramente por Francisco Russo como"
#define M_LINEA_2 "trabajo practico integrador de Algoritmos II en la FIUBA."

#define M_LINEA_3 \
	"Se reutilizaron trabajos practicos anteriores, asi como funciones"
#define M_LINEA_4 "de librerias estandar de C y algunas especificas al sistema"
#define M_LINEA_5 "operativo (se mantuvo la portabilidad a Windows y Linux)."

#define M_LINEA_6 \
	"Muchas gracias por jugar! (Nintendo no me hagas juicio porfis)"

#define C_LINK 150, 200, 255
#define M_LINK "https://github.com/Zokalyx"

void *pr_informacion_iniciar(struct pr_contexto *contexto)
{
	// Debemos retornar algo que no sea NULL.
	return (void *)0xDEADBEEF;
}

enum pr_nombre_escena pr_informacion_eventos(void *escenario_void, int input,
					     struct pr_contexto *contexto,
					     estado_t *estado)
{
	if (islower(input))
		input = toupper(input);

	if (input == T_SALIR)
		return POKERUSH_MENU_PRINCIPAL;

	return POKERUSH_INFORMACION;
}

void pr_informacion_graficos(void *escenario_void, pantalla_t *pantalla,
			     struct pr_contexto *contexto)
{
	// Fondo
	// Transición suave
	float opacidad_fondo =
		(linear(contexto->tiempo_escena_ms, 0, D_TRANSICION_FONDO,
			OPACIDAD_FONDO, 100) -
		 linear(contexto->tiempo_escena_ms, D_TRANSICION_FONDO,
			2 * D_TRANSICION_FONDO, 0, (100 - OPACIDAD_FONDO))) /
		100.0f;
	pantalla_color_fondo(pantalla, B_PRINCIPAL, opacidad_fondo);
	pantalla_fondo(pantalla);
	pantalla_color_fondo(pantalla, C_TRANSPARENTE);

	// Título
	pantalla_color_texto(pantalla, C_TITULO, 1.0f);
	pantalla_estilo_texto(pantalla, E_TITULO);
	pantalla_texto(pantalla, P_TITULO, M_TITULO);

	// Información
	pantalla_color_texto(pantalla, C_NORMAL, 1.0f);
	pantalla_estilo_texto(pantalla, E_NORMAL);
	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1, M_LINEA_1);
	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1 + 1, M_LINEA_2);

	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1 + 3, M_LINEA_3);
	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1 + 4, M_LINEA_4);
	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1 + 5, M_LINEA_5);

	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1 + 7, M_LINEA_6);

	pantalla_color_texto(pantalla, C_LINK, 1.0f);
	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1 + 12, M_LINK);

	// Volver
	pantalla_color_texto(pantalla, C_CONTROL, 1.0f);
	pantalla_estilo_texto(pantalla, E_CONTROL);
	pantalla_texto(pantalla, X_SALIR, Y_CONTROL, M_VOLVER);
}

void pr_informacion_destruir(void *escenario)
{
	// Nada que destruir ¯\_(ツ)_/¯
}

struct pr_escena pr_informacion()
{
	return (struct pr_escena){
		.iniciar = pr_informacion_iniciar,
		.procesar_eventos = pr_informacion_eventos,
		.dibujar_graficos = pr_informacion_graficos,
		.destruir = pr_informacion_destruir,
	};
}
