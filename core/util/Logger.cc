// Author: Adrian Böckenkamp
// Date:   21.11.2011

#include <sstream>
#include <fstream>
#include <cassert>
#include <time.h>

#include "Logger.hpp"

using std::endl;

void Logger::add(const std::string& what, const std::string& descr)
{
	(*m_pDest) << "[" << descr;
	if(m_showTime)
	{
		time_t rawtime;
		struct tm* timeinfo;
		char buffer [80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer, 80, "%H:%M:%S", timeinfo);
		(*m_pDest) << " " << buffer;
	}
	(*m_pDest) << "] " << what;
}
