#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "hash.h"

#define FACTOR_CARGA_MAXIMA 0.7
#define FACTOR_REHASH 2
#define CAPACIDAD_MINIMA 3

typedef struct entrada {
	char *clave;
	void *elemento;
} entrada_t;

typedef size_t (*funcion_hash_t)(const char *clave);

struct hash {
	funcion_hash_t funcion_hash;
	entrada_t *tabla;
	size_t capacidad;
	size_t cantidad;
};

/**
 * MurmurHash - One (Byte) At A Time - 32 bits
 * https://stackoverflow.com/a/57960443/10236655
*/
size_t murmurOAAT32(const char *clave)
{
	uint32_t h = 3323198485ul;
	while (*clave) {
		h = (uint32_t)((int)h ^ (*clave++));
		h *= (0x5bd1e995);
		h ^= (h >> 15);
	}
	return (size_t)h;
}

/**
 * Busca la primera entrada vacía o que coincide con
 * la clave pasada, empezando por la posición especificada.
*/
size_t hash_probing(entrada_t *tabla, size_t capacidad, size_t posicion,
		    const char *clave)
{
	bool posicion_encontrada = false;
	while (!posicion_encontrada) {
		entrada_t entrada = tabla[posicion];

		if (entrada.clave == NULL || strcmp(entrada.clave, clave) == 0)
			posicion_encontrada = true;
		else
			posicion = (posicion + 1) % capacidad;

		// Este loop debería terminar si se respetan
		// las reglas del rehash. Siempre debería haber
		// algún espacio disponible.
	}

	return posicion;
}

/**
 * Agranda la tabla de hash para poder 
 * ocupar más elementos sin tantas colisiones.
 * 
 * Se utiliza una macro como factor de agrandamiento.
 * 
 * Devuelve NULL si falla.
*/
hash_t *hash_rehash(hash_t *hash)
{
	size_t nueva_capacidad = hash->capacidad * FACTOR_REHASH;

	entrada_t *nueva_tabla_hash =
		malloc(nueva_capacidad * sizeof(entrada_t));
	if (nueva_tabla_hash == NULL)
		return NULL;

	for (size_t i = 0; i < nueva_capacidad; i++) {
		nueva_tabla_hash[i].clave = NULL;
		nueva_tabla_hash[i].elemento = NULL;
	}

	for (size_t i = 0; i < hash->capacidad; i++) {
		entrada_t entrada = hash->tabla[i];
		if (entrada.clave == NULL)
			continue;

		size_t nueva_posicion = hash_probing(
			nueva_tabla_hash, nueva_capacidad,
			hash->funcion_hash(entrada.clave) % nueva_capacidad,
			entrada.clave);
		nueva_tabla_hash[nueva_posicion].clave = entrada.clave;
		nueva_tabla_hash[nueva_posicion].elemento = entrada.elemento;
	}

	free(hash->tabla);
	hash->tabla = nueva_tabla_hash;
	hash->capacidad = nueva_capacidad;

	return hash;
}

hash_t *hash_crear(size_t capacidad)
{
	if (capacidad < CAPACIDAD_MINIMA)
		capacidad = CAPACIDAD_MINIMA;

	hash_t *hash = malloc(sizeof(hash_t));
	if (hash == NULL)
		return NULL;

	entrada_t *tabla = malloc(capacidad * sizeof(entrada_t));
	if (tabla == NULL) {
		free(hash);
		return NULL;
	}

	for (size_t i = 0; i < capacidad; i++)
		tabla[i].clave = NULL;

	hash->tabla = tabla;
	hash->funcion_hash = murmurOAAT32;
	hash->capacidad = capacidad;
	hash->cantidad = 0;

	return hash;
}

hash_t *hash_insertar(hash_t *hash, const char *clave, void *elemento,
		      void **anterior)
{
	if (hash == NULL || clave == NULL)
		return NULL;

	float nuevo_factor_carga =
		(((float)hash->cantidad + 1) / (float)hash->capacidad);

	if (nuevo_factor_carga > FACTOR_CARGA_MAXIMA) {
		hash = hash_rehash(hash);
		if (hash == NULL)
			return NULL;
	}

	size_t posicion = hash_probing(
		hash->tabla, hash->capacidad,
		hash->funcion_hash(clave) % hash->capacidad, clave);
	entrada_t *entrada = &hash->tabla[posicion];

	if (entrada->clave != NULL) {
		// Modificación
		if (anterior != NULL)
			*anterior = entrada->elemento;
		entrada->elemento = elemento;
		return hash;
	}

	// Creación
	char *copia_clave = malloc((strlen(clave) + 1) * sizeof(char));
	if (copia_clave == NULL)
		return NULL;

	strcpy(copia_clave, clave);
	entrada->clave = copia_clave;
	entrada->elemento = elemento;
	hash->cantidad++;
	if (anterior != NULL)
		*anterior = NULL;

	return hash;
}

