#include <t32.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>

using namespace std;

void err(int ernum){
	if(ernum != 0){
		cerr << "Error: " << ernum << endl;
		//exit(-1);
	}
}


int eval_command(){
	string cmd;
	char errmsg[128];
	cout << "%> ";
	getline(cin, cmd);
	if(cmd.compare("bye") == 0) return -1;

	int err = T32_Cmd((char*)cmd.c_str());
	if(err != 0){
		cerr << "Could not execute command: " << err << endl;
		word errnum;
		T32_GetMessage(errmsg, &errnum);
		cerr << errmsg << "Type: " << errnum << endl;
	}

	return 0;

}


int main(void){
	cout << "Lauterbach remote connection" << endl;
	cout << "Enter bye to exit." << endl;

	char hostname[] = "localhost";
	char packlen[] = "1024";
	char port[] = "20010";

	cout << "[T32] Connecting to " << hostname << ":" << port << " Packlen: " << packlen << endl;
	T32_Config("NODE=", hostname);
	T32_Config("PACKLEN=", packlen);
	T32_Config("PORT=", port);

	cout << "[T32] Init." << endl;
	err(T32_Init());

	cout << "[T32] Attach." << endl;
	err(T32_Attach(T32_DEV_ICD));

	while(eval_command() != -1);

	cout << "[T32] Exit." << endl;
	err(T32_Exit());
	cout << "Bye." << endl;

	return 0;
}

