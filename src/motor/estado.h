#ifndef __ESTADO_H__
#define __ESTADO_H__

typedef enum estado {
	// Propio del funcionamiento interno del motor
	FINALIZADO_POR_USUARIO,
	ERROR_MEMORIA,
	ERROR_BUFFER_STDOUT,
	JUEGO_INVALIDO,
	PUNTERO_NULL,
	SENIAL_INTERRUPCION,
	// De funcionalidades secundarias del motor
	ERROR_LECTURA_SPRITE,
	ERROR_LECTURA_LISTA_BMP,
	ERROR_LEER_BITMAP,
	ERROR_BITMAP_INVALIDO,
	// Del juego
	CONFIGURACION_JUEGO_INVALIDA,
	ERROR_CREACION_ESCENA,
	ERROR_CREACION_TP,
} estado_t;

/**
 * Muestra en pantalla un mensaje amigable
 * al usuario.
*/
void estado_mostrar(estado_t estado);

#endif // __ESTADO_H__