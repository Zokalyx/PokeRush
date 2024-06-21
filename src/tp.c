#include "tp.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "hash.h"
#include "lista.h"

#define TIEMPO_VALOR_BASE 10

#define MAX_NOMBRE_POKEMON 127
#define FORMATO_LINEA_CSV "%127[^, \t\n],%d,%d,%d "

#define MAX_STRING_NUMERO 15

struct jugador {
	struct pokemon_info *pokemon;
	lista_t *obstaculos;
};

struct tp {
	hash_t *pokemones;
	struct jugador jugador_1;
	struct jugador jugador_2;
};

/**
 * Como strcpy, pero deja el nombre de pokémon en un formato uniforme:
 * primera letra mayúscula y las demás minúsculas. "piKaCHU" -> "Pikachu".
*/
void strcpy_nombre_pokemon(char *destino, const char *origen)
{
	bool primer_caracter = true;

	while (*origen != '\0') {
		*destino = primer_caracter ? (char)toupper(*origen) :
					     (char)tolower(*origen);

		primer_caracter = false;
		destino++;
		origen++;
	}

	*destino = '\0';
}

/**
 * Libera la memoria reservada por un solo pokémon.
*/
void liberar_pokemon(void *pokemon_void)
{
	struct pokemon_info *pokemon = pokemon_void;

	free(pokemon->nombre);
	free(pokemon);
}

/**
 * Crea la colección de pokémones a partir de un archivo .csv.
 * El nombre del archivo debe ser válido.
 * 
 * En caso de error devuelve NULL.
*/
hash_t *leer_pokemones(const char *nombre_archivo)
{
	FILE *archivo = fopen(nombre_archivo, "r");
	if (archivo == NULL)
		return NULL;

	hash_t *pokemones = hash_crear(0);
	if (pokemones == NULL) {
		fclose(archivo);
		return NULL;
	}

	char buffer_nombre[MAX_NOMBRE_POKEMON + 1];
	int fuerza, destreza, inteligencia;

	bool error_lectura = false;
	while (true) {
		int leido = fscanf(archivo, FORMATO_LINEA_CSV, buffer_nombre,
				   &fuerza, &destreza, &inteligencia);
		if (leido == EOF) {
			// Éxito!
			break;
		} else if (leido != 4) {
			error_lectura = true;
			break;
		}

		struct pokemon_info *pokemon =
			malloc(sizeof(struct pokemon_info));
		if (pokemon == NULL) {
			error_lectura = true;
			break;
		}

		size_t longitud_nombre = strlen(buffer_nombre);
		char *nombre = malloc((longitud_nombre + 1) * sizeof(char));
		if (nombre == NULL) {
			free(pokemon);
			error_lectura = true;
			break;
		}

		strcpy_nombre_pokemon(nombre, buffer_nombre);

		pokemon->nombre = nombre;
		pokemon->fuerza = fuerza;
		pokemon->destreza = destreza;
		pokemon->inteligencia = inteligencia;

		struct pokemon_info *anterior;
		void *exito = hash_insertar(pokemones, nombre, pokemon,
					    (void **)&anterior);
		if (exito == NULL) {
			free(pokemon);
			free(nombre);
			error_lectura = true;
			break;
		}

		if (anterior != NULL)
			liberar_pokemon(anterior);
	}

	if (error_lectura) {
		hash_destruir_todo(pokemones, liberar_pokemon);
		fclose(archivo);
		return NULL;
	}

	fclose(archivo);

	return pokemones;
}

TP *tp_crear(const char *nombre_archivo)
{
	if (nombre_archivo == NULL)
		return NULL;

	hash_t *pokemones = leer_pokemones(nombre_archivo);
	if (pokemones == NULL)
		return NULL;

	TP *tp = malloc(sizeof(TP));
	if (tp == NULL) {
		hash_destruir_todo(pokemones, liberar_pokemon);
		return NULL;
	}

	lista_t *obstaculos1 = lista_crear();
	if (obstaculos1 == NULL) {
		hash_destruir_todo(pokemones, liberar_pokemon);
		free(tp);
		return NULL;
	}

	lista_t *obstaculos2 = lista_crear();
	if (obstaculos2 == NULL) {
		hash_destruir_todo(pokemones, liberar_pokemon);
		lista_destruir(obstaculos1);
		free(tp);
		return NULL;
	}

	tp->pokemones = pokemones;

	tp->jugador_1.obstaculos = obstaculos1;
	tp->jugador_1.pokemon = NULL;

	tp->jugador_2.obstaculos = obstaculos2;
	tp->jugador_2.pokemon = NULL;

	return tp;
}

