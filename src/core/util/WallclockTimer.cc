#include <iostream>
#include <string>
#include <sstream>

#include "WallclockTimer.hpp"

namespace fail {

WallclockTimer::WallclockTimer() {
	isRunning = false;
}

void WallclockTimer::startTimer() {
		isRunning = true;
		gettimeofday(&start, NULL);
}

std::string WallclockTimer::getRuntimeAsString() {
	
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

double WallclockTimer::getRuntimeAsDouble() {
	
	int length;
	long t1,t2, duration;
	double resultdouble
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
	resultdouble = atof( resultstring.str().c_str() );
	return resultdouble;
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
	current.tv_sec = 0;
	current.tv_usec = 0;
	end.tv_sec = 0;
	end.tv_usec = 0;
}

} // end-of-namespace: fail
