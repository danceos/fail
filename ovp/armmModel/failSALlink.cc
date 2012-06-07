#include <iostream>
//#include "SAL/SimulatorController.hpp"

using namespace std;


void hello(unsigned int p){
	cout << "&fail::simulator: " << hex << p << endl;
//	fail::SimulatorController * salp = reinterpret_cast<fail::SimulatorController * >(p);
	
}

extern "C" void failSALset(unsigned int pointer){
	hello(pointer);
}