int tp_cantidad_pokemon(TP *tp)
{
	if (tp == NULL)
		return 0;

	return (int)hash_cantidad(tp->pokemones);
}

const struct pokemon_info *tp_buscar_pokemon(TP *tp, const char *nombre)
{
	if (tp == NULL || nombre == NULL)
		return NULL;

	char buffer_nombre[MAX_NOMBRE_POKEMON + 1];
	strcpy_nombre_pokemon(buffer_nombre, nombre);

	return hash_obtener(tp->pokemones, buffer_nombre);
}

/**
 * Struct auxiliar para `agregar_a_nombres`.
*/
typedef struct contexto_agregar_nombre {
	const char **nombres;
	size_t indice;
	// Pokémon ya ocupados por los jugadores.
	struct pokemon_info *pokemon1, *pokemon2;
} contexto_agregar_nombre_t;

/**
 * Función auxiliar para `tp_nombres_disponibles`
 * 
 * Agrega todas las claves (nombres de pokémon) excepto las que ya están
 * tomadas por un jugador a un vector.
*/
bool agregar_nombre(const char *clave, void *valor, void *contexto_void)
{
	contexto_agregar_nombre_t *contexto = contexto_void;

	// Verificar que el Pokémon esté disponible.
	bool ocupado_por_jugador1 = contexto->pokemon1 != NULL &&
				    strcmp(clave, contexto->pokemon1->nombre) ==
					    0;

	bool ocupado_por_jugador2 = contexto->pokemon2 != NULL &&
				    strcmp(clave, contexto->pokemon2->nombre) ==
					    0;

	if (ocupado_por_jugador1 || ocupado_por_jugador2)
		return true;

	contexto->nombres[contexto->indice++] = clave;

	return true;
}

/**
 * Swap de tamaño variable a ser utilizado en `quicksort`
*/
void swap(void *a, void *b, size_t tamanio)
{
	for (size_t j = 0; j < tamanio; j++) {
		char aux = *((char *)a + j);
		*((char *)a + j) = *((char *)b + j);
		*((char *)b + j) = aux;
	}
}

/**
 * Quicksort para reemplazar al prohibido y mítico `qsort`...
*/
void quicksort(void *base, size_t cantidad_elementos, size_t tamanio_elemento,
	       int (*comparador)(const void *, const void *))
{
	// Caso base
	if (cantidad_elementos <= 1)
		return;

	// Pivoteo y separación de bloques
	// Se elije el último elemento como pivote.
	void *pos_pivote_inicial =
		(void *)((size_t)base +
			 tamanio_elemento * (cantidad_elementos - 1));

	void *pos_pivote_final = base;
	for (size_t i = 0; i < cantidad_elementos; i++) {
		void *elemento = (void *)((size_t)base + i * tamanio_elemento);

		if (elemento == pos_pivote_inicial)
			continue;

		if (comparador(elemento, pos_pivote_inicial) < 0) {
			swap(elemento, pos_pivote_final, tamanio_elemento);

			pos_pivote_final = (void *)((size_t)pos_pivote_final +
						    1 * tamanio_elemento);
		}
	}
	swap(pos_pivote_final, pos_pivote_inicial, tamanio_elemento);

	// Sort bloque izquierda
	void *base_izquierda = base;
	size_t cantidad_izquierda =
		((size_t)pos_pivote_final - (size_t)base) / tamanio_elemento;
	quicksort(base_izquierda, cantidad_izquierda, tamanio_elemento,
		  comparador);

	// Sort bloque derecha
	void *base_derecha =
		(void *)((size_t)pos_pivote_final + 1 * tamanio_elemento);
	size_t cantidad_derecha = cantidad_elementos - cantidad_izquierda - 1;
	quicksort(base_derecha, cantidad_derecha, tamanio_elemento, comparador);
}

/**
 * Función auxiliar para que `strcmp` sea compatible con `qsort`
*/
int strcmp_qsort(const void *a_void, const void *b_void)
{
	const char **a = (const char **)a_void;
	const char **b = (const char **)b_void;

	return strcmp(*a, *b);
}

