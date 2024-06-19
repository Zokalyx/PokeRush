#include "motor_tiempo.h"

#ifdef _WIN32
#include <windows.h>
// Windows no es muy preciso con su Sleep().
// Así que ponemos un factor totalmente empírico,
// que depende de la máquina del usuario.
#define COMPENSACION 8 / 10
#endif

#ifdef __unix__
#include <time.h>
struct timespec {
	time_t tv_sec; // Seconds - >= 0
	long tv_nsec; // Nanoseconds - [0, 999999999]
};
int nanosleep(const struct timespec *req, struct timespec *rem);
int clock_gettime(int clk_id, struct timespec *tp);
#endif

void sleep_ms(uint64_t milisegundos)
{
#ifdef _WIN32
	Sleep((DWORD)(milisegundos * COMPENSACION));
#endif

#ifdef __unix__
	struct timespec ts;
	ts.tv_sec = (time_t)(milisegundos / 1000);
	ts.tv_nsec = (long)((milisegundos % 1000) * 1000000);
	nanosleep(&ts, NULL);
#endif
}

uint64_t ms_actuales()
{
#ifdef _WIN32
	LARGE_INTEGER time, frequency;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&time);
	return (uint64_t)(time.QuadPart / (frequency.QuadPart / 1000));
#endif

#ifdef __unix__
	struct timespec ts;
	clock_gettime(0, &ts);
	return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif
}
