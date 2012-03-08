#include <iostream>
#include <fstream>
#include "controller/ExperimentData.hpp"
#include "controller/ExperimentDataQueue.hpp"
#include "jobserver/JobServer.hpp"
#include "FaultCoverageExperiment.pb.h"

using namespace std;

int main(int argc, char* argv[]){

	
	fi::ExperimentDataQueue exDaQu;
	fi::ExperimentData* readFromQueue;


	//Daten in Struktur schreiben und in Datei speichern
	
	ofstream fileWrite;
	fileWrite.open("test.txt");


	FaultCoverageExperimentData faultCovExWrite;

	//Namen setzen
	faultCovExWrite.set_data_name("Testfall 42");

	//Instruktionpointer 1
	faultCovExWrite.set_m_instrptr1(0x4711);
	

	//Instruktionpointer 2
	faultCovExWrite.set_m_instrptr2(0x1122);


	//In ExperimentData verpacken
	fi::ExperimentData exDaWrite(&faultCovExWrite);

	//In Queue einbinden
	exDaQu.addData(&exDaWrite);

	//Aus Queue holen
	if(exDaQu.size() != 0)
		readFromQueue = exDaQu.getData();

	//Serialisierung ueber Wrapper-Methode in ExperimentData
	readFromQueue->serialize(&fileWrite);

	//cout << "Ausgabe: " << out << endl;

	fileWrite.close(); 



//-------------------------------------------------------------------------------------------------




	//Daten aus Datei lesen und in Struktur schreiben
	

	ifstream fileRead;
	fileRead.open("test.txt");

	
	FaultCoverageExperimentData faultCovExRead;
	
	fi::ExperimentData exDaRead(&faultCovExRead);

	exDaRead.unserialize( &fileRead);


	//Wenn Name, dann ausgeben
	if(faultCovExRead.has_data_name()){
		cout << "Name: "<< faultCovExRead.data_name() << endl;
	}

	//m_instrptr1 augeben
	cout << "m_instrptr1: " << faultCovExRead.m_instrptr1() << endl;

	//m_instrptr2 augeben
	cout << "m_instrptr2: " << faultCovExRead.m_instrptr2() << endl;

	fileRead.close();


	return 0;
}