char *tp_nombres_disponibles(TP *tp)
{
	if (tp == NULL)
		return NULL;

	size_t cantidad = hash_cantidad(tp->pokemones);
	if (tp->jugador_1.pokemon != NULL)
		cantidad--;
	if (tp->jugador_2.pokemon != NULL)
		cantidad--;

	const char **nombres = malloc(cantidad * sizeof(char *));
	if (nombres == NULL)
		return NULL;

	contexto_agregar_nombre_t contexto = {
		.nombres = nombres,
		.indice = 0,
		.pokemon1 = tp->jugador_1.pokemon,
		.pokemon2 = tp->jugador_2.pokemon,
	};
	hash_con_cada_clave(tp->pokemones, agregar_nombre, &contexto);
	quicksort(nombres, cantidad, sizeof(char *), strcmp_qsort);

	size_t caracteres_totales = 0;
	for (size_t i = 0; i < cantidad; i++) {
		// Agregamos +1 por la ',' o por '\0'
		caracteres_totales += strlen(nombres[i]) + 1;
	}

	char *string = malloc(caracteres_totales * sizeof(char));
	if (string == NULL) {
		free(nombres);
		return NULL;
	}

	size_t caracteres_escritos = 0;
	for (size_t i = 0; i < cantidad; i++) {
		strcpy(string + caracteres_escritos, nombres[i]);
		caracteres_escritos += strlen(nombres[i]);

		if (i == cantidad - 1)
			*(string + caracteres_escritos++) = '\0';
		else
			*(string + caracteres_escritos++) = ',';
	}

	free(nombres);

	return string;
}

bool tp_seleccionar_pokemon(TP *tp, enum TP_JUGADOR jugador, const char *nombre)
{
	if (tp == NULL || nombre == NULL)
		return false;

	struct jugador *jugador_actual;
	struct jugador *jugador_rival;
	switch (jugador) {
	case JUGADOR_1:
		jugador_actual = &tp->jugador_1;
		jugador_rival = &tp->jugador_2;
		break;
	case JUGADOR_2:
		jugador_actual = &tp->jugador_2;
		jugador_rival = &tp->jugador_1;
		break;
	default:
		return false;
	}

	char buffer_nombre[MAX_NOMBRE_POKEMON + 1];
	strcpy_nombre_pokemon(buffer_nombre, nombre);

	struct pokemon_info *pokemon =
		hash_obtener(tp->pokemones, buffer_nombre);
	if (pokemon == NULL)
		return false;

	if (jugador_rival->pokemon != NULL && pokemon == jugador_rival->pokemon)
		return false;

	jugador_actual->pokemon = pokemon;
	return true;
}

const struct pokemon_info *tp_pokemon_seleccionado(TP *tp,
						   enum TP_JUGADOR jugador)
{
	if (tp == NULL)
		return NULL;

	switch (jugador) {
	case JUGADOR_1:
		return tp->jugador_1.pokemon;
	case JUGADOR_2:
		return tp->jugador_2.pokemon;
	default:
		return NULL;
	}
}

unsigned tp_agregar_obstaculo(TP *tp, enum TP_JUGADOR jugador,
			      enum TP_OBSTACULO obstaculo, unsigned posicion)
{
	if (tp == NULL)
		return 0;

	lista_t *obstaculos;
	switch (jugador) {
	case JUGADOR_1:
		obstaculos = tp->jugador_1.obstaculos;
		break;
	case JUGADOR_2:
		obstaculos = tp->jugador_2.obstaculos;
		break;
	default:
		return 0;
	}

	enum TP_OBSTACULO *obstaculo_a_insertar =
		malloc(sizeof(enum TP_OBSTACULO));
	if (obstaculo_a_insertar == NULL)
		return 0;

	*obstaculo_a_insertar = obstaculo;
	obstaculos = lista_insertar_en_posicion(obstaculos,
						obstaculo_a_insertar, posicion);
	if (obstaculos == NULL)
		return 0;

	return (unsigned)lista_tamanio(obstaculos);
}

unsigned tp_quitar_obstaculo(TP *tp, enum TP_JUGADOR jugador, unsigned posicion)
{
	if (tp == NULL)
		return 0;

	lista_t *obstaculos;
	switch (jugador) {
	case JUGADOR_1:
		obstaculos = tp->jugador_1.obstaculos;
		break;
	case JUGADOR_2:
		obstaculos = tp->jugador_2.obstaculos;
		break;
	default:
		return 0;
	}

	void *obstaculo = lista_quitar_de_posicion(obstaculos, posicion);
	if (obstaculo == NULL)
		return 0;

	free(obstaculo);
	return (unsigned)lista_tamanio(obstaculos);
}

