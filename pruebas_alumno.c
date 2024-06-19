#include "pa2m.h"
#include "src/tp.h"
#include <stdlib.h>
#include <string.h>

#define CSV_PRUEBAS "./pokemones_pruebas.csv"
#define POKEMON_ORDENADOS \
	"Articuno,Blastoise,Bulbasaur,Caterpie,Charizard,Diglett,Dragonair,Dragonite,Dratini,Ekans,Geodude,Grimer,Ivysaur,Meowth,Moltres,Pidgey,Pikachu,Raichu,Rattata,Sandshrew,Squirtle,Venusaur,Vulpix,Wartortle,Zapdos"

void pruebas_null()
{
	unsigned cantidad_obstaculos =
		tp_agregar_obstaculo(NULL, JUGADOR_1, OBSTACULO_FUERZA, 0);
	pa2m_afirmar(cantidad_obstaculos == 0,
		     "No se pueden agregar obstáculos a un TP NULL");

	TP *tp = tp_crear(NULL);
	pa2m_afirmar(tp == NULL, "No se puede crear un TP con un archivo NULL");

	tp = tp_crear(CSV_PRUEBAS);

	const struct pokemon_info *pokemon = tp_buscar_pokemon(NULL, "Pikachu");
	pa2m_afirmar(pokemon == NULL,
		     "No se puede buscar un pokémon con un TP NULL");

	pokemon = tp_buscar_pokemon(tp, NULL);
	pa2m_afirmar(pokemon == NULL,
		     "No se puede buscar un pokémon con nombre NULL");

	unsigned tiempo = tp_calcular_tiempo_pista(NULL, JUGADOR_1);
	pa2m_afirmar(tiempo == 0,
		     "El tiempo de una pista es 0 para un TP NULL");

	int cantidad_pokemon = tp_cantidad_pokemon(NULL);
	pa2m_afirmar(cantidad_pokemon == 0,
		     "La cantidad de pokémon es 0 para un TP NULL");

	tp_destruir(NULL);
	pa2m_afirmar(true, "No ocurre nada al destruir un TP NULL");

	tp_limpiar_pista(NULL, JUGADOR_1);
	pa2m_afirmar(true,
		     "No ocurre nada al limpiar una pista con un TP NULL");

	char *nombres = tp_nombres_disponibles(NULL);
	pa2m_afirmar(nombres == NULL, "No se dan nombres para un TP NULL");

	char *obstaculos = tp_obstaculos_pista(NULL, JUGADOR_1);
	pa2m_afirmar(obstaculos == NULL,
		     "No se dan obstáculos para un TP NULL");

	pokemon = tp_pokemon_seleccionado(NULL, JUGADOR_1);
	pa2m_afirmar(
		pokemon == NULL,
		"Un jugador no tiene pokémon seleccionado si el TP es NULL");

	cantidad_obstaculos = tp_quitar_obstaculo(NULL, JUGADOR_1, 0);
	pa2m_afirmar(cantidad_obstaculos == 0,
		     "No se pueden quitar obstáculos de un TP NULL");

	bool seleccionado = tp_seleccionar_pokemon(NULL, JUGADOR_1, "Pikachu");
	pa2m_afirmar(!seleccionado,
		     "No se puede seleccionar pokémon en un TP NULL");

	seleccionado = tp_seleccionar_pokemon(tp, JUGADOR_1, NULL);
	pa2m_afirmar(!seleccionado,
		     "No se puede seleccionar un pokémon con nombre NULL");

	char *tiempos = tp_tiempo_por_obstaculo(NULL, JUGADOR_1);
	pa2m_afirmar(
		tiempos == NULL,
		"No se pueden obtener los tiempos de obstáculo con un TP NULL");

	tp_destruir(tp);
}

