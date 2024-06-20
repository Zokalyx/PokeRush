#ifndef __POKERUSH_MACROS_H__
#define __POKERUSH_MACROS_H__

/**
 * Las macros tienen abreviaciones para que sean
 * más fácil de escribir.
 * 
 * X = Columna
 * Y = Fila
 * P = Posicion (X, Y)
 * C = Color de texto (R, G, B)
 * B = Color de fondo (R, G, B)
 * M = Mensaje/Texto (string)
 * T = Tecla (int)
 * E = Estilo (negrita, subrayado, itálico)
 * S = Símbolo (char)
 * D = Duración (frames)
 * R = Retraso/Delay (frames)
*/

// Configuración de motor

#define ANCHO_PANTALLA 90
#define ALTO_PANTALLA 30

// General
#define C_TRANSPARENTE 0, 0, 0, 0.0f
#define X_MARGEN 10
#define Y_MARGEN 5
#define B_INICIAL 0, 0, 0
#define B_PRINCIPAL 10, 50, 60
#define OPACIDAD_FONDO 35 // de 100
#define D_TRANSICION_FONDO 150
#define B_CARRERA 100, 200, 100

// Pokemones (sprites)
#define X_POKEMON 32
#define Y_POKEMON 16

// Fila inferior de controles
#define Y_CONTROL 27
#define C_CONTROL 150, 150, 150

// Controles principales, a la izquierda
#define X_CONTROL_1 5
#define X_CONTROL_2 15
#define X_CONTROL_3 25
#define X_CONTROL_4 35
#define X_CONTROL_5 45
#define M_ARRIBA "[UP]"
#define M_ABAJO "[DOWN]"
#define M_ENTER "[ENTER]"
#define M_IZQUIERDA "[LEFT]"
#define M_DERECHA "[RIGHT]"
#define E_CONTROL false, false, false

// Control de retorno, a la derecha
#define X_SALIR 75
#define M_SALIR "[Q] Salir"
#define M_VOLVER "[Q] Volver"
#define T_SALIR 'Q'

// Título de la escena
#define P_TITULO 5, 2
#define C_TITULO 255, 255, 255
#define E_TITULO true, true, false

// Texto normal
#define C_NORMAL 220, 255, 220
#define E_NORMAL false, false, false

// Texto seleccionado
#define C_SELECCION 5, 25, 30
#define B_SELECCION 50, 200, 255

// Opciones principales dentro de la escena
#define E_OPCION true, false, false
#define S_OPCION '>'

// Nombre del entrenador
#define E_NOMBRE true, false, true
#define C_NOMBRE_1 200, 200, 100
#define C_NOMBRE_2 200, 100, 200

// Oponentes
#define M_FACIL "Brock"
#define M_MEDIO "Misty"
#define M_DIFICIL "Red"
#define M_IMPOSIBLE "Cynthia"

// Atributos
#define C_FUERZA 255, 127, 127
#define C_INTELIGENCIA 150, 150, 255
#define C_DESTREZA 255, 255, 127
#define M_FUERZA "FUERZA"
#define M_INTELIGENCIA "INTELIGENCIA"
#define M_DESTREZA "DESTREZA"
#define S_FUERZA 'F'
#define S_INTELIGENCIA 'I'
#define S_DESTREZA 'D'

// Versus
#define Y_NOMBRE_START 2
#define Y_NOMBRE_END 14
#define X_NOMBRE_1 20
#define X_NOMBRE_2 74
#define X_VERSUS 44
#define M_VERSUS "vs"

// Sprites
#define SPRITE_POKEMON_DEFAULT "pokemon_default"

#endif // __POKERUSH_MACROS_H__