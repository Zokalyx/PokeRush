#include "motor_sprite.h"

#include <stdint.h>
#include <stdlib.h>

#define GREEN_SCREEN_R 0
#define GREEN_SCREEN_G 255
#define GREEN_SCREEN_B 0
#define PROPORCION_X_Y 2

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct bmp_header {
	uint16_t firma;
	uint32_t tamanio_bytes;
	uint16_t reservado1;
	uint16_t reservado2;
	uint32_t offset_pixeles;
}
#ifdef __GNUC__
__attribute__((packed))
#endif
bmp_header_t;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct dib_header {
	uint32_t tamanio_header;
	int32_t ancho;
	int32_t alto;
	uint16_t planos_de_color;
	uint16_t bits_por_pixel;
	uint32_t compresion;
	uint32_t tamanio_imagen;
	int32_t resolucion_horizontal;
	int32_t resolucion_vertical;
	uint32_t colors_paleta;
	uint32_t colores_importantes;
}
#ifdef __GNUC__
__attribute__((packed))
#endif
dib_header_t;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

/**
 * Representa un pixel del bmp.
*/
typedef struct bmp_pixel {
	uint8_t b;
	uint8_t g;
	uint8_t r;
}
#ifdef __GNUC__
__attribute__((packed))
#endif
bmp_pixel_t;

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

/**
 * Retorna true si el encabezado cumple con lo pedido para el sprite:
 * - Formato BMP
 * - Color 24 bits
 * - Sin compresión
*/
bool bmp_valido(bmp_header_t *encabezado_bmp, dib_header_t *encabezado_dib)
{
	return encabezado_bmp->firma == 0x4d42 &&
	       encabezado_dib->compresion == 0 &&
	       encabezado_dib->bits_por_pixel == 24;
}

/**
 * Genera el vector de colores y la máscara
 * a partir de los pixeles.
*/
sprite_t *procesar_pixeles(unsigned ancho, unsigned alto, bmp_pixel_t *pixeles)
{
	sprite_t *sprite = malloc(sizeof(sprite_t));
	if (sprite == NULL)
		return NULL;

	unsigned tamanio_bmp = ancho * alto;

	unsigned ancho_final = PROPORCION_X_Y * ancho;
	unsigned tamanio_final = ancho_final * alto;

	color_t *colores = malloc(tamanio_final * sizeof(color_t));
	if (colores == NULL) {
		free(sprite);
		return NULL;
	}

	bool *mascara = malloc(tamanio_final * sizeof(bool));
	if (mascara == NULL) {
		free(sprite);
		free(colores);
		return NULL;
	}

	for (unsigned i = 0; i < tamanio_bmp; i++) {
		for (unsigned j = 0; j < PROPORCION_X_Y; j++) {
			bmp_pixel_t p = pixeles[i];

			if (p.r == GREEN_SCREEN_R && p.g == GREEN_SCREEN_G &&
			    p.b == GREEN_SCREEN_B) {
				mascara[PROPORCION_X_Y * i + j] = false;
			} else {
				colores[PROPORCION_X_Y * i + j].r = p.r;
				colores[PROPORCION_X_Y * i + j].g = p.g;
				colores[PROPORCION_X_Y * i + j].b = p.b;
				mascara[PROPORCION_X_Y * i + j] = true;
			}
		}
	}

	sprite->ancho = ancho_final;
	sprite->alto = alto;
	sprite->color_fondo = colores;
	sprite->mascara = mascara;

	return sprite;
}

sprite_t *sprite_crear(FILE *archivo, estado_t *estado)
{
	if (archivo == NULL || estado == NULL)
		return NULL;

	bmp_header_t encabezado_bmp;
	size_t leido = fread(&encabezado_bmp, sizeof(bmp_header_t), 1, archivo);
	if (!leido) {
		*estado = ERROR_LEER_BITMAP;
		return NULL;
	}

	dib_header_t encabezado_dib;
	leido = fread(&encabezado_dib, sizeof(dib_header_t), 1, archivo);
	if (!leido) {
		*estado = ERROR_LEER_BITMAP;
		return NULL;
	}

	if (!bmp_valido(&encabezado_bmp, &encabezado_dib)) {
		*estado = ERROR_BITMAP_INVALIDO;
		return NULL;
	}

	printf("  Encabezado válido.\n");
	printf("  Dimensiones: %i x %i\n", encabezado_dib.ancho,
	       encabezado_dib.alto);

	unsigned ancho = (unsigned)encabezado_dib.ancho;
	unsigned alto = encabezado_dib.alto < 0 ?
				(unsigned)(-encabezado_dib.alto) :
				(unsigned)encabezado_dib.alto;
	unsigned tamanio = ancho * alto;

	bmp_pixel_t *pixeles = malloc(tamanio * sizeof(bmp_pixel_t));
	if (pixeles == NULL) {
		*estado = ERROR_MEMORIA;
		return NULL;
	}

	printf("  Pixeles creados.\n");

	fseek(archivo, (long)encabezado_bmp.offset_pixeles, SEEK_SET);
	// https://en.wikipedia.org/wiki/BMP_file_format#Pixel_storage
	unsigned bytes_por_fila =
		(unsigned)(ancho * sizeof(bmp_pixel_t) * 8 + 31) / 32 * 4;
	unsigned bytes_padding =
		bytes_por_fila - (unsigned)(ancho * sizeof(bmp_pixel_t));

	// El formato que tiene el BMP "dado vuelta".
	// Se podría flexibilizar fácilmente viendo el signo
	// de la altura en el header.
	for (unsigned i = 0; i < alto; i++) {
		leido = fread(pixeles + ancho * (alto - 1 - i),
			      sizeof(bmp_pixel_t), ancho, archivo);
		if (leido != ancho) {
			*estado = ERROR_LEER_BITMAP;
			free(pixeles);
			return NULL;
		}
		fseek(archivo, (long)bytes_padding, SEEK_CUR);
	}

	printf("  Lectura exitosa.\n");

	sprite_t *sprite = procesar_pixeles(ancho, alto, pixeles);
	if (sprite == NULL)
		*estado = ERROR_MEMORIA;

	free(pixeles);

	return sprite;
}

void sprite_destruir(sprite_t *sprite)
{
	if (sprite == NULL)
		return;

	free(sprite->mascara);
	free(sprite->color_fondo);
	free(sprite);
}