void pruebas_invalid()
{
	TP *tp = tp_crear("Archivo Inexistente");
	pa2m_afirmar(tp == NULL,
		     "No se puede crear un TP en base a un .csv inexistente");

	tp = tp_crear("README.md");
	pa2m_afirmar(
		tp == NULL,
		"No se puede crear un TP en base a un archivo sin el formato especificado");

	tp = tp_crear("pokemones_malformado1.csv");
	pa2m_afirmar(tp == NULL,
		     "No se puede crear un TP con un .csv inválido (1)");

	tp = tp_crear("pokemones_malformado2.csv");
	pa2m_afirmar(tp == NULL,
		     "No se puede crear un TP con un .csv inválido (2)");

	tp = tp_crear(CSV_PRUEBAS);
	const struct pokemon_info *pokemon = tp_buscar_pokemon(tp, "Creeper");
	pa2m_afirmar(pokemon == NULL,
		     "Buscar un pokémon inexistente devuelve NULL");

	bool seleccionado = tp_seleccionar_pokemon(tp, JUGADOR_1, "Creeper");
	pa2m_afirmar(!seleccionado,
		     "No se puede seleccionar un pokémon inexistente");

	tp_agregar_obstaculo(tp, JUGADOR_1, OBSTACULO_DESTREZA, 0);
	unsigned tiempo = tp_calcular_tiempo_pista(tp, JUGADOR_1);
	char *tiempos = tp_tiempo_por_obstaculo(tp, JUGADOR_1);
	pa2m_afirmar(
		tiempo == 0 && tiempos == NULL,
		"No se pueden calcular tiempos si el jugador no tiene seleccionado un pokémon");

	unsigned cantidad_obstaculos = tp_quitar_obstaculo(tp, JUGADOR_2, 0);
	pa2m_afirmar(cantidad_obstaculos == 0,
		     "No se puede quitar un obstáculo de una pista vacía");

	tp_seleccionar_pokemon(tp, JUGADOR_1, "Pikachu");

	seleccionado = tp_seleccionar_pokemon(tp, JUGADOR_2, "Pikachu");
	pa2m_afirmar(
		!seleccionado,
		"No se puede seleccionar el mismo pokémon que el oponente");

	tp_quitar_obstaculo(tp, JUGADOR_1, 0);

	tiempo = tp_calcular_tiempo_pista(tp, JUGADOR_1);
	pa2m_afirmar(
		tiempo == 0,
		"El tiempo para una pista vacía (con pokémon seleccionado) es 0");

	tiempos = tp_tiempo_por_obstaculo(tp, JUGADOR_1);
	pa2m_afirmar(
		tiempos == NULL,
		"Los tiempos por obstáculo son NULL para una pista vacía (con pokémon seleccionado)");

	tp_limpiar_pista(tp, JUGADOR_1);
	pa2m_afirmar(
		true,
		"El programa no explota cuando se quiere limpiar una pista vacía");

	tp_agregar_obstaculo(tp, JUGADOR_2, OBSTACULO_DESTREZA, 0);
	tiempo = tp_calcular_tiempo_pista(tp, JUGADOR_2);
	pa2m_afirmar(
		tiempo == 0,
		"El tiempo para un jugador sin pokémon (con obstáculos) es 0");

	tiempos = tp_tiempo_por_obstaculo(tp, JUGADOR_2);
	pa2m_afirmar(
		tiempos == NULL,
		"Los tiempo por obstáculo son NULL para un jugador sin pokémon (con obstáculos)");

	tp_destruir(tp);
}

