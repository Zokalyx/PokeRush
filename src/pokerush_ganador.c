#include "pokerush_ganador.h"
#include "pokerush_macros.h"

#include "motor_estado.h"
#include "motor_pantalla.h"
#include "motor_input.h"
#include "motor_animacion.h"

#include "hash.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define M_TITULO "PUNTAJE"
#define M_REINTENTAR "[R] Reintentar"
#define T_REINTENTAR 'R'
#define C_HAY_INTENTOS 50, 255, 50
#define C_NO_HAY_INTENTOS 255, 50, 50
#define Y_INTENTOS 23
#define M_HAY_INTENTOS "Tenes %d reintento(s) disponible(s)"
#define M_NO_HAY_INTENTOS "No tenes mas reintentos disponibles"

#define M_TIEMPO_DE "Tiempo de "
#define X_TIEMPO1 25
#define X_TIEMPO2 55
#define Y_TIEMPO 5

#define M_PUNTAJE "Tu puntaje: %d / 100"
#define Y_PUNTAJE 13
#define X_PUNTAJE 37

#define D_TRANSICION 500

typedef struct escenario {
	unsigned tiempo1, tiempo2;
	unsigned puntaje;
	const char *nombre_cpu;
} escenario_t;

/**
 * Valor absoluto.
*/
int abs(int x)
{
	return x < 0 ? -x : x;
}

void *pr_ganador_iniciar(struct pr_contexto *contexto)
{
	escenario_t *escenario = malloc(sizeof(escenario_t));
	if (escenario == NULL)
		return NULL;

	unsigned tiempo1, tiempo2;
	tiempo1 = tp_calcular_tiempo_pista(contexto->tp, JUGADOR_1);
	tiempo2 = tp_calcular_tiempo_pista(contexto->tp, JUGADOR_2);

	escenario->tiempo1 = tiempo1;
	escenario->tiempo2 = tiempo2;
	if (tiempo1 == 0 && tiempo2 == 0)
		escenario->puntaje = 100;
	else
		escenario->puntaje =
			(unsigned)(100 -
				   100 * abs((int)tiempo1 - (int)tiempo2) /
					   ((int)tiempo1 + (int)tiempo2));

	switch (contexto->dificultad) {
	case FACIL:
		escenario->nombre_cpu = M_FACIL;
		break;
	case MEDIO:
		escenario->nombre_cpu = M_MEDIO;
		break;
	case DIFICIL:
		escenario->nombre_cpu = M_DIFICIL;
		break;
	case IMPOSIBLE:
		escenario->nombre_cpu = M_IMPOSIBLE;
		break;
	}

	return escenario;
}

enum pr_nombre_escena pr_ganador_eventos(void *escenario_void, int input,
					 struct pr_contexto *contexto,
					 estado_t *estado)
{
	// escenario_t *escenario = escenario_void;

	if (islower(input))
		input = toupper(input);

	if (input == T_SALIR) {
		return POKERUSH_MENU_PRINCIPAL;
	} else if (input == T_REINTENTAR && contexto->intentos_restantes > 0) {
		contexto->intentos_restantes--;
		contexto->es_reintento = true;
		return POKERUSH_PREPARACION;
	}

	return POKERUSH_GANADOR;
}

void pr_ganador_graficos(void *escenario_void, pantalla_t *pantalla,
			 struct pr_contexto *contexto)
{
	escenario_t *escenario = escenario_void;

	// Fondo
	// Transición suave
	float opacidad_fondo =
		(linear(contexto->frames_escena, 0, D_TRANSICION_FONDO,
			OPACIDAD_FONDO, 100) -
		 linear(contexto->frames_escena, D_TRANSICION_FONDO,
			2 * D_TRANSICION_FONDO, 0, (100 - OPACIDAD_FONDO))) /
		100.0f;
	pantalla_color_fondo(pantalla, B_PRINCIPAL, opacidad_fondo);
	pantalla_fondo(pantalla);
	pantalla_color_fondo(pantalla, C_TRANSPARENTE);

