#include "pokerush_splash_screen.h"
#include "pokerush_macros.h"

#include "motor_estado.h"
#include "motor_pantalla.h"
#include "motor_color.h"
#include "motor_animacion.h"

#include <stdint.h>
#include <stdlib.h>

#define P_LOGO 7, 5
#define D_ANIMACION 1500
#define R_LOGO 1000
#define D_PARPADEO 700
#define B_INICIAL 0, 0, 0
#define B_FINAL 255, 255, 255
#define M_CONTROL "Presiona cualquier tecla para continuar"
#define X_CONTROL 25

#define P_POKEBALL 35, 14

typedef struct escenario {
	void *logo_back;
	void *logo_front;
	void *pokeball;
} escenario_t;

void *pr_splash_screen_iniciar(struct pr_contexto *contexto)
{
	escenario_t *escenario = malloc(sizeof(escenario_t));
	if (escenario == NULL)
		return NULL;

	escenario->logo_back = hash_obtener(contexto->sprites, "logo_back");
	escenario->logo_front = hash_obtener(contexto->sprites, "logo_front");
	escenario->pokeball = hash_obtener(contexto->sprites, "pokeball");

	return escenario;
}

enum pr_nombre_escena pr_splash_screen_eventos(void *escenario, int input,
					       struct pr_contexto *contexto,
					       estado_t *estado)
{
	if (input != 0) {
		return POKERUSH_ENTRENADOR;
	}

	return POKERUSH_SPLASH_SCREEN;
}

void pr_splash_screen_graficos(void *escenario_void, pantalla_t *pantalla,
			       struct pr_contexto *contexto)
{
	escenario_t *escenario = escenario_void;

	uint64_t t = contexto->frames_escena;

	// Fondo
	float progreso = linear(t, 0, D_ANIMACION, 0, 1);
	color_t color = color_mezcla(color_crear(B_FINAL),
				     color_crear(B_INICIAL), progreso);
	pantalla_color_fondo(pantalla, color.r, color.g, color.b, 1.0f);
	pantalla_fondo(pantalla);

	// Logo
	progreso = linear(t, R_LOGO, R_LOGO + D_ANIMACION, 0, 1);
	pantalla_sprite(pantalla, P_LOGO, escenario->logo_back, 1.0f);
	pantalla_sprite(pantalla, P_LOGO, escenario->logo_front, progreso);

	pantalla_sprite(pantalla, P_POKEBALL, escenario->pokeball, progreso);

	// Mensaje de controles
	if (t > D_ANIMACION + R_LOGO && !parpadeo(t, D_PARPADEO, false, true)) {
		pantalla_color_fondo(pantalla, C_TRANSPARENTE);
		pantalla_color_texto(pantalla, C_CONTROL, 1.0f);
		pantalla_estilo_texto(pantalla, E_CONTROL);
		pantalla_texto(pantalla, X_CONTROL, Y_CONTROL, M_CONTROL);
	}
}

void pr_splash_screen_destruir(void *escenario)
{
	free(escenario);
}

struct pr_escena pr_splash_screen()
{
	return (struct pr_escena){
		.iniciar = pr_splash_screen_iniciar,
		.procesar_eventos = pr_splash_screen_eventos,
		.dibujar_graficos = pr_splash_screen_graficos,
		.destruir = pr_splash_screen_destruir,
	};
}
