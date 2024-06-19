#include "motor_animacion.h"

float linear(uint64_t tiempo_ms, uint64_t comienzo_ms, uint64_t fin_ms,
	     int inicio, int fin)
{
	if (tiempo_ms < comienzo_ms)
		return (float)inicio;
	else if (tiempo_ms > fin_ms)
		return (float)fin;

	float a = (float)inicio;
	float b = (float)fin;
	float t = (float)(tiempo_ms - comienzo_ms) /
		  (float)(fin_ms - comienzo_ms);

	return a + (b - a) * t;
}

int ease_in_out(uint64_t tiempo_ms, uint64_t comienzo_ms, uint64_t fin_ms,
		int inicio, int fin)
{
	if (tiempo_ms < comienzo_ms)
		return inicio;
	else if (tiempo_ms > fin_ms)
		return fin;

	float a = (float)inicio;
	float b = (float)fin;
	float t = (float)(tiempo_ms - comienzo_ms) /
		  (float)(fin_ms - comienzo_ms);

	float p = t < 0.5 ? 2 * t * t : 1 - (-2 * t + 2) * (-2 * t + 2) / 2;

	return (int)(a + (b - a) * p);
}

int parpadeo(uint64_t tiempo_ms, uint64_t periodo_ms, int inicio, int fin)
{
	if ((tiempo_ms / periodo_ms) % 2 == 0)
		return inicio;
	else
		return fin;
}
