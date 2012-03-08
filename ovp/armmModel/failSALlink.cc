#include <iostream>
#include "SAL/SimulatorController.hpp"

using namespace std;


void hello(unsigned int p){
	cout << "&sal::simulator: " << hex << p << endl;
	sal::SimulatorController * salp = reinterpret_cast<sal::SimulatorController * >(p);
	
}

extern "C" void failSALset(unsigned int pointer){
	hello(pointer);
}
