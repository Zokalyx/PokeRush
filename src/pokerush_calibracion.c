#include "pokerush_calibracion.h"
#include "pokerush_macros.h"

#include "motor_estado.h"
#include "motor_pantalla.h"
#include "motor_input.h"
#include "motor_animacion.h"

#include "hash.h"
#include <ctype.h>

#define M_TITULO "CALIBRACION"

#define M_INSTRUCCION_1 \
	"Asegurate de que la pantalla quepa dentro de tu terminal."
#define M_INSTRUCCION_2 "Todo el borde de la pantalla está marcado con '*'"

#define M_INSTRUCCION_3 "Ajusta la velocidad para que el cuadrado parpadee"
#define M_INSTRUCCION_4 "aproximadamente 1 vez por segundo."

void *pr_calibracion_iniciar(struct pr_contexto *contexto)
{
	// Debemos retornar algo que no sea NULL.
	return (void *)0xDEADBEEF;
}

enum pr_nombre_escena pr_calibracion_eventos(void *escenario_void, int input,
					     struct pr_contexto *contexto,
					     estado_t *estado)
{
	if (input != 0)
		return POKERUSH_SPLASH_SCREEN;

	return POKERUSH_CALIBRACION;
}

void pr_calibracion_graficos(void *escenario_void, pantalla_t *pantalla,
			     struct pr_contexto *contexto)
{
	// Fondo
	pantalla_color_fondo(pantalla, 0, 0, 0, 1.0f);
	pantalla_fondo(pantalla);
	pantalla_color_fondo(pantalla, C_TRANSPARENTE);

	// Borde
	pantalla_color_texto(pantalla, C_TITULO, 1.0f);
	pantalla_rectangulo(pantalla, 0, 0, ANCHO_PANTALLA, ALTO_PANTALLA, '*');

	// Título
	pantalla_color_texto(pantalla, C_TITULO, 1.0f);
	pantalla_estilo_texto(pantalla, E_TITULO);
	pantalla_texto(pantalla, P_TITULO, M_TITULO);
	pantalla_estilo_texto(pantalla, E_NORMAL);

	// Controles
	pantalla_color_texto(pantalla, C_CONTROL, 1.0f);
	pantalla_texto(pantalla, X_CONTROL_1, Y_CONTROL, M_IZQUIERDA);
	pantalla_texto(pantalla, X_CONTROL_2, Y_CONTROL, M_DERECHA);
	pantalla_texto(pantalla, X_CONTROL_3, Y_CONTROL, M_ENTER);
}

void pr_calibracion_destruir(void *escenario)
{
	// Nada que destruir ¯\_(ツ)_/¯
}

struct pr_escena pr_calibracion()
{
	return (struct pr_escena){
		.iniciar = pr_calibracion_iniciar,
		.procesar_eventos = pr_calibracion_eventos,
		.dibujar_graficos = pr_calibracion_graficos,
		.destruir = pr_calibracion_destruir,
	};
}
