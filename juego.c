#include <stdlib.h>
#include <stdio.h>
#include "src/motor/motor.h"
#include "src/motor/estado.h"
#include "src/pokerush/pokerush.h"

#define CSV_POKEMONES "./pokemones_juego.csv"
#define DIR_SPRITES "./sprites"

int main(int argc, char const *argv[])
{
	juego_t pokerush = pr_juego();
	struct pr_config config = (struct pr_config){
		.pokemones_csv = CSV_POKEMONES,
		.sprites_dir = DIR_SPRITES,
	};

	estado_t estado = motor_ejecutar_juego(&pokerush, &config);

	estado_mostrar(estado);

	return estado;
}
