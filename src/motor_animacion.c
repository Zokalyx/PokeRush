#include "motor_animacion.h"

float linear(uint64_t tiempo, uint64_t tiempo_comienzo, uint64_t tiempo_fin,
	     int inicio, int fin)
{
	if (tiempo < tiempo_comienzo)
		return (float)inicio;
	else if (tiempo > tiempo_fin)
		return (float)fin;

	float a = (float)inicio;
	float b = (float)fin;
	float t = (float)(tiempo - tiempo_comienzo) /
		  (float)(tiempo_fin - tiempo_comienzo);

	return a + (b - a) * t;
}

int ease_in_out(uint64_t tiempo, uint64_t tiempo_comienzo, uint64_t tiempo_fin,
		int inicio, int fin)
{
	if (tiempo < tiempo_comienzo)
		return inicio;
	else if (tiempo > tiempo_fin)
		return fin;

	float a = (float)inicio;
	float b = (float)fin;
	float t = (float)(tiempo - tiempo_comienzo) /
		  (float)(tiempo_fin - tiempo_comienzo);

	float p = t < 0.5 ? 2 * t * t : 1 - (-2 * t + 2) * (-2 * t + 2) / 2;

	return (int)(a + (b - a) * p);
}

int parpadeo(uint64_t tiempo, uint64_t periodo, int inicio, int fin)
{
	if ((tiempo / periodo) % 2 == 0)
		return inicio;
	else
		return fin;
}
