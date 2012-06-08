#include <sstream>
#include <fstream>
#include <time.h>

#include "Logger.hpp"

namespace fail {

void Logger::timestamp()
{
	(*m_pDest) << "[" << m_description;
	if (m_showTime) {
		time_t rawtime;
		struct tm* timeinfo;
		char buffer[80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer, 80, "%H:%M:%S", timeinfo);
		(*m_pDest) << " " << buffer;
	}
	(*m_pDest) << "] ";
}

} // end-of-namespace: fail