void pruebas_pokemon()
{
	TP *tp = tp_crear(CSV_PRUEBAS);

	const struct pokemon_info *pokemon = tp_buscar_pokemon(tp, "PiKaChU");
	pa2m_afirmar(
		pokemon != NULL,
		"Se puede buscar un pokémon, incluso escribiendo el nombre con mayús y minús distintas");

	int cantidad_pokemon = tp_cantidad_pokemon(tp);
	pa2m_afirmar(cantidad_pokemon == 25,
		     "La cantidad de pokémon coincide con el .csv");

	pokemon = tp_buscar_pokemon(tp, "Pikachu");
	pa2m_afirmar(pokemon != NULL,
		     "El primer pokémon del .csv está presente");

	pokemon = tp_buscar_pokemon(tp, "Venusaur");
	pa2m_afirmar(pokemon != NULL,
		     "Un pokémon del medio del .csv está presente");

	pokemon = tp_buscar_pokemon(tp, "Dragonite");
	pa2m_afirmar(pokemon != NULL,
		     "El último pokémon del .csv está presente");

	char *nombres = tp_nombres_disponibles(tp);
	pa2m_afirmar(
		strcmp(nombres, POKEMON_ORDENADOS) == 0,
		"Los nombres devueltos están en el formato correcto y están en orden alfabético");

	free(nombres);

	pokemon = tp_buscar_pokemon(tp, "Caterpie");
	pa2m_afirmar(strcmp(pokemon->nombre, "Caterpie") == 0 &&
			     pokemon->fuerza == 3 && pokemon->destreza == 8 &&
			     pokemon->inteligencia == 1,
		     "Los datos de los pokémon se cargan correctamente");

	tp_destruir(tp);
}

void pruebas_obstaculo_jugador1()
{
	TP *tp = tp_crear(CSV_PRUEBAS);

	unsigned cantidad_obstaculos =
		tp_agregar_obstaculo(tp, JUGADOR_1, OBSTACULO_DESTREZA, 10);
	pa2m_afirmar(
		cantidad_obstaculos == 1,
		"Se puede agregar un obstáculo, aún cuando se especifica una posición fuera de rango");

	cantidad_obstaculos =
		tp_agregar_obstaculo(tp, JUGADOR_1, OBSTACULO_FUERZA, 10);
	pa2m_afirmar(
		cantidad_obstaculos == 2,
		"Se puede agregar otro obstáculo y la cantidad se actualiza correctamente");

	cantidad_obstaculos =
		tp_agregar_obstaculo(tp, JUGADOR_1, OBSTACULO_INTELIGENCIA, 1);
	pa2m_afirmar(cantidad_obstaculos == 3,
		     "Se puede agregar un obstáculo entre otros 2");

	char *obstaculos = tp_obstaculos_pista(tp, JUGADOR_1);
	pa2m_afirmar(strcmp(obstaculos, "DIF") == 0,
		     "La lista de obstáculos se genera correctamente");

	free(obstaculos);

	cantidad_obstaculos = tp_quitar_obstaculo(tp, JUGADOR_1, 1);
	pa2m_afirmar(
		cantidad_obstaculos == 2,
		"Se puede eliminar un obstáculo en una posición arbitraria");

	cantidad_obstaculos = tp_quitar_obstaculo(tp, JUGADOR_1, 10);
	pa2m_afirmar(
		cantidad_obstaculos == 1,
		"Se puede quitar el último obstáculo pasando una posición fuera de rango");

	obstaculos = tp_obstaculos_pista(tp, JUGADOR_1);
	pa2m_afirmar(strcmp(obstaculos, "D") == 0,
		     "Queda el obstáculo correcto");
	free(obstaculos);

	tp_agregar_obstaculo(tp, JUGADOR_1, OBSTACULO_INTELIGENCIA, 0);
	tp_limpiar_pista(tp, JUGADOR_1);
	cantidad_obstaculos =
		tp_agregar_obstaculo(tp, JUGADOR_1, OBSTACULO_DESTREZA, 0);
	pa2m_afirmar(cantidad_obstaculos == 1, "Se puede limpiar la pista");

	tp_destruir(tp);
}

