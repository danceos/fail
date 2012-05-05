/*
 * Copyright (c) 2005-2011 Imperas Software Ltd., www.imperas.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied.
 *
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

#include "platform.hpp"
#include "statreg.hpp"
#include "SAL/SALInst.hpp"

#include "icm/icmCpuManager.h"
#include "icm/icmQuery.h"

// enable relaxed scheduling for maximum performance
//#define SIM_ATTRS (ICM_ATTR_RELAXED_SCHED|ICM_ATTR_TRACE)
#define SIM_ATTRS (ICM_ATTR_RELAXED_SCHED)

// GDB port
#define GDB_PORT 1234

// Variables set by arguments
const char  *variant     = "Cortex-M3";   // the model variant

using namespace std;

ARM_Cortex_M3 arm;

ARM_Cortex_M3::ARM_Cortex_M3() {
}

ARM_Cortex_M3::~ARM_Cortex_M3() {
	delete platform;
	delete cpu;
	delete attrList;
}

void ARM_Cortex_M3::init(bool gdb=false) {

	// initialize OVPsim, enabling verbose mode to get statistics at end of execution
	platform = new icmPlatform("CortexM3Platform", //name
					ICM_VERBOSE|ICM_STOP_ON_CTRLC, //attributes
					gdb ? "rsp" : 0, 	// optional protocol
					gdb ? GDB_PORT : 0);	// optional port number


//	const char *armModel    = icmGetVlnvString(NULL, "arm.ovpworld.org", "processor", "armm", "1.0", "model");
	const char *armModel    = "/srv/scratch/hoffmann/test/build/lib/libarmmModel.so";
	const char *armSemihost = icmGetVlnvString(NULL, "arm.ovpworld.org", "semihosting", "armNewlib", "1.0", "model");

	attrList = new icmAttrListObject();

	attrList->addAttr("endian", "little");
	attrList->addAttr("compatibility", "nopBKPT");
	attrList->addAttr("variant", variant);
	attrList->addAttr("UAL", "1");
	attrList->addAttr("fail_salp", &sal::simulator);

	char cpuname[64];
	sprintf(cpuname,"cpu-%s", variant);

	// create a processor instance
	cpu = new icmProcessorObject(
			cpuname,	// CPU name
			"armm",		// CPU type
			0,		// CPU cpuId
			0,		// CPU model flags
			32,		// address bits
			armModel,	// model file
			"modelAttrs",	// morpher attributes
			SIM_ATTRS,	// attributes
			attrList,	// user-defined attributes
			armSemihost,	// semi-hosting file
			"modelAttrs"	// semi-hosting attributes
	);

	// Nominate this processor to be attached to a debugger
	if(gdb) cpu->debugThisProcessor();

	processorP = cpu->getProcessorP();
}

void ARM_Cortex_M3::createFullMMC(const char *vlnRoot=0) {
// TODO: ELF Groesse auslesen per OVP (hat funktionen) und dann den Bereich als MMC einrichten...
/*	const char *mmc_model = icmGetVlnvString("/srv/scratch/sirozipp/build/lib", "ovpworld.org", "mmc", "failMMC", "1.0", "model");
	if(mmc_model == NULL) {
		std::cerr << "mmc_model not found!" << std::endl;
		exit(1);
	}
*/
	const char *mmc_model = "/srv/scratch/sirozipp/build/lib/libflaky.so";

	// create full MMCs
	mmcInstr = new icmMmcObject("mmci", mmc_model, "modelAttrs", 0,  False);
	mmcData = new icmMmcObject("mmcd", mmc_model, "modelAttrs", 0,  False);

	// create processor instruction and data bus
	instrBus = new icmBusObject("instrbus", 32);
	dataBus = new icmBusObject("databus", 32);

	// create processor main bus
	mainBus = new icmBusObject("mainbus", 32);

	// connect processor ports to their buses
	cpu->connect(*instrBus, *dataBus);

	// connect mmcs to buses
	mmcInstr->connect(*instrBus, "sp1", False);
	mmcData->connect(*dataBus, "sp1", False);

	// connet master ports of mmc to main bus
	mmcInstr->connect(*mainBus, "mp1", True);
	mmcData->connect(*mainBus, "mp1", True);

	// create simulated memory
	Addr appHighAddr, appLowAddr;

	appHighAddr = 0xFFFFFFFF;
	appLowAddr = 0x0;
	icmMem0 = new icmMemoryObject("mem1", ICM_PRIV_RWX, appHighAddr-appLowAddr);

	// connect memory to main bus
	icmMem0->connect("mp1", *mainBus, 0);
}

