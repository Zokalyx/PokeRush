#include "lista.h"
#include <stddef.h>
#include <stdlib.h>

typedef struct nodo {
	void *elemento;
	struct nodo *siguiente;
} nodo_t;

struct lista {
	nodo_t *nodo_inicio;
	nodo_t *nodo_fin; // Requerido si queremos reusar este TDA para la cola y tener operaciones O(1)
};

struct lista_iterador {
	lista_t *lista;
	nodo_t *corriente;
};

lista_t *lista_crear()
{
	lista_t *lista = malloc(sizeof(lista_t));
	if (lista == NULL)
		return NULL;

	lista->nodo_inicio = NULL;
	lista->nodo_fin = NULL;

	return lista;
}

lista_t *lista_insertar(lista_t *lista, void *elemento)
{
	if (lista == NULL)
		return NULL;

	nodo_t *nodo = malloc(sizeof(nodo_t));
	if (nodo == NULL)
		return NULL;

	nodo->elemento = elemento;
	nodo->siguiente = NULL;

	if (lista->nodo_fin != NULL)
		// Por lo menos había un elemento antes
		lista->nodo_fin->siguiente = nodo;
	else
		// No hay elementos
		lista->nodo_inicio = nodo;

	lista->nodo_fin = nodo;

	return lista;
}

lista_t *lista_insertar_en_posicion(lista_t *lista, void *elemento,
				    size_t posicion)
{
	if (lista == NULL)
		return NULL;

	nodo_t *nodo = malloc(sizeof(nodo_t));
	if (nodo == NULL)
		return NULL;

	nodo->elemento = elemento;

	if (posicion == 0 || lista->nodo_inicio == NULL) {
		if (lista->nodo_inicio == NULL)
			lista->nodo_fin = nodo;
		nodo->siguiente = lista->nodo_inicio;
		lista->nodo_inicio = nodo;

	} else {
		// En esta rama, por lo menos hay un elemento en la lista,
		// y no estamos insertando al comienzo.
		nodo_t *nodo_anterior = lista->nodo_inicio;

		// Llegar hasta el último nodo, o hasta la posición indicada
		while (nodo_anterior->siguiente != NULL && posicion != 1) {
			nodo_anterior = nodo_anterior->siguiente;
			posicion--;
		}

		if (nodo_anterior->siguiente == NULL)
			lista->nodo_fin = nodo;
		nodo->siguiente = nodo_anterior->siguiente;
		nodo_anterior->siguiente = nodo;
	}

	return lista;
}

void *lista_quitar(lista_t *lista)
{
	if (lista == NULL || lista->nodo_inicio == NULL)
		return NULL;

	// Al ser una lista simplemente enlazada, debemos obtener
	// el anteúltimo nodo de esta manera, la cual es O(N).
	nodo_t *nodo = lista->nodo_inicio;
	nodo_t *nodo_anterior = NULL;
	while (nodo->siguiente != NULL) {
		nodo_anterior = nodo;
		nodo = nodo->siguiente;
	}

	if (nodo_anterior == NULL)
		// Solo había un elemento en la lista, ahora está vacía
		lista->nodo_inicio = NULL;
	else
		// Había más de un elemento, y ahora el anteúltimo se convierte en el último
		nodo_anterior->siguiente = NULL;

	lista->nodo_fin = nodo_anterior;

	void *elemento = nodo->elemento;
	free(nodo);

	return elemento;
}

void *lista_quitar_de_posicion(lista_t *lista, size_t posicion)
{
	if (lista == NULL || lista->nodo_inicio == NULL)
		return NULL;

	nodo_t *nodo = lista->nodo_inicio;
	nodo_t *nodo_anterior = NULL;
	while (nodo->siguiente != NULL && posicion != 0) {
		nodo_anterior = nodo;
		nodo = nodo->siguiente;
		posicion--;
	}

	if (nodo_anterior == NULL)
		// El nodo a destruir es el inicial
		lista->nodo_inicio = lista->nodo_inicio->siguiente;
	else
		// Lo importante de esta función
		nodo_anterior->siguiente = nodo->siguiente;

	if (nodo->siguiente == NULL)
		// El nodo a destruir es el final
		lista->nodo_fin = nodo_anterior;

	void *elemento = nodo->elemento;
	free(nodo);

	return elemento;
}