	// Título
	pantalla_color_texto(pantalla, C_TITULO, 1.0f);
	pantalla_estilo_texto(pantalla, E_TITULO);
	pantalla_texto(pantalla, P_TITULO, M_TITULO);

	// Puntaje
	pantalla_estilo_texto(pantalla, E_NORMAL);
	pantalla_texto(pantalla, X_TIEMPO1, Y_TIEMPO, M_TIEMPO_DE);
	pantalla_texto(pantalla, X_TIEMPO1 + 8, Y_TIEMPO + 2, "%d",
		       escenario->tiempo1);
	pantalla_texto(pantalla, X_TIEMPO2, Y_TIEMPO, M_TIEMPO_DE);
	pantalla_texto(pantalla, X_TIEMPO2 + 8, Y_TIEMPO + 2, "%d",
		       escenario->tiempo2);

	pantalla_estilo_texto(pantalla, E_TITULO);
	pantalla_texto(pantalla, X_PUNTAJE, Y_PUNTAJE, M_PUNTAJE,
		       escenario->puntaje);

	pantalla_estilo_texto(pantalla, E_NOMBRE);
	pantalla_color_texto(pantalla, C_NOMBRE_1, 1.0f);
	pantalla_texto(pantalla, X_TIEMPO1 + strlen(M_TIEMPO_DE), Y_TIEMPO,
		       contexto->nombre_entrenador);
	pantalla_color_texto(pantalla, C_NOMBRE_2, 1.0f);
	pantalla_texto(pantalla, X_TIEMPO2 + strlen(M_TIEMPO_DE), Y_TIEMPO,
		       escenario->nombre_cpu);

	// Mensaje reintento
	pantalla_estilo_texto(pantalla, E_NORMAL);
	if (contexto->intentos_restantes > 0) {
		pantalla_color_texto(pantalla, C_HAY_INTENTOS, 1.0f);
		pantalla_texto(pantalla, X_MARGEN, Y_INTENTOS, M_HAY_INTENTOS,
			       contexto->intentos_restantes);
	} else {
		pantalla_color_texto(pantalla, C_NO_HAY_INTENTOS, 1.0f);
		pantalla_texto(pantalla, X_MARGEN, Y_INTENTOS,
			       M_NO_HAY_INTENTOS);
	}

	// Volver y reintento
	pantalla_color_texto(pantalla, C_CONTROL, 1.0f);
	pantalla_estilo_texto(pantalla, E_CONTROL);
	if (contexto->intentos_restantes > 0)
		pantalla_texto(pantalla, X_CONTROL_1, Y_CONTROL, M_REINTENTAR,
			       contexto->intentos_restantes);
	pantalla_texto(pantalla, X_SALIR, Y_CONTROL, M_VOLVER);

	// Transición
	pantalla_color_fondo(pantalla, C_NORMAL, 1.0f);
	int x = ease_in_out(contexto->frames_escena, 0, D_TRANSICION, 0,
			    ANCHO_PANTALLA / 2);
	pantalla_rectangulo(pantalla, 0, 0, (unsigned)(ANCHO_PANTALLA / 2 - x),
			    ALTO_PANTALLA, ' ');
	pantalla_rectangulo(pantalla, ANCHO_PANTALLA / 2 + 1 + x, 0,
			    ANCHO_PANTALLA, ALTO_PANTALLA, ' ');
}

void pr_ganador_destruir(void *escenario_void)
{
	escenario_t *escenario = escenario_void;

	free(escenario);
}

struct pr_escena pr_ganador()
{
	return (struct pr_escena){
		.iniciar = pr_ganador_iniciar,
		.procesar_eventos = pr_ganador_eventos,
		.dibujar_graficos = pr_ganador_graficos,
		.destruir = pr_ganador_destruir,
	};
}
