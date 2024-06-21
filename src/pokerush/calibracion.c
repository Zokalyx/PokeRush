#include "calibracion.h"
#include "macros.h"

#include "../motor/estado.h"
#include "../motor/pantalla.h"
#include "../motor/input.h"
#include "../motor/animacion.h"

#include "../hash.h"
#include <ctype.h>
#include <stdlib.h>

#define M_TITULO "CALIBRACION"

#define M_INSTRUCCION_1 \
	"Asegurate de que la pantalla quepa dentro de tu terminal."
#define M_INSTRUCCION_2 "Todo el borde de la pantalla esta marcado con '*'"

#define M_INSTRUCCION_3 "Ajusta la velocidad con las flechitas para que la"
#define M_INSTRUCCION_4 "pokebola parpadee 1 o 2 veces por segundo."

#define Y_INSTRUCCION 5

#define Y_BARRA 24
#define X_BARRA 5
#define VALOR_MAX (ANCHO_PANTALLA - 10)

#define P_POKEBALL 36, 12

typedef struct escenario {
	unsigned valor;
	void *pokeball;
} escenario_t;

void *pr_calibracion_iniciar(struct pr_contexto *contexto)
{
	escenario_t *escenario = malloc(sizeof(escenario_t));
	if (escenario == NULL)
		return NULL;

	escenario->valor = 1;
	escenario->pokeball = hash_obtener(contexto->sprites, "pokeball");

	return escenario;
}

enum pr_nombre_escena pr_calibracion_eventos(void *escenario_void, int input,
					     struct pr_contexto *contexto,
					     estado_t *estado)
{
	escenario_t *escenario = escenario_void;

	switch (input) {
	case FLECHA_DERECHA:
		if (escenario->valor < VALOR_MAX - 2)
			escenario->valor++;
		break;

	case FLECHA_IZQUIERDA:
		if (escenario->valor > 1)
			escenario->valor--;
		break;

	case LINEFEED:
		contexto->multiplicador_frames = escenario->valor;
		return POKERUSH_SPLASH_SCREEN;
	}

	return POKERUSH_CALIBRACION;
}

void pr_calibracion_graficos(void *escenario_void, pantalla_t *pantalla,
			     struct pr_contexto *contexto)
{
	escenario_t *escenario = escenario_void;

	uint64_t t = contexto->frames_escena * escenario->valor;

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

	// Instrucciones
	pantalla_texto(pantalla, X_MARGEN, Y_INSTRUCCION, M_INSTRUCCION_1);
	pantalla_texto(pantalla, X_MARGEN, Y_INSTRUCCION + 1, M_INSTRUCCION_2);

	pantalla_texto(pantalla, X_MARGEN, Y_INSTRUCCION + 3, M_INSTRUCCION_3);
	pantalla_texto(pantalla, X_MARGEN, Y_INSTRUCCION + 4, M_INSTRUCCION_4);

	// Cuadrado
	float opacidad = (float)parpadeo(t, 1000, 0, 1) / 3.0f;
	pantalla_sprite(pantalla, P_POKEBALL, escenario->pokeball, opacidad);

	// Barrita
	pantalla_rectangulo(pantalla, X_BARRA, Y_BARRA, VALOR_MAX, 1, '-');
	pantalla_color_fondo(pantalla, 255, 255, 255, 1.0f);
	pantalla_texto(pantalla, X_BARRA + (int)escenario->valor, Y_BARRA, " ");

	// Controles
	pantalla_color_fondo(pantalla, C_TRANSPARENTE);
	pantalla_color_texto(pantalla, C_CONTROL, 1.0f);

	if (escenario->valor > 1)
		pantalla_texto(pantalla, X_CONTROL_1, Y_CONTROL, M_IZQUIERDA);
	if (escenario->valor < VALOR_MAX - 2)
		pantalla_texto(pantalla, X_CONTROL_2, Y_CONTROL, M_DERECHA);
	pantalla_texto(pantalla, X_CONTROL_3, Y_CONTROL, M_ENTER);
}

void pr_calibracion_destruir(void *escenario)
{
	free(escenario);
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
