#include <iostream>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "WallclockTimer.hpp"

namespace fail {

void WallclockTimer::startTimer()
{
	m_IsRunning = true;
	gettimeofday(&m_Start, NULL);
}

std::string WallclockTimer::getRuntimeAsString() const
{
	std::stringstream result;
	result << getRuntimeAsDouble();

	return result.str().c_str();
}

double WallclockTimer::getRuntimeAsDouble() const
{
	double result;
	struct timeval current;

	if (m_IsRunning) {
		gettimeofday(&current, NULL);
		result = current.tv_sec - m_Start.tv_sec;
		result = result + (((double)current.tv_usec-m_Start.tv_usec)/1000000);
	} else {
		result = m_End.tv_sec - m_Start.tv_sec;
		result = result + (((double)m_End.tv_usec-m_Start.tv_usec)/1000000);
	}

	return result;
}

void WallclockTimer::stopTimer()
{
	if (m_IsRunning) {
		m_IsRunning = false;
		gettimeofday(&m_End, NULL);
	}
}

void WallclockTimer::reset()
{
	m_IsRunning = false;
	m_Start.tv_sec  = 0;
	m_Start.tv_usec = 0;
	m_End.tv_sec    = 0;
	m_End.tv_usec   = 0;
}

} // end-of-namespace: fail
