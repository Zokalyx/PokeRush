#include "motor_directorio.h"

#ifdef _WIN32
#include <windows.h>
#include <stdbool.h>
#endif

#ifdef __unix__
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#define MAX_PATH 1024
#endif

int iterar_directorio(const char *nombre_directorio,
		      void (*funcion)(FILE *, const char *, void *),
		      void *contexto, const char *modo_abrir)
{
	if (nombre_directorio == NULL || funcion == NULL)
		return -1;

	int recorridos = 0;

#ifdef _WIN32
	char todos_los_archivos[MAX_PATH + 1];
	snprintf(todos_los_archivos, MAX_PATH + 1, "%s\\*", nombre_directorio);
	// TODO: Verificar este snprintf.
	// Aunque algo más adelante fallaría y devolvería -1 de todas maneras.

	WIN32_FIND_DATA informacion_archivo;
	HANDLE handle = FindFirstFile(todos_los_archivos, &informacion_archivo);
	if (handle == INVALID_HANDLE_VALUE)
		return -1;

	char path_completo[MAX_PATH + 1];
	do {
		if (informacion_archivo.dwFileAttributes &
		    FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}

		snprintf(path_completo, MAX_PATH + 1, "%s\\%s",
			 nombre_directorio, informacion_archivo.cFileName);

		FILE *archivo = fopen(path_completo, modo_abrir);
		if (archivo == NULL)
			continue;

		funcion(archivo, informacion_archivo.cFileName, contexto);
		fclose(archivo);
		recorridos++;
	} while (FindNextFile(handle, &informacion_archivo));

	FindClose(handle);
#endif

#ifdef __unix__
	DIR *directorio = opendir(nombre_directorio);
	if (directorio == NULL)
		return -1;

	struct stat datos_archivo;
	char path_completo[MAX_PATH + 1];
	struct dirent *entrada;
	while ((entrada = readdir(directorio)) != NULL) {
		if (strcmp(entrada->d_name, ".") == 0 ||
		    strcmp(entrada->d_name, "..") == 0)
			continue;

		snprintf(path_completo, MAX_PATH + 1, "%s/%s",
			 nombre_directorio, entrada->d_name);
		int estado_stat = stat(path_completo, &datos_archivo);
		if (estado_stat == -1)
			continue;

		if (S_ISDIR(datos_archivo.st_mode))
			continue;

		FILE *archivo = fopen(path_completo, modo_abrir);
		if (archivo == NULL)
			continue;

		funcion(archivo, entrada->d_name, contexto);
		fclose(archivo);
		recorridos++;
	}

	closedir(directorio);
#endif

	return recorridos;
}