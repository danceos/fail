#include <iostream>
#include <string>
#include <sstream>

#include "WallclockTimer.hpp"

namespace fail {

WallclockTimer::WallclockTimer() {
	m_log.setDescription("WallclockTimer");
	m_log.showTime(false);
	isRunning = false;
}

void WallclockTimer::startTimer() {
	if (isRunning) {
		m_log << "WallclockTimer is already running." << std::endl;
	} else {
		isRunning = true;
		gettimeofday(&start, NULL);
		m_log << "WallclockTimer started." << std::endl;
	}
}

std::string WallclockTimer::getRuntime() {
	
	int length;
	long t1,t2, duration;
	std::stringstream lengthinfo, resultstring; 
	
	if (isRunning) {
		gettimeofday(&current, NULL);
		t1 = (start.tv_sec*1000000)+start.tv_usec;
		t2 = (current.tv_sec*1000000)+current.tv_usec;
	} else {
		t1 = (start.tv_sec*1000000)+start.tv_usec;
		t2 = (end.tv_sec*1000000)+end.tv_usec;
	}
	
	duration = t2-t1;
	lengthinfo << duration-((duration/1000000)*1000000);
	length = lengthinfo.str().length();
	resultstring << (int) duration/1000000 << ".";
	
	if (length < 6) {
		int i;
		for (i = 0 ; i< 6-length ; i++){
			resultstring << "0";
		}
	}
	
	resultstring << (int) duration-((duration/1000000)*1000000);
	return resultstring.str();
}

void WallclockTimer::stopTimer() {
	if (isRunning) {
		isRunning = false;
		gettimeofday(&end, NULL);
		m_log << "WallclockTimer stopped." << std::endl;
	} else {
		m_log << "WallclockTimer is already stopped." << std::endl;
	}
}

void WallclockTimer::reset() {
	isRunning = false;
	start.tv_sec = 0;
	start.tv_usec = 0;
	current.tv_sec = 0;
	current.tv_usec = 0;
	end.tv_sec = 0;
	end.tv_usec = 0;
	m_log << "WallclockTimer reseted." << std::endl;
}

} // end-of-namespace: fail