void *lista_quitar_elemento(lista_t *lista, int (*comparador)(void *, void *),
			    void *contexto)
{
	if (lista == NULL || lista->nodo_inicio == NULL)
		return NULL;

	nodo_t *nodo = lista->nodo_inicio;
	nodo_t *nodo_anterior = NULL;
	while (nodo != NULL) {
		if (comparador(nodo->elemento, contexto) == 0)
			break;

		nodo_anterior = nodo;
		nodo = nodo->siguiente;
	}
	if (nodo == NULL)
		return NULL;

	if (nodo_anterior == NULL)
		// El nodo a destruir es el inicial
		lista->nodo_inicio = lista->nodo_inicio->siguiente;
	else
		// Lo importante de esta función
		nodo_anterior->siguiente = nodo->siguiente;

	if (nodo->siguiente == NULL)
		// El nodo a destruir es el final
		lista->nodo_fin = nodo_anterior;

	void *elemento = nodo->elemento;
	free(nodo);

	return elemento;
}

void *lista_elemento_en_posicion(lista_t *lista, size_t posicion)
{
	if (lista == NULL)
		return NULL;

	nodo_t *nodo = lista->nodo_inicio;

	while (nodo != NULL && posicion != 0) {
		nodo = nodo->siguiente;
		posicion--;
	}

	if (nodo == NULL)
		return NULL;

	return nodo->elemento;
}

void *lista_buscar_elemento(lista_t *lista, int (*comparador)(void *, void *),
			    void *contexto)
{
	if (lista == NULL || comparador == NULL)
		return NULL;

	nodo_t *nodo = lista->nodo_inicio;
	void *elemento = NULL;

	while (nodo != NULL) {
		if (comparador(nodo->elemento, contexto) == 0) {
			elemento = nodo->elemento;
			break;
		}
		nodo = nodo->siguiente;
	}

	return elemento;
}

void *lista_primero(lista_t *lista)
{
	if (lista == NULL || lista->nodo_inicio == NULL)
		return NULL;

	return lista->nodo_inicio->elemento;
}

void *lista_ultimo(lista_t *lista)
{
	if (lista == NULL || lista->nodo_fin == NULL)
		return NULL;

	return lista->nodo_fin->elemento;
}

bool lista_vacia(lista_t *lista)
{
	if (lista == NULL)
		return true;

	return lista->nodo_inicio == NULL;
}

size_t lista_tamanio(lista_t *lista)
{
	if (lista == NULL)
		return 0;

	size_t tamanio = 0;

	nodo_t *nodo = lista->nodo_inicio;
	while (nodo != NULL) {
		nodo = nodo->siguiente;
		tamanio++;
	}

	return tamanio;
}

void lista_destruir(lista_t *lista)
{
	lista_destruir_todo(lista, NULL);
}

void lista_destruir_todo(lista_t *lista, void (*funcion)(void *))
{
	if (lista == NULL)
		return;

	nodo_t *nodo = lista->nodo_inicio;

	while (nodo != NULL) {
		nodo_t *aux = nodo->siguiente;
		if (funcion != NULL)
			funcion(nodo->elemento);
		free(nodo);
		nodo = aux;
	}

	free(lista);
}

lista_iterador_t *lista_iterador_crear(lista_t *lista)
{
	if (lista == NULL)
		return NULL;

	lista_iterador_t *lista_iterador = malloc(sizeof(lista_iterador_t));
	if (lista_iterador == NULL)
		return NULL;

	lista_iterador->lista = lista;
	lista_iterador->corriente = lista->nodo_inicio;

	return lista_iterador;
}

bool lista_iterador_tiene_siguiente(lista_iterador_t *iterador)
{
	if (iterador == NULL)
		return false;

	return iterador->corriente != NULL;
}

bool lista_iterador_avanzar(lista_iterador_t *iterador)
{
	if (iterador == NULL || iterador->corriente == NULL)
		return false;

	iterador->corriente = iterador->corriente->siguiente;
	return iterador->corriente != NULL;
}

void *lista_iterador_elemento_actual(lista_iterador_t *iterador)
{
	if (iterador == NULL || iterador->corriente == NULL)
		return NULL;

	return iterador->corriente->elemento;
}

void lista_iterador_destruir(lista_iterador_t *iterador)
{
	// Como el iterador no es el "dueño" de la lista, no debemos hacer mucho.
	free(iterador);
}

size_t lista_con_cada_elemento(lista_t *lista, bool (*funcion)(void *, void *),
			       void *contexto)
{
	if (lista == NULL || funcion == NULL)
		return 0;

	size_t elementos_recorridos = 0;
	nodo_t *nodo = lista->nodo_inicio;

	while (nodo != NULL) {
		elementos_recorridos++;
		if (!funcion(nodo->elemento, contexto))
			break;
		nodo = nodo->siguiente;
	}

	return elementos_recorridos;
}