/**
 * Struct auxiliar para `agregar_obstaculo_string`
*/
typedef struct contexto_agregar_obstaculo {
	char *string;
	size_t indice;
} contexto_agregar_obstaculo_t;

/**
 * Función auxiliar para `tp_obstaculos_pista`
*/
bool agregar_obstaculo_string(void *obstaculo_void, void *contexto_void)
{
	enum TP_OBSTACULO *obstaculo = obstaculo_void;
	contexto_agregar_obstaculo_t *contexto = contexto_void;

	char letra;
	switch (*obstaculo) {
	case OBSTACULO_FUERZA:
		letra = IDENTIFICADOR_OBSTACULO_FUERZA;
		break;

	case OBSTACULO_DESTREZA:
		letra = IDENTIFICADOR_OBSTACULO_DESTREZA;
		break;

	case OBSTACULO_INTELIGENCIA:
		letra = IDENTIFICADOR_OBSTACULO_INTELIGENCIA;
		break;

	default:
		letra = '?';
	}

	contexto->string[contexto->indice++] = letra;

	return true;
}

char *tp_obstaculos_pista(TP *tp, enum TP_JUGADOR jugador)
{
	if (tp == NULL)
		return NULL;

	lista_t *obstaculos;
	switch (jugador) {
	case JUGADOR_1:
		obstaculos = tp->jugador_1.obstaculos;
		break;
	case JUGADOR_2:
		obstaculos = tp->jugador_2.obstaculos;
		break;
	default:
		return NULL;
	}

	size_t cantidad_obstaculos = lista_tamanio(obstaculos);
	if (cantidad_obstaculos == 0)
		return NULL;

	char *string_obstaculos =
		malloc((cantidad_obstaculos + 1) * sizeof(char));
	if (string_obstaculos == NULL)
		return NULL;

	struct contexto_agregar_obstaculo contexto = {
		.indice = 0,
		.string = string_obstaculos,
	};

	lista_con_cada_elemento(obstaculos, agregar_obstaculo_string,
				&contexto);

	string_obstaculos[cantidad_obstaculos] = '\0';
	return string_obstaculos;
}

void tp_limpiar_pista(TP *tp, enum TP_JUGADOR jugador)
{
	if (tp == NULL)
		return;

	lista_t *obstaculos;
	switch (jugador) {
	case JUGADOR_1:
		obstaculos = tp->jugador_1.obstaculos;
		break;
	case JUGADOR_2:
		obstaculos = tp->jugador_2.obstaculos;
		break;
	default:
		return;
	}

	while (!lista_vacia(obstaculos)) {
		enum TP_OBSTACULO *obstaculo = lista_quitar(obstaculos);
		free(obstaculo);
	}
}

/**
 * Struct auxiliar para `sumar_tiempo`
*/
struct contexto_sumar_tiempo {
	struct pokemon_info *pokemon;
	size_t indice_obstaculo;
	unsigned *tiempos_por_obstaculo;
	unsigned tiempo_total;
	int valor_base;
	enum TP_OBSTACULO *obstaculo_anterior;
};

/**
 * Función auxiliar para `tp_calcular_tiempo_pista` y `tp_tiempo_por_obstaculo`
 * 
 * `tiempos_por_obstaculo` es opcional.
*/
bool sumar_tiempo(void *obstaculo_void, void *contexto_void)
{
	enum TP_OBSTACULO *obstaculo = obstaculo_void;
	struct contexto_sumar_tiempo *contexto = contexto_void;

	struct pokemon_info *pokemon = contexto->pokemon;

	if (contexto->obstaculo_anterior != NULL &&
	    *obstaculo == *contexto->obstaculo_anterior) {
		contexto->valor_base--;
	} else {
		contexto->obstaculo_anterior = obstaculo;
		contexto->valor_base = TIEMPO_VALOR_BASE;
	}

	int tiempo;
	switch (*obstaculo) {
	case OBSTACULO_FUERZA:
		tiempo = contexto->valor_base - pokemon->fuerza;
		break;

	case OBSTACULO_DESTREZA:
		tiempo = contexto->valor_base - pokemon->destreza;
		break;

	case OBSTACULO_INTELIGENCIA:
		tiempo = contexto->valor_base - pokemon->inteligencia;
		break;

	default:
		tiempo = 0;
	}
	if (tiempo < 0)
		tiempo = 0;

	if (contexto->tiempos_por_obstaculo != NULL)
		contexto->tiempos_por_obstaculo[contexto->indice_obstaculo] =
			(unsigned)tiempo;

	contexto->indice_obstaculo++;
	contexto->tiempo_total += (unsigned)tiempo;

	return true;
}

