#ifndef SIMTIMER_C
#define SIMTIMER_C

#include "simtimer.h"

// constants

const char RADIX_POINT = '.';

void runTimer(int milliSeconds)
{
	struct timeval startTime, endTime;
	int startSec, startUSec, endSec, endUSec;
	int uSecDiff, mSecDiff, secDiff, timeDiff;

	gettimeofday(&startTime, NULL);

	startSec = startTime.tv_sec;
	startUSec = startTime.tv_usec;

	timeDiff = 0;

	while (timeDiff < milliSeconds)
	{
		gettimeofday(&endTime, NULL);

		endSec = endTime.tv_sec;
		endUSec = endTime.tv_usec;
		uSecDiff = endUSec - startUSec;

		if (uSecDiff < 0)
		{
			uSecDiff = uSecDiff + 1000000;

			endSec = endSec - 1;
		}

		mSecDiff = uSecDiff / 1000;
		secDiff = (endSec - startSec) * 1000;
		timeDiff = secDiff + mSecDiff;
	}
}
double accessTimer(int controlCode, char *timeStr)
{
	static Boolean running = False;
	static int startSec = 0, endSec = 0, startUSec = 0, endUSec = 0;
	static int lapSec = 0, lapUSec = 0;
	struct timeval startData, lapData, endData;
	double fpTime = 0.0;

	switch (controlCode)
	{
	case ZERO_TIMER:
		gettimeofday(&startData, NULL);
		running = True;

		startSec = startData.tv_sec;
		startUSec = startData.tv_usec;

		fpTime = 0.000000000;
		lapSec = 0.000000000;
		lapUSec = 0.000000000;

		timeToString(lapSec, lapUSec, timeStr);
		break;
	case LAP_TIMER:
		if (running == True)
		{
			gettimeofday(&lapData, NULL);

			lapSec = lapData.tv_sec;
			lapUSec = lapData.tv_usec;

			fpTime = processTime(startSec, lapSec, startUSec, lapUSec, timeStr);
		}
		else
		{
			fpTime = 0.000000000;
		}
		break;
	case STOP_TIMER:
		if (running == True)
		{
			gettimeofday(&endData, NULL);
			running = False;

			endSec = endData.tv_sec;
			endUSec = endData.tv_usec;

			fpTime = processTime(startSec, endSec, startUSec, endUSec, timeStr);
		}

		// assume timer not running
		else
		{
			fpTime = 0.000000000;
		}
		break;
	}

	return fpTime;
}

double processTime(double startSec, double EndSec, double startUSec, double endUSec, char *timeStr)
{

	double secDiff = EndSec - startSec;
	double uSecDiff = endUSec - startUSec;
	double fpTime;

	fpTime = (double)secDiff + (double)uSecDiff / 1000000;

	if (uSecDiff < 0)
	{
		uSecDiff = uSecDiff + 1000000;
		secDiff = secDiff - 1;
	}
	timeToString(secDiff, uSecDiff, timeStr);

	return fpTime;
}

void timeToString(int secTime, int uSecTime, char *timeStr)
{
	int low, high, index = 0;
	char temp;

	while (uSecTime > 0)
	{
		timeStr[index] = (char)(uSecTime % 10 + '0');
		uSecTime /= 10;

		index++;
	}
	while (index < 6)
	{
		timeStr[index] = '0';
		index++;
	}
	timeStr[index] = RADIX_POINT;

	index++;

	if (secTime < 10) // single digit in display
	{
		timeStr[index] = (char)(secTime % 10 + '0');

		index++;

		timeStr[index] = SPACE;

		index++;

		secTime = 0;
	}

	while (secTime > 0)
	{
		timeStr[index] = (char)(secTime % 10 + '0');

		secTime /= 10;

		index++;
	}

	timeStr[index] = NULL_CHAR;

	low = 0;
	high = index - 1;

	while (low < high)
	{
		temp = timeStr[low];
		timeStr[low] = timeStr[high];
		timeStr[high] = temp;

		low++;
		high--;
	}
}

#endif // ifndef SIMTIMER_C
