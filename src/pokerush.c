#include "pokerush.h"
#include "pokerush_macros.h"

#include "pokerush_splash_screen.h"
#include "pokerush_entrenador.h"
#include "pokerush_menu_principal.h"
#include "pokerush_informacion.h"
#include "pokerush_pokedex.h"
#include "pokerush_menu_juego.h"
#include "pokerush_preparacion.h"
#include "pokerush_versus.h"
#include "pokerush_reglas.h"
#include "pokerush_carrera.h"
#include "pokerush_ganador.h"

#include "motor_estado.h"
#include "motor_directorio.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "split.h"

#define LONGITUD_NOMBRE_ARCHIVO 1023

void aniadir_sprite_a_hash(FILE *archivo, const char *nombre_archivo,
			   void *hash_void)
{
	hash_t *hash = hash_void;

	printf("Añadiendo: %s\n", nombre_archivo);

	char nombre_sin_extension[LONGITUD_NOMBRE_ARCHIVO + 1];
	bool string_entra = false;
	for (int i = 0; i < LONGITUD_NOMBRE_ARCHIVO + 1; i++) {
		if (nombre_archivo[i] == '.') {
			nombre_sin_extension[i] = '\0';
			string_entra = true;
			break;
		} else if (nombre_archivo[i] == '\0') {
			string_entra = true;
			nombre_sin_extension[i] = nombre_archivo[i];
			break;
		}

		nombre_sin_extension[i] = nombre_archivo[i];
	}
	if (!string_entra)
		return;

	estado_t estado;
	sprite_t *sprite = sprite_crear(archivo, &estado);

	printf("  Sprite creado?: %p\n", (void *)sprite);

	if (sprite != NULL)
		hash_insertar(hash, nombre_sin_extension, sprite, NULL);
}

/**
 * Crea un hashmap de sprites a partir de un directorio.
 * 
 * Si falla devuelve NULL, escribiendo en `estado` la razón.
*/
hash_t *crear_sprites(const char *directorio, estado_t *estado)
{
	hash_t *hash = hash_crear(0);
	if (hash == NULL) {
		*estado = ERROR_MEMORIA;
		return NULL;
	}

	int estado_directorio = iterar_directorio(
		directorio, aniadir_sprite_a_hash, hash, "rb");
	if (estado_directorio == -1) {
		*estado = ERROR_RECORRIDO_DIRECTORIO;
		return NULL;
	}

	return hash;
}

/**
 * Genera una lista de todos los Pokémon usando el TP.
 * 
 * Debe llamarse antes de que un jugador elija un Pokémon
 * ya que una vez hecho eso se pierde la habilidad de 
 * obtener la lista completa en orden alfabético.
*/
const struct pokemon_info **obtener_lista_pokemones(TP *tp)
{
	char *nombres_juntos = tp_nombres_disponibles(tp);
	if (nombres_juntos == NULL)
		return NULL;

	char **nombres_ordenados = split(nombres_juntos, ',');
	free(nombres_juntos);
	if (nombres_ordenados == NULL)
		return NULL;

	size_t cantidad = (size_t)tp_cantidad_pokemon(tp);
	const struct pokemon_info **pokemones =
		malloc(cantidad * sizeof(struct pokemon_info));
	if (pokemones == NULL) {
		free(nombres_ordenados);
		return NULL;
	}

	for (size_t i = 0; i < cantidad; i++) {
		pokemones[i] = tp_buscar_pokemon(tp, nombres_ordenados[i]);
		if (pokemones[i] == NULL) {
			free(nombres_ordenados);
			free(pokemones);
			return NULL;
		}
		free(nombres_ordenados[i]);
	}

	free(nombres_ordenados);
	return pokemones;
}