unsigned tp_calcular_tiempo_pista(TP *tp, enum TP_JUGADOR jugador)
{
	if (tp == NULL)
		return 0;

	lista_t *obstaculos;
	struct pokemon_info *pokemon;
	switch (jugador) {
	case JUGADOR_1:
		pokemon = tp->jugador_1.pokemon;
		obstaculos = tp->jugador_1.obstaculos;
		break;
	case JUGADOR_2:
		pokemon = tp->jugador_2.pokemon;
		obstaculos = tp->jugador_2.obstaculos;
		break;
	default:
		return 0;
	}

	if (pokemon == NULL || lista_tamanio(obstaculos) == 0)
		return 0;

	struct contexto_sumar_tiempo contexto = {
		.pokemon = pokemon,
		.tiempo_total = 0,
		.valor_base = TIEMPO_VALOR_BASE,
		.obstaculo_anterior = NULL,
		.indice_obstaculo = 0,
		.tiempos_por_obstaculo = NULL,
	};
	lista_con_cada_elemento(obstaculos, sumar_tiempo, &contexto);

	return contexto.tiempo_total;
}

char *tp_tiempo_por_obstaculo(TP *tp, enum TP_JUGADOR jugador)
{
	if (tp == NULL)
		return NULL;

	lista_t *obstaculos;
	struct pokemon_info *pokemon;
	switch (jugador) {
	case JUGADOR_1:
		pokemon = tp->jugador_1.pokemon;
		obstaculos = tp->jugador_1.obstaculos;
		break;

	case JUGADOR_2:
		pokemon = tp->jugador_2.pokemon;
		obstaculos = tp->jugador_2.obstaculos;
		break;

	default:
		return NULL;
	}

	size_t cantidad_obstaculos = lista_tamanio(obstaculos);
	if (pokemon == NULL || cantidad_obstaculos == 0)
		return NULL;

	unsigned *tiempos_por_obstaculo =
		malloc(cantidad_obstaculos * sizeof(unsigned));
	if (tiempos_por_obstaculo == NULL)
		return NULL;

	struct contexto_sumar_tiempo contexto = {
		.pokemon = pokemon,
		.tiempo_total = 0,
		.valor_base = TIEMPO_VALOR_BASE,
		.obstaculo_anterior = NULL,
		.indice_obstaculo = 0,
		.tiempos_por_obstaculo = tiempos_por_obstaculo,
	};
	lista_con_cada_elemento(obstaculos, sumar_tiempo, &contexto);

	// El búfer tiene tamaño de sobra, por simplicidad.
	char *tiempos_string =
		malloc(cantidad_obstaculos * MAX_STRING_NUMERO * sizeof(char));
	if (tiempos_string == NULL) {
		free(tiempos_por_obstaculo);
		return NULL;
	}

	size_t caracteres_totales = 0;
	for (size_t i = 0; i < cantidad_obstaculos; i++) {
		int caracteres_escritos = snprintf(
			tiempos_string + caracteres_totales,
			MAX_STRING_NUMERO + 1, "%d,", tiempos_por_obstaculo[i]);

		if (caracteres_escritos <= 0) {
			free(tiempos_por_obstaculo);
			free(tiempos_string);
			return NULL;
		}

		caracteres_totales += (size_t)caracteres_escritos;

		if (i == cantidad_obstaculos - 1)
			// Reemplazar ',' con '\0'
			tiempos_string[caracteres_totales - 1] = '\0';
	}
	free(tiempos_por_obstaculo);

	return tiempos_string;
}

void tp_destruir(TP *tp)
{
	if (tp == NULL)
		return;

	hash_destruir_todo(tp->pokemones, liberar_pokemon);
	lista_destruir_todo(tp->jugador_1.obstaculos, free);
	lista_destruir_todo(tp->jugador_2.obstaculos, free);
	free(tp);
}
