#include <iostream>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "WallclockTimer.hpp"

namespace fail {

WallclockTimer::WallclockTimer() {
	isRunning = false;
}

void WallclockTimer::startTimer() {
		isRunning = true;
		gettimeofday(&start, NULL);
}

std::string WallclockTimer::getRuntimeAsString() const {
	
	std::stringstream result;
	result << getRuntimeAsDouble();
	
	return result.str().c_str();
}

double WallclockTimer::getRuntimeAsDouble() const {
	
	double result;
	struct timeval current;
	
	if (isRunning) {
		gettimeofday(&current, NULL);
		result = current.tv_sec - start.tv_sec;
		result = result + (((double)current.tv_usec-start.tv_usec)/1000000);
	} else {
		result = end.tv_sec - start.tv_sec;
		result = result + (((double)end.tv_usec-start.tv_usec)/1000000);
	}
	
	return result;
}



void WallclockTimer::stopTimer() {
	if (isRunning) {
		isRunning = false;
		gettimeofday(&end, NULL);
	} 
}

void WallclockTimer::reset() {
	isRunning = false;
	start.tv_sec = 0;
	start.tv_usec = 0;
	end.tv_sec = 0;
	end.tv_usec = 0;
}


std::ostream& operator<< (std::ostream& os, const WallclockTimer& w) {
  os << w.getRuntimeAsString();
  return os;
}

} // end-of-namespace: fail
