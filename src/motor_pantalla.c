#include "motor_pantalla.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// ANSI Escape Codes
#define ESC "\x1b"
#define CSI "["

#define BACKGROUND_RGB_PREFIX "48;2;"
#define FOREGROUND_RGB_PREFIX "38;2;"
#define RGB_POSTFIX "m"
#define RESET_COLORES "0m"
#define ENCENDER_NEGRITA "1m"
#define RESETEAR_NEGRITA "22m"
#define ENCENDER_ITALICO "3m"
#define RESETEAR_ITALICO "23m"
#define ENCENDER_SUBRAYO "4m"
#define RESETEAR_SUBRAYO "24m"

#define BORRAR_TODO "2J"
#define HOME "H"

#define VISIBILIDAD_CURSOR "?25"
#define OFF "l"
#define ON "h"

// Se buscó el mejor valor con prueba y error.
#define TAMANIO_BUFFER_STDOUT 4096

#define LONGITUD_MAXIMA_TEXTO 127

/**
 * "Meta-data" de cada caracter
 * (estilo en pantalla).
*/
typedef struct meta_caracter {
	color_t fondo;
	color_t texto;
	bool negrita : 1;
	bool subrayado : 1;
	bool italico : 1;
} meta_caracter_t;

/**
 * Estilo con el que vamos a escribir
 * el siguiente caracter.
 * 
 * Si hubiera opacidad menor a 1, se mezcla
 * con el color anterior, ya existente.
 */
typedef struct paleta {
	color_t fondo;
	float opacidad_fondo;
	color_t texto;
	float opacidad_texto;
	bool negrita : 1;
	bool subrayado : 1;
	bool italico : 1;
} paleta_t;

struct pantalla {
	// Tamaño del viewport donde se ve el juego
	unsigned ancho, alto;

	// Auxiliar, utilizado como búfer de stdout.
	char *buffer_stdout;

	// Donde se almacena todo lo que será printeado en el frame actual
	char *buffer_caracteres;
	// Donde se almacena los colores y estilo de cada caracter
	meta_caracter_t *meta_caracteres;

	// Con qué pintar o estilizar los caracteres
	// a ser dibujados. Una especie de "contexto".
	paleta_t paleta;
};

/**
 * Aplica la paleta al color final en pantalla,
 * teniendo en cuenta la opacidad.
*/
void mezclar_paleta(paleta_t *paleta, meta_caracter_t *meta)
{
	meta->italico = paleta->italico;
	meta->subrayado = paleta->subrayado;
	meta->negrita = paleta->negrita;

	meta->fondo = color_mezcla(paleta->fondo, meta->fondo,
				   paleta->opacidad_fondo);
	meta->texto = color_mezcla(paleta->texto, meta->fondo,
				   paleta->opacidad_texto);
}

/**
 * Evita que `stdout` se flushee automáticamente
 * con cada newline (modo fully buffered)
 * 
 * Devuelve el buffer externo a ser utilizado.
*/
char *configurar_stdout(size_t tamanio_buffer)
{
	char *buffer = malloc(tamanio_buffer);
	if (buffer == NULL)
		return NULL;

	if (setvbuf(stdout, buffer, _IOFBF, tamanio_buffer) != 0) {
		free(buffer);
		return NULL;
	}

	return buffer;
}

/**
 * Reestablecer el búfer default de `stdout`
*/
void reestablecer_stdout()
{
	// Si esto falla ni idea qué se puede hacer

	// Eliminar búfering completamente
	setbuf(stdout, NULL);

	// Asociar un nuevo búfer con tamaño default
	// y comportamiento default (line buffering)
	setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
}

/**
 * Mueve el cursor de la terminal a una coordenada
 * específica relativo al origen de la pantalla
*/
void mover_cursor_a(pantalla_t *pantalla, int x, int y)
{
	printf(ESC CSI "%d;%dH", y, x);
}