static ICM_MEM_READ_FN(extMemRead) {
	unsigned char res[4];
//	Int32 tmpres = *(Int32*)value;
//	icmPrintf("EXTERNAL MEMORY: Reading 0x%08x from 0x%08x\n", *(Int32*)value, (Int32)address);
//	*(Int32*) value = (Int32)arm.mem[(address-arm.offset)>>2];

	sal::simulator.onMemoryAccessEvent(address, 4, false, ovpplatform.getPC());

	res[0] = arm.mem[(address-arm.offset)+0];
	res[1] = arm.mem[(address-arm.offset)+1];
	res[2] = arm.mem[(address-arm.offset)+2];
	res[3] = arm.mem[(address-arm.offset)+3];

//	icmPrintf("Reading [0] 0x%08x, [1] 0x%08x, [2] 0x%08x, [3] 0x%08x\n", arm.mem[(address-arm.offset)+0], arm.mem[(address-arm.offset)+1], arm.mem[(address-arm.offset)+2], arm.mem[(address-arm.offset)+3]);

	*(Int32*)value = (res[3] << 24) | (res[2] << 16) | (res[1] << 8) | res[0];
//	
//	icmPrintf("Reading: 0x%08x\n", (res[3] << 24) | (res[2] << 16) | (res[1] << 8) | res[0]);
	
//	icmPrintf("Callback: Reading 0x%08x from mem[0x%08x] should be 0x%08x from [0x%08x]\n", *(Int32*)value, (Int32)(address-arm.offset), tmpres, (Int32) address);
	
}

static ICM_MEM_WRITE_FN(extMemWrite) {
//	icmPrintf("EXTERNAL MEMORY: Writing 0x%08x to 0x%08x\n", (Int32)value, (Int32)address);
//	icmPrintf("Callback: Writing 0x%08x to mem[0x%08x] should be 0x%08x from [0x%08x]\n", *(Int32*)value, (Int32)(address-arm.offset), *(Int32*) value, (Int32) address);
	sal::simulator.onMemoryAccessEvent(address, 4, true, ovpplatform.getPC());

	Int32 val = *(Int32*)value;

	arm.mem[(address-arm.offset) + 0] = val & 0xFF;
	arm.mem[(address-arm.offset) + 1] = (val >> 8) & 0xFF;
	arm.mem[(address-arm.offset) + 2] = (val >> 16) & 0xFF;
	arm.mem[(address-arm.offset) + 3] = (val >> 24) & 0xFF;

//	icmPrintf("Writing [0] 0x%08x, [1] 0x%08x, [2] 0x%08x, [3] 0x%08x\n", val & 0xFF, (val >> 8) & 0xFF, (val >> 16) & 0xFF, (val >> 24) & 0xFF);
//	icmPrintf("Writing [0] 0x%08x, [1] 0x%08x, [2] 0x%08x, [3] 0x%08x\n", arm.mem[(address-arm.offset)+0], arm.mem[(address-arm.offset)+1], arm.mem[(address-arm.offset)+2], arm.mem[(address-arm.offset)+3]);

//	Int32 res = arm.mem[(address-arm.offset)+0] | arm.mem[(address-arm.offset)+1] << 8 | arm.mem[(address-arm.offset)+2]  << 16 | arm.mem[(address-arm.offset)+3] << 24;

//	icmPrintf("Callback: Writing 0x%08x to mem[0x%08x] should be 0x%08x to [0x%08x]\n", res, (Int32)(address-arm.offset), *(Int32*) value, (Int32) address);
//	arm.mem[(address-arm.offset) >> 2] = *(Int32*) value;
}

