#include "split.h"

// La implementación original tenía unos errores
// así que aprovecho la lista para solucionarlos fácil.
#include "lista.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
 * Crea un string en el heap a partir de una sección de un string ya existente.
 * Se copian `tamaño` caracteres comenzando por la posición del puntero pasado.
 * Si `malloc` falla, se devuelve NULL.
 */
char *slice_a_string(const char *comienzo, size_t tamaño)
{
	// + 1 para hacer espacio al '\0'
	char *string = malloc((tamaño + 1) * sizeof(char));
	if (string == NULL)
		return NULL;

	strncpy(string, comienzo, tamaño);
	string[tamaño] = '\0';

	return string;
}

typedef struct contexto {
	char **vector;
	size_t indice;
} contexto_t;

/**
 * Función auxiliar que agrega los elementos de la lista
 * a un vector.
*/
bool agregar_a_vector(void *string_void, void *contexto_void)
{
	char *string = string_void;
	contexto_t *contexto = contexto_void;

	contexto->vector[contexto->indice++] = string;

	return true;
}

char **split(const char *string, char separador)
{
	if (string == NULL)
		return NULL;

	lista_t *lista = lista_crear();
	if (lista == NULL)
		return NULL;

	// Los strings son creados a partir de un slice,
	// con los límites definidos por estas variables
	size_t slice_comienzo = 0;
	size_t slice_tamaño = 0;

	for (size_t i = 0; string[i] != '\0'; i++) {
		if (string[i] == separador) {
			// Nuestro slice está listo para ser convertido
			// a un string y ser guardado en el vector final
			char *split_string = slice_a_string(
				string + slice_comienzo, slice_tamaño);
			if (split_string == NULL) {
				lista_destruir_todo(lista, free);
				return NULL;
			}

			void *exito = lista_insertar(lista, split_string);
			if (exito == NULL) {
				lista_destruir_todo(lista, free);
				return NULL;
			};

			// Preparar la siguiente string a ser splitteada
			slice_comienzo = i + 1;
			slice_tamaño = 0;
		} else {
			// El slice adquiere el caracter actual
			slice_tamaño++;
		}
	}

	// Queda el último string sin splittear,
	// N separadores significan N+1 substrings
	// https://en.wikipedia.org/wiki/Off-by-one_error#Fencepost_error
	char *split_string =
		slice_a_string(string + slice_comienzo, slice_tamaño);
	if (split_string == NULL) {
		lista_destruir_todo(lista, free);
		return NULL;
	}

	void *exito = lista_insertar(lista, split_string);
	if (exito == NULL) {
		lista_destruir_todo(lista, free);
		return NULL;
	};

	// NULL terminador
	exito = lista_insertar(lista, NULL);
	if (exito == NULL) {
		lista_destruir_todo(lista, free);
		return NULL;
	}

	contexto_t contexto = {
		.vector = malloc(lista_tamanio(lista) * sizeof(char *)),
		.indice = 0,
	};
	if (contexto.vector == NULL) {
		lista_destruir_todo(lista, free);
		return NULL;
	}

	lista_con_cada_elemento(lista, agregar_a_vector, &contexto);
	lista_destruir(lista);
	return contexto.vector;
}
