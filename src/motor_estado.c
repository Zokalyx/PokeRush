#include "motor_estado.h"
#include <stdio.h>

#define TEXTO_ROJO "\x1b[31m"
#define TEXTO_VERDE "\x1b[32m"
#define TEXTO_NORMAL "\x1b[0m"

void estado_mostrar(estado_t estado)
{
	if (estado == FINALIZADO_POR_USUARIO) {
		printf(TEXTO_VERDE "Finalizado correctamente.\n" TEXTO_NORMAL);
		return;
	}

	printf(TEXTO_ROJO "Error: ");
	switch (estado) {
	case FINALIZADO_POR_USUARIO:
		break;
	case ERROR_MEMORIA:
		printf("Error al malloc'ear! Hay poca memoria disponible?");
		break;
	case ERROR_BUFFER_STDOUT:
		printf("Error al configurar stdout.");
		break;
	case ERROR_CREACION_ESCENA:
		printf("Error al crear una escena del juego :(");
		break;
	case JUEGO_INVALIDO:
		printf("El juego cargado no es válido.");
		break;
	case PUNTERO_NULL:
		printf("Hay algún puntero importante en NULL.");
		break;
	case CONFIGURACION_JUEGO_INVALIDA:
		printf("La configuración pasada es inválida.");
		break;
	case ERROR_CREACION_TP:
		printf("Error al crear el TP. Asegurate de que el archivo de pokemones.csv sea el correcto.");
		break;
	case ERROR_LECTURA_LISTA_BMP:
		printf("Error al leer la lista de sprites.");
		break;
	case ERROR_LECTURA_SPRITE:
		printf("Error al leer un sprite.");
		break;
	case ERROR_LEER_BITMAP:
		printf("Error al leer una imagen Bitmap.");
		break;
	case ERROR_BITMAP_INVALIDO:
		printf("Error al leer una imagen Bitmap.");
		break;
	case SENIAL_INTERRUPCION:
		printf("Señal de interrupción recibida.");
		break;
	}
	printf("\n" TEXTO_NORMAL);
}