void pruebas_obstaculo_jugador2()
{
	TP *tp = tp_crear(CSV_PRUEBAS);

	unsigned cantidad_obstaculos =
		tp_agregar_obstaculo(tp, JUGADOR_2, OBSTACULO_DESTREZA, 10);
	pa2m_afirmar(
		cantidad_obstaculos == 1,
		"Se puede agregar un obstáculo, aún cuando se especifica una posición fuera de rango");

	cantidad_obstaculos =
		tp_agregar_obstaculo(tp, JUGADOR_2, OBSTACULO_FUERZA, 10);
	pa2m_afirmar(
		cantidad_obstaculos == 2,
		"Se puede agregar otro obstáculo y la cantidad se actualiza correctamente");

	cantidad_obstaculos =
		tp_agregar_obstaculo(tp, JUGADOR_2, OBSTACULO_INTELIGENCIA, 1);
	pa2m_afirmar(cantidad_obstaculos == 3,
		     "Se puede agregar un obstáculo entre otros 2");

	char *obstaculos = tp_obstaculos_pista(tp, JUGADOR_2);
	pa2m_afirmar(strcmp(obstaculos, "DIF") == 0,
		     "La lista de obstáculos se genera correctamente");

	free(obstaculos);

	cantidad_obstaculos = tp_quitar_obstaculo(tp, JUGADOR_2, 1);
	pa2m_afirmar(
		cantidad_obstaculos == 2,
		"Se puede eliminar un obstáculo en una posición arbitraria");

	cantidad_obstaculos = tp_quitar_obstaculo(tp, JUGADOR_2, 10);
	pa2m_afirmar(
		cantidad_obstaculos == 1,
		"Se puede quitar el último obstáculo pasando una posición fuera de rango");

	obstaculos = tp_obstaculos_pista(tp, JUGADOR_2);
	pa2m_afirmar(strcmp(obstaculos, "D") == 0,
		     "Queda el obstáculo correcto");
	free(obstaculos);

	tp_agregar_obstaculo(tp, JUGADOR_2, OBSTACULO_INTELIGENCIA, 0);
	tp_limpiar_pista(tp, JUGADOR_2);
	cantidad_obstaculos =
		tp_agregar_obstaculo(tp, JUGADOR_2, OBSTACULO_DESTREZA, 0);
	pa2m_afirmar(cantidad_obstaculos == 1, "Se puede limpiar la pista");

	tp_destruir(tp);
}

void pruebas_carrera_jugador1()
{
	TP *tp = tp_crear(CSV_PRUEBAS);

	tp_seleccionar_pokemon(tp, JUGADOR_1, "Pikachu");
	tp_agregar_obstaculo(tp, JUGADOR_1, OBSTACULO_DESTREZA, 0);

	unsigned tiempo = tp_calcular_tiempo_pista(tp, JUGADOR_1);
	pa2m_afirmar(tiempo == 1,
		     "El tiempo para un obstáculo se calcula correctamente");

	char *tiempos = tp_tiempo_por_obstaculo(tp, JUGADOR_1);
	pa2m_afirmar(strcmp(tiempos, "1") == 0,
		     "El string de tiempos también es correcto");
	free(tiempos);

	tp_agregar_obstaculo(tp, JUGADOR_1, OBSTACULO_DESTREZA, 0);
	tp_agregar_obstaculo(tp, JUGADOR_1, OBSTACULO_DESTREZA, 0);

	tiempo = tp_calcular_tiempo_pista(tp, JUGADOR_1);
	pa2m_afirmar(
		tiempo == 1,
		"El tiempo para dos obstáculos iguales se reduce en 1 cada vez");

	tiempos = tp_tiempo_por_obstaculo(tp, JUGADOR_1);
	pa2m_afirmar(strcmp(tiempos, "1,0,0") == 0,
		     "El tiempo no puede ser menor a 0");
	free(tiempos);

	tp_agregar_obstaculo(tp, JUGADOR_1, OBSTACULO_FUERZA, 0);
	tp_agregar_obstaculo(tp, JUGADOR_1, OBSTACULO_INTELIGENCIA, 10);

	tiempo = tp_calcular_tiempo_pista(tp, JUGADOR_1);
	pa2m_afirmar(
		tiempo == 1 + 0 + 2,
		"El tiempo para obstáculos de todo tipo se calcula correctamente");

	tiempos = tp_tiempo_por_obstaculo(tp, JUGADOR_1);
	pa2m_afirmar(strcmp(tiempos, "0,1,0,0,2") == 0,
		     "El string correspondiente también es correcto");
	free(tiempos);

	tp_seleccionar_pokemon(tp, JUGADOR_1, "Squirtle");
	tiempos = tp_tiempo_por_obstaculo(tp, JUGADOR_1);
	pa2m_afirmar(
		strcmp(tiempos, "6,7,6,5,5") == 0,
		"Si se cambia de pokémon los tiempos también son correctos");
	free(tiempos);

	tp_destruir(tp);
}

