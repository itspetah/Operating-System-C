#ifndef SIMTIMER_H
#define SIMTIMER_H

#include <sys/time.h>
#include <string.h>
#include <math.h>
#include "configops.h"

enum TIMER_CTRL_CODES
{
    ZERO_TIMER,
    LAP_TIMER,
    STOP_TIMER
};

extern const char RADIX_POINT;
extern const char SPACE;

void runTimer(int milliSeconds);
double accessTimer(int controlCode, char *timeStr);
double processTime(double startSec, double EndSec, double startUSec, double endUSec, char *timeStr);
void timeToString(int secTime, int uSecTime, char *timeStr);

#endif // ifndef SIMTIMER_H