pantalla_t *pantalla_crear(unsigned ancho, unsigned alto, estado_t *estado)
{
	char *buffer_stdout =
		configurar_stdout(TAMANIO_BUFFER_STDOUT * sizeof(char));
	if (buffer_stdout == NULL) {
		if (estado != NULL)
			*estado = ERROR_STDOUT;
		return NULL;
	}

	pantalla_t *pantalla = malloc(sizeof(pantalla_t));
	if (pantalla == NULL) {
		free(buffer_stdout);
		if (estado != NULL)
			*estado = ERROR_MEMORIA;
		return NULL;
	}

	unsigned tamanio_buffer = ancho * alto;
	char *buffer = malloc(tamanio_buffer * sizeof(char));
	if (buffer == NULL) {
		free(pantalla);
		if (estado != NULL)
			*estado = ERROR_MEMORIA;
		return NULL;
	}
	memset(buffer, 0, tamanio_buffer * sizeof(char));

	meta_caracter_t *meta =
		malloc(tamanio_buffer * sizeof(meta_caracter_t));
	if (meta == NULL) {
		free(pantalla);
		free(buffer);
		if (estado != NULL)
			*estado = ERROR_MEMORIA;
		return NULL;
	}
	memset(meta, 0, tamanio_buffer * sizeof(meta_caracter_t));

	pantalla->ancho = ancho;
	pantalla->alto = alto;
	pantalla->buffer_caracteres = buffer;
	pantalla->meta_caracteres = meta;
	memset(&pantalla->paleta, 0, sizeof(paleta_t));
	pantalla->paleta.opacidad_fondo = 1.0f;
	pantalla->paleta.opacidad_texto = 1.0f;
	pantalla->buffer_stdout = buffer_stdout;

	printf(ESC CSI VISIBILIDAD_CURSOR OFF);
	printf(ESC CSI BORRAR_TODO);
	fflush(stdout);

	return pantalla;
}

void pantalla_color_fondo(pantalla_t *pantalla, uint8_t r, uint8_t g, uint8_t b,
			  float opacidad)
{
	if (pantalla == NULL)
		return;

	pantalla->paleta.fondo = color_crear(r, g, b);
	pantalla->paleta.opacidad_fondo = opacidad;
}

void pantalla_color_texto(pantalla_t *pantalla, uint8_t r, uint8_t g, uint8_t b,
			  float opacidad)
{
	if (pantalla == NULL)
		return;

	pantalla->paleta.texto = color_crear(r, g, b);
	pantalla->paleta.opacidad_texto = opacidad;
}

/**
 * Escribe un caracter en buffer de pantalla con el estilo
 * indicado en la paleta.
*/
void pantalla_escribir(pantalla_t *pantalla, int x, int y, char caracter)
{
	if (x < 0 || x >= pantalla->ancho || y < 0 || y >= pantalla->alto)
		return;

	unsigned pos_buffer = (unsigned)y * pantalla->ancho + (unsigned)x;
	pantalla->buffer_caracteres[pos_buffer] = caracter;
	mezclar_paleta(&pantalla->paleta,
		       pantalla->meta_caracteres + pos_buffer);
}

void pantalla_estilo_texto(pantalla_t *pantalla, bool negrita, bool subrayado,
			   bool italico)
{
	if (pantalla == NULL)
		return;

	pantalla->paleta.negrita = negrita;
	pantalla->paleta.subrayado = subrayado;
	pantalla->paleta.italico = italico;
}

void pantalla_fondo(pantalla_t *pantalla)
{
	if (pantalla == NULL)
		return;

	for (int y = 0; y < pantalla->alto; y++)
		for (int x = 0; x < pantalla->ancho; x++)
			pantalla_escribir(pantalla, x, y, ' ');
}

void pantalla_texto(pantalla_t *pantalla, int x, int y, const char *formato,
		    ...)
{
	if (pantalla == NULL || formato == NULL)
		return;

	if (y < 0 || y >= pantalla->alto) {
		return;
	}

	char texto[LONGITUD_MAXIMA_TEXTO + 1];

	va_list args;
	va_start(args, formato);
	int escrito = vsnprintf(texto, LONGITUD_MAXIMA_TEXTO, formato, args);
	va_end(args);

	if (escrito <= 0)
		return;

	for (int i = 0; texto[i] != '\0'; i++)
		pantalla_escribir(pantalla, x + i, y, texto[i]);
}

