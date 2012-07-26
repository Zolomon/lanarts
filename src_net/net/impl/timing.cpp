#include "../timing.h"

void Timer::start() {
	time = SDL_GetTicks();
}
long Timer::get_microseconds() {
	Uint32 now = SDL_GetTicks();
	return (now - time)*1000;
}