static ICM_MEM_READ_FN(extTextRead) {
//	Int16 val = *(Int16*)value;
	*(Int16*) value = (Int16)arm.textmem[(address-arm.textOffset)>>1];
//	icmPrintf("Callback: Reading 0x%04x from textmem[0x%08x] should be 0x%04x from [0x%08x]\n", *(Int16*)value, (Int32)(address-arm.textOffset), val, (Int32) address);
}

static ICM_MEM_WRITE_FN(extTextWrite) {
	arm.textmem[(address-arm.textOffset) >> 1] = *(Int16*) value;
}

void ARM_Cortex_M3::mapMemToCallback() {
	// create processor bus
	mainBus = new icmBusObject("mainbus", 32);
	
	// connect the processor bus
	cpu->connect(*mainBus, *mainBus);

	// create simulated memory
	Addr appHighAddr, appLowAddr, textLowAddr, textHighAddr, LowAddr, LowAddrH, HighAddr, HighAddrL;

	appLowAddr = offset;
	appHighAddr = appLowAddr + memSize - 1;

	textLowAddr = textOffset;
	textHighAddr = textLowAddr + textMemSize - 1;

	// check which section is first
	if(appLowAddr < textLowAddr) {
		LowAddr = appLowAddr;
		LowAddrH = appHighAddr;
		HighAddr = textHighAddr;
		HighAddrL = textLowAddr;
	} else {
		LowAddr = textLowAddr;
		LowAddrH = textHighAddr;
		HighAddr = appHighAddr;
		HighAddrL = appLowAddr;
	}

	// Memory from 0x0 to LowAddr
	icmMem0 = new icmMemoryObject("mem0", ICM_PRIV_RWX, LowAddr - 1); 
	// Memory after callback mem to end
	icmMem1 = new icmMemoryObject("mem1", ICM_PRIV_RWX, 0xFFFFFFFF - HighAddr - 1);
	// Memory between text and other sections
	icmMem2 = new icmMemoryObject("mem2", ICM_PRIV_RWX, HighAddrL - LowAddrH - 1 -1);

	// map the adress range appLowAddr:appHighAddr externally to the processor
	mainBus->mapExternalMemory("external", ICM_PRIV_RWX, appLowAddr, appHighAddr, 
					extMemRead,	// read callback
					extMemWrite,	// write callback
					0);

	mainBus->mapExternalMemory("exttext", ICM_PRIV_RWX, textLowAddr, textHighAddr,
					extTextRead,
					extTextWrite,
					0);
	
	// just build memory before external memory if it's not at the beginning
	if(LowAddr > 0x0) {
		icmMem0->connect("mp1", *mainBus, 0x0);
	}

	if(HighAddr < 0xffffffff) {
		icmMem1->connect("mp2", *mainBus, HighAddr+1);
	}

	if(LowAddrH != HighAddrL) {
		icmMem2->connect("mp3", *mainBus, LowAddrH+1);
	}

	mainBus->printConnections();
}


void ARM_Cortex_M3::makeGPRegister() {
	const string names[] = {"r0", "r1", "r2", "r3", "r4", "r5",
				"r6", "r7", "r8", "r9", "r10", "r11", "r12"};

	

	for(int i = 0; i <= 12; ++i) {
		icmRegInfoP reg = icmGetRegByIndex(processorP, i);
		sal::simulator.makeGPRegister(32, (void *)reg, names[i]);
	}

	// set SP pointer
	// ARM Cortex M3: SP pointer is ID 13
	icmRegInfoP sp_reg = icmGetRegByIndex(processorP, 13);
	setSPReg(sp_reg);
}

void ARM_Cortex_M3::makeSTRegister() {
	OVPStatusRegister *streg = new CortexM3StatusRegister();

	sal::simulator.makeSTRegister(streg, "sp");
}

void ARM_Cortex_M3::makePCRegister() {
	// ARM Cortex M3: PC pointer is ID 15
	icmRegInfoP pc_reg = icmGetRegByIndex(processorP, 15);
	sal::simulator.makePCRegister(32, (void *)pc_reg, "PC");
}

