#include "reglas.h"
#include "macros.h"

#include "../motor/estado.h"
#include "../motor/pantalla.h"
#include "../motor/input.h"
#include "../motor/animacion.h"

#include "../hash.h"
#include <ctype.h>

#define M_TITULO "TUTORIAL"

#define Y_LINEA_1 7

#define M_LINEA_1 "Bienvenido/a a Pokerush!"

#define M_LINEA_2 "Al jugar estaras enfrentado a un oponente el cual armara"
#define M_LINEA_3 "una pista de obstaculos para su pokemon, al igual que vos."
#define M_LINEA_4 "No te confundas! Esto no es una carrera, sino un desafio"
#define M_LINEA_5 \
	"amigable entre pokemon: deben llegar a la meta al mismo tiempo."

#define M_LINEA_6 \
	"Cada obstaculo tiene un atributo (fuerza, inteligencia o destreza)"
#define M_LINEA_7 "el cual define cuan rapido tu pokemon lo atravesara."
#define M_LINEA_8 "Las rachas de obstaculos similares haran a tu pokemon"
#define M_LINEA_9 "mas efectivo contra los mismos!"

#define M_LINEA_10 "Revisa el pokedex para conocer mas sobre los pokemon."

void *pr_reglas_iniciar(struct pr_contexto *contexto)
{
	// Debemos retornar algo que no sea NULL.
	return (void *)0xDEADBEEF;
}

enum pr_nombre_escena pr_reglas_eventos(void *escenario_void, int input,
					struct pr_contexto *contexto,
					estado_t *estado)
{
	if (islower(input))
		input = toupper(input);

	if (input == T_SALIR)
		return POKERUSH_MENU_PRINCIPAL;

	return POKERUSH_REGLAS;
}

void pr_reglas_graficos(void *escenario_void, pantalla_t *pantalla,
			struct pr_contexto *contexto)
{
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

	// Información
	pantalla_color_texto(pantalla, C_NORMAL, 1.0f);
	pantalla_estilo_texto(pantalla, E_NORMAL);

	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1, M_LINEA_1);

	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1 + 2, M_LINEA_2);
	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1 + 3, M_LINEA_3);
	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1 + 4, M_LINEA_4);
	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1 + 5, M_LINEA_5);

	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1 + 7, M_LINEA_6);
	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1 + 8, M_LINEA_7);
	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1 + 9, M_LINEA_8);
	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1 + 10, M_LINEA_9);

	pantalla_texto(pantalla, X_MARGEN, Y_LINEA_1 + 12, M_LINEA_10);

	// Volver
	pantalla_color_texto(pantalla, C_CONTROL, 1.0f);
	pantalla_estilo_texto(pantalla, E_CONTROL);
	pantalla_texto(pantalla, X_SALIR, Y_CONTROL, M_VOLVER);
}

void pr_reglas_destruir(void *escenario)
{
	// Nada que destruir ¯\_(ツ)_/¯
}

struct pr_escena pr_reglas()
{
	return (struct pr_escena){
		.iniciar = pr_reglas_iniciar,
		.procesar_eventos = pr_reglas_eventos,
		.dibujar_graficos = pr_reglas_graficos,
		.destruir = pr_reglas_destruir,
	};
}
