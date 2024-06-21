#ifndef __PANTALLA_H__
#define __PANTALLA_H__

#include <stdbool.h>
#include "color.h"
#include "estado.h"
#include "sprite.h"

/**
 * TDA que abstrae la terminal como una grilla
 * de caracteres y permite dibujar gráficos sobre
 * ella.
*/
typedef struct pantalla pantalla_t;

/**
 * Crea una pantalla con las dimensiones especificadas
 * (medidas en columnas y filas).
 * 
 * Además, se debe especificar la carpeta en donde
 * se encuentran los sprites, los cuales pueden
 * ser reutilizados al dibujar.
 * 
 * Opcionalmente se puede pasar una variable de
 * estado para saber qué error hubo.
*/
pantalla_t *pantalla_crear(unsigned ancho, unsigned alto, estado_t *estado);

/**
 * Setea el color de background de los caracteres como contexto
 * para llamadas a gráficos siguientes.
 */
void pantalla_color_fondo(pantalla_t *pantalla, uint8_t r, uint8_t g, uint8_t b,
			  float opacidad);

/**
 * Setea el color de foreground de los caracteres (su trazado en sí)
 * como contexto para llamadas a gráficos siguientes.
*/
void pantalla_color_texto(pantalla_t *pantalla, uint8_t r, uint8_t g, uint8_t b,
			  float opacidad);

/**
 * Configura el estilo del texto a ser escrito posteriormente.
*/
void pantalla_estilo_texto(pantalla_t *pantalla, bool negrita, bool subrayado,
			   bool italico);

/**
 * Limpia la pantalla de caracteres, aplicando el color de
 * fondo guardado.
*/
void pantalla_fondo(pantalla_t *pantalla);

/**
 * Dibuja un rectángulo con su esquina superior izquierda en (x, y), con un
 * ancho y alto dado. Debe pasarse un caracter que actúa como recuadro, el cual
 * puede ser un espacio si no se desea un recuadro visible.
*/
void pantalla_rectangulo(pantalla_t *pantalla, int x, int y, unsigned ancho,
			 unsigned alto, char borde);

/**
 * Dibuja arte ASCII especificado por parámetro.
 * Cada sprite tiene sus propios colores y dimensiones, por
 * lo cual solo se puede especificar su posición en pantalla.
*/
void pantalla_sprite(pantalla_t *pantalla, int x, int y, const sprite_t *sprite,
		     float opacidad);

/**
 * Escribe texto con formato empezando en la posición especificada.
*/
void pantalla_texto(pantalla_t *pantalla, int x, int y, const char *formato,
		    ...);

/**
 * Actualiza el frame actual para que se pueda ver en pantalla.
*/
void pantalla_actualizar_frame(pantalla_t *pantalla);

/**
 * Libera toda la memoria reservada por la pantalla.
 * Reestablece el comportamiento original de la terminal.
*/
void pantalla_destruir(pantalla_t *pantalla);

#endif // __PANTALLA_H__