void pruebas_carrera_jugador2()
{
	TP *tp = tp_crear(CSV_PRUEBAS);

	tp_seleccionar_pokemon(tp, JUGADOR_2, "Pikachu");
	tp_agregar_obstaculo(tp, JUGADOR_2, OBSTACULO_DESTREZA, 0);

	unsigned tiempo = tp_calcular_tiempo_pista(tp, JUGADOR_2);
	pa2m_afirmar(tiempo == 1,
		     "El tiempo para un obstáculo se calcula correctamente");

	char *tiempos = tp_tiempo_por_obstaculo(tp, JUGADOR_2);
	pa2m_afirmar(strcmp(tiempos, "1") == 0,
		     "El string de tiempos también es correcto");
	free(tiempos);

	tp_agregar_obstaculo(tp, JUGADOR_2, OBSTACULO_DESTREZA, 0);
	tp_agregar_obstaculo(tp, JUGADOR_2, OBSTACULO_DESTREZA, 0);

	tiempo = tp_calcular_tiempo_pista(tp, JUGADOR_2);
	pa2m_afirmar(
		tiempo == 1,
		"El tiempo para dos obstáculos iguales se reduce en 1 cada vez");

	tiempos = tp_tiempo_por_obstaculo(tp, JUGADOR_2);
	pa2m_afirmar(strcmp(tiempos, "1,0,0") == 0,
		     "El tiempo no puede ser menor a 0");
	free(tiempos);

	tp_agregar_obstaculo(tp, JUGADOR_2, OBSTACULO_FUERZA, 0);
	tp_agregar_obstaculo(tp, JUGADOR_2, OBSTACULO_INTELIGENCIA, 10);

	tiempo = tp_calcular_tiempo_pista(tp, JUGADOR_2);
	pa2m_afirmar(
		tiempo == 1 + 0 + 2,
		"El tiempo para obstáculos de todo tipo se calcula correctamente");

	tiempos = tp_tiempo_por_obstaculo(tp, JUGADOR_2);
	pa2m_afirmar(strcmp(tiempos, "0,1,0,0,2") == 0,
		     "El string correspondiente también es correcto");
	free(tiempos);

	tp_seleccionar_pokemon(tp, JUGADOR_2, "Squirtle");
	tiempos = tp_tiempo_por_obstaculo(tp, JUGADOR_2);
	pa2m_afirmar(
		strcmp(tiempos, "6,7,6,5,5") == 0,
		"Si se cambia de pokémon los tiempos también son correctos");
	free(tiempos);

	tp_destruir(tp);
}

int main()
{
	pa2m_nuevo_grupo("\nARGUMENTOS NULL");
	pruebas_null();

	pa2m_nuevo_grupo("\nARGUMENTOS Y ESTADOS INVÁLIDOS");
	pruebas_invalid();

	pa2m_nuevo_grupo("\nPOKÉMONES");
	pruebas_pokemon();

	pa2m_nuevo_grupo("\nOBSTÁCULOS JUGADOR 1");
	pruebas_obstaculo_jugador1();

	pa2m_nuevo_grupo("\nOBSTÁCULOS JUGADOR 2");
	pruebas_obstaculo_jugador2();

	pa2m_nuevo_grupo("\nCARRERAS JUGADOR 1");
	pruebas_carrera_jugador1();

	pa2m_nuevo_grupo("\nCARRERAS JUGADOR 2");
	pruebas_carrera_jugador2();

	return pa2m_mostrar_reporte();
}
