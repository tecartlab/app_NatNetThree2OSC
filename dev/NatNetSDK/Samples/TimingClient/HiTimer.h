/* 
Copyright © 2014 NaturalPoint Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#pragma once

#include <windows.h>

#define HITIMER_MAX_SAMPLES 100			// size of averaging queue
#define HITIMER_UNDEFINED   -99999.0f	// undefined

// HiTimer : A high performance timer class for calculating individual
// and average elapsed times using the windows performance timer.
//
// Usage :
//   HiTimer timer;
//   ...
//   timer.Start()
//   ...
//   timer.Stop()
//   double dElapsed = timer.Duration();
//   timer.AddSample(dElapsed);
//   double dAverageTime = timer.Average();
class HiTimer
{
public:

	HiTimer()
	{
		startTime.QuadPart = 0;
		stopTime.QuadPart = 0;
		mIndex = 0;
		if(QueryPerformanceFrequency(&freq) ==0)
		{
			// high perf not supported
			//throw new Win32Exception();
		}
		for(int i=0; i<HITIMER_MAX_SAMPLES; i++)
			m_SamplesQueue[i] = HITIMER_UNDEFINED;
	}

	void Start()
	{
		QueryPerformanceCounter(&startTime);
	}

	void Stop()
	{
		QueryPerformanceCounter(&stopTime);
	}

	// get elapsed duration (in seconds)
	double Duration()
	{
		return (double)(stopTime.QuadPart - startTime.QuadPart) / (double) freq.QuadPart;
	}

	// add sample (used for average calc)
	void AddSample(double Sample)
	{
		m_SamplesQueue[mIndex] = Sample;
		mIndex++;
		mIndex %= HITIMER_MAX_SAMPLES;	
	}

	// calculate average of stored samples
	double Average() 
	{ 
		double dAverage = 0.0f;
		double dSum = 0.0f;
		int nValidSamples = 0;
		for(int i=0; i<HITIMER_MAX_SAMPLES; i++)
		{
			if(m_SamplesQueue[i] == HITIMER_UNDEFINED)
				break;
			dSum += m_SamplesQueue[i];
			nValidSamples++;
		}
		
		if(nValidSamples > 0)
			dAverage = dSum / (double)nValidSamples;

		return dAverage;
	}

	// reset timer and clear out samples queue
	void Reset()
	{
		for(int i=0; i<HITIMER_MAX_SAMPLES; i++)
			m_SamplesQueue[i] = HITIMER_UNDEFINED;
		startTime.QuadPart = 0;
		stopTime.QuadPart = 0;
		mIndex = 0;
	}

private:
	LARGE_INTEGER startTime;
	LARGE_INTEGER stopTime;
	LARGE_INTEGER freq;
	double m_SamplesQueue[HITIMER_MAX_SAMPLES];
	int mIndex;

};
