#ifndef __DIRECTORIO_H__
#define __DIRECTORIO_H__

#include <stdlib.h>
#include <stdio.h>

/**
 * Aplica la función a todos los archivos del directorio.
 * 
 * Devuelve la cantidad de archivos recorridos o -1 en caso
 * de error.
*/
int iterar_directorio(const char *nombre_directorio,
		      void (*funcion)(FILE *, const char *, void *), void *,
		      const char *modo_abrir);

#endif // __DIRECTORIO_H__