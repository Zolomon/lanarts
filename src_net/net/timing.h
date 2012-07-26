/*
 * timing.h:
 *  Simple high resolution timing.
 */
#ifndef __TIMING_H_
#define __TIMING_H_
#include <SDL/SDL.h>
class Timer {
public:
	void start();
	long get_microseconds();
private:
	Uint32 time;
};

#endif