/**
 * Devuelve -1, 0 o 1 según el orden cíclico de sus elementos.
 * +1: A<B<C, B<C<A, C<A<B
 *  0: A==B,  B==C,  C==A
 * -1: A>B>C, B>C>A, C>A>B
*/
int orden_ciclico(size_t a, size_t b, size_t c)
{
	if (a == b || b == c || c == a)
		return 0;

	if (a < c)
		return a < b && b < c ? 1 : -1;
	else // c < a
		return b < c || a < b ? 1 : -1;
}

void *hash_quitar(hash_t *hash, const char *clave)
{
	if (hash == NULL || clave == NULL)
		return NULL;

	size_t posicion = hash_probing(
		hash->tabla, hash->capacidad,
		hash->funcion_hash(clave) % hash->capacidad, clave);
	entrada_t *entrada = &hash->tabla[posicion];

	if (entrada->clave == NULL)
		return NULL;

	void *elemento = entrada->elemento;
	free(entrada->clave);
	entrada->clave = NULL;
	entrada->elemento = NULL;
	hash->cantidad--;

	// Variables nuevas solo para mayor legibilidad
	size_t posicion_libre = posicion;
	entrada_t *entrada_libre = entrada;

	size_t posicion_next_real = (posicion + 1) % hash->capacidad;
	entrada_t *entrada_next = &hash->tabla[posicion_next_real];

	while (entrada_next->clave != NULL) {
		size_t posicion_next_hash =
			hash->funcion_hash(entrada_next->clave) %
			hash->capacidad;

		if (posicion_next_hash == posicion_libre ||
		    orden_ciclico(posicion_next_hash, posicion_libre,
				  posicion_next_real) == 1) {
			entrada_libre->clave = entrada_next->clave;
			entrada_libre->elemento = entrada_next->elemento;

			entrada_next->clave = NULL;
			entrada_next->elemento = NULL;

			posicion_libre = posicion_next_real;
			entrada_libre = entrada_next;
		}

		posicion_next_real = (posicion_next_real + 1) % hash->capacidad;
		entrada_next = &hash->tabla[posicion_next_real];
	}

	return elemento;
}

void *hash_obtener(hash_t *hash, const char *clave)
{
	if (hash == NULL || clave == NULL)
		return false;

	size_t posicion = hash_probing(
		hash->tabla, hash->capacidad,
		hash->funcion_hash(clave) % hash->capacidad, clave);
	entrada_t entrada = hash->tabla[posicion];

	if (entrada.clave == NULL)
		return NULL;

	return entrada.elemento;
}

bool hash_contiene(hash_t *hash, const char *clave)
{
	return hash_obtener(hash, clave) != NULL;
}

size_t hash_cantidad(hash_t *hash)
{
	if (hash == NULL)
		return 0;

	return hash->cantidad;
}

void hash_destruir(hash_t *hash)
{
	hash_destruir_todo(hash, NULL);
}

void hash_destruir_todo(hash_t *hash, void (*destructor)(void *))
{
	if (hash == NULL)
		return;

	for (size_t i = 0; i < hash->capacidad; i++) {
		entrada_t *entrada = &hash->tabla[i];
		if (entrada->clave == NULL)
			continue;

		hash->cantidad--;
		if (destructor != NULL)
			destructor(entrada->elemento);
		free(entrada->clave);
	}
	free(hash->tabla);
	free(hash);
}

size_t hash_con_cada_clave(hash_t *hash,
			   bool (*f)(const char *clave, void *valor, void *aux),
			   void *aux)
{
	if (hash == NULL || f == NULL)
		return 0;

	size_t recorridos = 0;
	for (size_t i = 0; i < hash->capacidad; i++) {
		entrada_t entrada = hash->tabla[i];
		if (entrada.clave == NULL)
			continue;

		recorridos++;
		if (!f(entrada.clave, entrada.elemento, aux))
			break;
	}

	return recorridos;
}