int ARM_Cortex_M3::startSimulation(const char *app) {
	bool loadPhysical = true;
	bool verbose = true;
	bool useEntry = true;
	if(!cpu->loadLocalMemory(app, loadPhysical, verbose, useEntry))
		return -1;
	
	// application is loaded, now fill the callback memory with program data
	fillCallbackMemory();

//	icmSimulatePlatform();

	while(1) {
		// save PC
		Addr pc_ptr = cpu->getPC();

		sal::simulator.onInstrPtrChanged(pc_ptr);

		// simulate the platform
		icmStopReason stopreason = cpu->simulate(1);

		if(stopreason!=0x00) {
		// was simulation interrupted or did it complete
			if(stopreason==ICM_SR_INTERRUPT) {
				icmPrintf("*** simulation interrupted\n");
			}

			break;
		}
	}
	//icmTerminate();

	return 0;
}

void ARM_Cortex_M3::makeCallbackMemory(size_t sizeText, size_t offText, size_t sizeMem, size_t offMem) {
	mem = new unsigned char[sizeMem];
	//mem = new Int32[sizeMem>>2];
	textmem = new Int16[sizeText>>1];
	offset = offMem;
	textOffset = offText;
	memSize = sizeMem;
	textMemSize = sizeText;
}

void ARM_Cortex_M3::save(const string& path) {
	OVPStatusMessage ovpstat;

	// save registers
	// FIXME: what about reg with id 16? should be status register
	for(unsigned int i = 0; i < 16; ++i) {
		uint32_t tmp = 0;
		icmRegInfoP reg = icmGetRegByIndex(processorP, i);
		icmReadRegInfoValue(processorP, reg, (void *)&tmp);
		OVPStatusMessage::Register *msgReg = ovpstat.add_reg();

		switch(i) {
			case 13:	msgReg->set_name("sp");
					break;
			case 14:	msgReg->set_name("lr");
					break;
			case 15:	msgReg->set_name("pc");
					break;
			case 16:	msgReg->set_name("sr");
					break;
			default:	char num[3];
					sprintf(num, "r%d", i);
					msgReg->set_name(num);
		}
					
		msgReg->set_value(tmp);
// std::cerr << "save " <<  i << ": " << tmp << std::endl;
	}


	string stmp;
	ovpstat.SerializeToString(&stmp);
	ofstream file;
	file.open(path.c_str(), ios::out);
	file << stmp;
	file.close();
/*	ofstream file(path.c_str(), ios::out | ios::trunc);
	if(!ovpstat.SerializeToOstream(&file)) {
		std::cerr << "Error writing to file " << path << "!" << std::endl;
	}*/
}

void ARM_Cortex_M3::restore(const string& path) {
	fstream input(path.c_str(), ios::in);
	OVPStatusMessage ovpstat;
	ovpstat.ParseFromIstream(&input);

	for(int i = 0; i < ovpstat.reg_size(); ++i) {
		const OVPStatusMessage::Register& ovpreg = ovpstat.reg(i); 
		uint32_t val = ovpreg.value();
//		std::cerr << "restore "<<ovpreg.name()<<": " << val << std::endl;
		icmRegInfoP reg = icmGetRegByIndex(processorP, i);
		icmWriteRegInfoValue(processorP, reg, (void *)&val);
	}
}

int main(int argc, char **argv) {
	if(argc != 2) {
		std::cerr << "Usage: " << argv[0] << " application" << std::endl;
		exit(1);
	}

	arm.init(false);

//	arm.createFullMMC();

//	arm.makeCallbackMemory(0x100, 0x20000000);
//	arm.makeCallbackMemory(0x8034, 0x0, 0x11c, 0x82e8);
//	arm.makeCallbackMemory(0x8034, 0x0, 0x940, 0x8040);
	arm.makeCallbackMemory(0x802c, 0x0, 0x930, 0x8038);
	arm.mapMemToCallback();


	ovpplatform.setCpu((void *) &arm);

	arm.makeGPRegister();
	arm.makeSTRegister();
	arm.makePCRegister();

	arm.startSimulation(argv[1]);
}
