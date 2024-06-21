#ifndef __POKERUSH_H__
#define __POKERUSH_H__

#include "motor.h"
#include "motor_estado.h"
#include "motor_pantalla.h"

#include <stdbool.h>
#include <stdint.h>
#include "tp.h"
#include "hash.h"

#define LONGITUD_NOMBRE 31

/**
 * Datos a ser pasados al constructor
 * de pokerush.
*/
struct pr_config {
	const char *pokemones_csv;
	const char *sprites_dir;
};

/**
 * Nombre de escena. A su vez actúa como índice
 * para elegir una escena de un vector.
*/
enum pr_nombre_escena {
	// No es una escena real, simplemente un indicador
	// de que el juego debe cerrarse ASAP.
	POKERUSH_CERRAR = -1,
	// Escenas
	POKERUSH_CALIBRACION,
	POKERUSH_SPLASH_SCREEN,
	POKERUSH_ENTRENADOR,
	POKERUSH_MENU_PRINCIPAL,
	POKERUSH_MENU_JUEGO,
	POKERUSH_PREPARACION,
	POKERUSH_VERSUS,
	POKERUSH_CARRERA,
	POKERUSH_GANADOR,
	POKERUSH_POKEDEX,
	POKERUSH_INFORMACION,
	POKERUSH_REGLAS,
	// Tampoco es una escena, solo una manera de
	// tener la cantidad de elementos en un enum
	POKERUSH_CANTIDAD_ESCENAS,
};

/**
 * Dificultad elegida antes de la carrera, puede cambiar
 * a lo largo de la sesión de juego.
*/
enum pr_dificultad {
	FACIL,
	MEDIO,
	DIFICIL,
	IMPOSIBLE,
};

/**
 * Datos globales del juego que deben ser accesibles
 * en las escenas, pero son persistentes a lo largo
 * de toda la sesión.
*/
struct pr_contexto {
	TP *tp;
	hash_t *sprites;
	char nombre_entrenador[LONGITUD_NOMBRE + 1];
	uint64_t frames_escena;
	unsigned multiplicador_frames;
	enum pr_dificultad dificultad;
	bool primera_vez_en_menu;
	unsigned intentos_restantes;
	bool es_reintento;
	size_t cantidad_pokemones;
	const struct pokemon_info **pokemones;
};

/**
 * Escena o menú particular del juego.
*/
struct pr_escena {
	// Constructor, retorna escenario
	void *(*iniciar)(struct pr_contexto *contexto);
	// Procesador de inputs y eventos, devuelve la siguiente escena.
	// Si la misma es la de CERRAR, se escribe en estado la razón.
	enum pr_nombre_escena (*procesar_eventos)(void *escenario, int input,
						  struct pr_contexto *contexto,
						  estado_t *estado);
	// Muestra en pantalla lo correspondiente a la escena
	void (*dibujar_graficos)(void *escenario, pantalla_t *pantalla,
				 struct pr_contexto *contexto);
	// Destructor
	void (*destruir)(void *escenario);
};

/**
 * Estructura principal de pokerush, incluye
 * todo lo requerido para el funcionamiento interno,
 * y todo tipo de datos globales y contextos.
 * 
 * No todos los campos deben ser accesibles por las
 * escenas (ahora mismo, solo `escenas` no es accesible
 * por las escenas individuales).
*/
struct pokerush {
	// Contexto particular a la escena actual
	void *escenario;
	// Contexto global del juego
	struct pr_contexto contexto;
	// Escena actual
	enum pr_nombre_escena escena_actual;
	// Vector de todas las escenas
	struct pr_escena escenas[POKERUSH_CANTIDAD_ESCENAS];
};

/**
 * Crea la estructura de juego asociada a Pokerush.
*/
juego_t pr_juego();

/**
 * Crea la configuración de motor que requiere Pokerush.
*/
motor_config_t pr_config_motor();

#endif // __POKERUSH_H__