void *pokerush_iniciar(void *configuracion_void, estado_t *estado)
{
	struct pr_config *configuracion = configuracion_void;
	if (configuracion == NULL) {
		*estado = CONFIGURACION_INVALIDA;
		return NULL;
	}

	struct pokerush *estructura = malloc(sizeof(struct pokerush));
	if (estructura == NULL) {
		*estado = ERROR_MEMORIA;
		return NULL;
	}

	TP *tp = tp_crear(configuracion->pokemones_csv);
	if (tp == NULL) {
		*estado = ERROR_CREACION_TP;
		free(estructura);
		return NULL;
	}

	const struct pokemon_info **pokemones = obtener_lista_pokemones(tp);
	if (pokemones == NULL) {
		free(estructura);
		tp_destruir(tp);
		return NULL;
	}

	hash_t *sprites = crear_sprites(configuracion->sprites_dir, estado);
	if (sprites == NULL) {
		free(estructura);
		tp_destruir(tp);
		free(pokemones);
		return NULL;
	}

	struct pr_escena *escenas = estructura->escenas;
	escenas[POKERUSH_SPLASH_SCREEN] = pr_splash_screen();
	escenas[POKERUSH_ENTRENADOR] = pr_entrenador();
	escenas[POKERUSH_MENU_PRINCIPAL] = pr_menu_principal();
	escenas[POKERUSH_MENU_JUEGO] = pr_menu_juego();
	escenas[POKERUSH_PREPARACION] = pr_preparacion();
	escenas[POKERUSH_VERSUS] = pr_versus();
	escenas[POKERUSH_CARRERA] = pr_carrera();
	escenas[POKERUSH_GANADOR] = pr_ganador();
	escenas[POKERUSH_POKEDEX] = pr_pokedex();
	escenas[POKERUSH_INFORMACION] = pr_informacion();
	escenas[POKERUSH_REGLAS] = pr_reglas();

	estructura->contexto = (struct pr_contexto){
		.primera_vez_en_menu = true,
		.frames_escena = 0,
		.tp = tp,
		.sprites = sprites,
		.pokemones = pokemones,
		.cantidad_pokemones = (size_t)tp_cantidad_pokemon(tp),
		// Estos últimos campos no importan inicializar ahora mismo
		.dificultad = FACIL,
		.es_reintento = false,
		.intentos_restantes = 0,
	};
	memset(estructura->contexto.nombre_entrenador, '\0',
	       LONGITUD_NOMBRE + 1);

	estructura->escenario =
		escenas[POKERUSH_SPLASH_SCREEN].iniciar(&estructura->contexto);
	if (estructura->escenario == NULL) {
		*estado = ERROR_CREACION_ESCENA;
		free(estructura);
		tp_destruir(tp);
		free(pokemones);
		return NULL;
	}

	estructura->escena_actual = POKERUSH_SPLASH_SCREEN;

	return estructura;
}

bool pokerush_procesar_eventos(void *estructura_void, int input,
			       estado_t *estado)
{
	struct pokerush *estructura = estructura_void;
	struct pr_contexto *contexto = &estructura->contexto;

	contexto->frames_escena++;

	enum pr_nombre_escena indice_actual = estructura->escena_actual;

	struct pr_escena escena = estructura->escenas[indice_actual];

	enum pr_nombre_escena indice_nuevo = escena.procesar_eventos(
		estructura->escenario, input, &estructura->contexto, estado);

	if (indice_nuevo != indice_actual) {
		// Cambio de escena
		escena.destruir(estructura->escenario);
		estructura->escenario = NULL;

		if (indice_nuevo == POKERUSH_CERRAR)
			return true;

		contexto->frames_escena = 0;
		estructura->escena_actual = indice_nuevo;

		escena = estructura->escenas[indice_nuevo];
		estructura->escenario = escena.iniciar(contexto);
		if (estructura->escenario == NULL) {
			*estado = ERROR_CREACION_ESCENA;
			return true;
		}
	}

	return false;
}

void pokerush_dibujar_graficos(void *estructura_void, pantalla_t *pantalla)
{
	struct pokerush *estructura = estructura_void;

	struct pr_escena escena =
		estructura->escenas[estructura->escena_actual];

	escena.dibujar_graficos(estructura->escenario, pantalla,
				&estructura->contexto);
}

void destructor_sprite(void *sprite)
{
	sprite_destruir(sprite);
}

void pokerush_finalizar(void *estructura_void)
{
	struct pokerush *estructura = estructura_void;

	enum pr_nombre_escena escena = estructura->escena_actual;
	if (escena != POKERUSH_CERRAR || estructura->escenario != NULL) {
		estructura->escenas[escena].destruir(estructura->escenario);
	}

	tp_destruir(estructura->contexto.tp);
	hash_destruir_todo(estructura->contexto.sprites, destructor_sprite);
	free(estructura->contexto.pokemones);
	free(estructura);
}

motor_config_t pokerush_config_motor()
{
	return (motor_config_t){ .alto_pantalla = ALTO_PANTALLA,
				 .ancho_pantalla = ANCHO_PANTALLA,
				 .frames_por_segundo = FRAMES_POR_SEGUNDO };
}

juego_t pr_juego()
{
	return (juego_t){
		.iniciar = pokerush_iniciar,
		.config_motor = pokerush_config_motor,
		.procesar_eventos = pokerush_procesar_eventos,
		.dibujar_graficos = pokerush_dibujar_graficos,
		.finalizar = pokerush_finalizar,
	};
}