void pantalla_rectangulo(pantalla_t *pantalla, int x, int y, unsigned ancho,
			 unsigned alto, char borde)
{
	if (pantalla == NULL)
		return;

	for (int j = 0; j < alto; j++)
		for (int i = 0; i < ancho; i++) {
			if (i != 0 && i + 1 != ancho && j != 0 && j + 1 != alto)
				borde = ' ';

			pantalla_escribir(pantalla, x + i, y + j, borde);
		}
}

void pantalla_sprite(pantalla_t *pantalla, int x, int y, const sprite_t *sprite,
		     float opacidad)
{
	if (pantalla == NULL || sprite == NULL)
		return;

	paleta_t paleta_original = pantalla->paleta;

	for (unsigned j = 0; j < sprite->alto; j++)
		for (unsigned i = 0; i < sprite->ancho; i++) {
			unsigned posicion = j * sprite->ancho + i;
			if (sprite->mascara[posicion]) {
				pantalla->paleta.opacidad_fondo = opacidad;
				pantalla->paleta.fondo =
					sprite->color_fondo[posicion];
				pantalla_escribir(pantalla, x + (int)i,
						  y + (int)j, ' ');
			}
		}

	pantalla->paleta = paleta_original;
}

void pantalla_actualizar_frame(pantalla_t *pantalla)
{
	if (pantalla == NULL)
		return;

	meta_caracter_t meta_anterior = pantalla->meta_caracteres[0];
	bool primer_caracter = true;

	for (unsigned y = 0; y < pantalla->alto; y++) {
		printf(ESC CSI "%d;%dH", y, 0);

		for (unsigned x = 0; x < pantalla->ancho; x++) {
			unsigned pos_buffer = y * pantalla->ancho + x;

			meta_caracter_t meta_actual =
				pantalla->meta_caracteres[pos_buffer];

			if (meta_actual.negrita != meta_anterior.negrita ||
			    primer_caracter) {
				if (meta_actual.negrita)
					printf(ESC CSI ENCENDER_NEGRITA);
				else
					printf(ESC CSI RESETEAR_NEGRITA);
			}

			if (meta_actual.subrayado != meta_anterior.subrayado ||
			    primer_caracter) {
				if (meta_actual.subrayado)
					printf(ESC CSI ENCENDER_SUBRAYO);
				else
					printf(ESC CSI RESETEAR_SUBRAYO);
			}

			if (meta_actual.italico != meta_anterior.italico ||
			    primer_caracter) {
				if (meta_actual.italico)
					printf(ESC CSI ENCENDER_ITALICO);
				else
					printf(ESC CSI RESETEAR_ITALICO);
			}

			if (!color_iguales(meta_actual.fondo,
					   meta_anterior.fondo) ||
			    primer_caracter)
				printf(ESC CSI BACKGROUND_RGB_PREFIX
				       "%d;%d;%d" RGB_POSTFIX,
				       meta_actual.fondo.r, meta_actual.fondo.g,
				       meta_actual.fondo.b);

			if (!color_iguales(meta_actual.texto,
					   meta_anterior.texto) ||
			    primer_caracter)
				printf(ESC CSI FOREGROUND_RGB_PREFIX
				       "%d;%d;%d" RGB_POSTFIX,
				       meta_actual.texto.r, meta_actual.texto.g,
				       meta_actual.texto.b);

			printf("%c", pantalla->buffer_caracteres[pos_buffer]);
			primer_caracter = false;
			meta_anterior = meta_actual;
		}
	}

	fflush(stdout);
}

void pantalla_destruir(pantalla_t *pantalla)
{
	if (pantalla == NULL)
		return;

	printf(ESC CSI VISIBILIDAD_CURSOR ON);
	printf(ESC CSI RESET_COLORES);
	printf(ESC CSI BORRAR_TODO);
	printf(ESC CSI HOME);
	fflush(stdout);

	reestablecer_stdout();
	free(pantalla->buffer_caracteres);
	free(pantalla->meta_caracteres);
	free(pantalla->buffer_stdout);
	free(pantalla);
}
