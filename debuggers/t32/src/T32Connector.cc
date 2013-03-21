#include "T32Connector.hpp"
#include <iostream>
#include <t32.h>
#include "sal/MemoryInstruction.hpp"

using namespace fail;

T32Connector::~T32Connector() {
  if(m_connection_established){
  // Close Connection to T32 on object deletion. Also works, on simulator.terminate -> global object.
    m_log << "Closing connection." << std::endl;
  }
}

bool T32Connector::startup(){
  // Setup connection to Lauterbach
  m_log << "Remote connection: " << m_hostname << ":" << m_port << " - Packet length: " << m_packetlength << std::endl;
  T32_Config("NODE=", m_hostname);
  T32_Config("PACKLEN=", m_packetlength);
  T32_Config("PORT=", m_port);

  m_log << "Init." << std::endl;
  if(!err(T32_Init()) ) { return false; }

  m_log << "Attach." << std::endl;
  if(!err(T32_Attach(T32_DEV_ICD)) ) { return false; }

  word tmp, fpu, endian;
  char * cpuname;
  //if(!err(T32_GetCpuInfo( reinterpret_cast<char**>(&m_cpustring), &fpu, &endian, &tmp))) { return false; }
  if(!err(T32_GetCpuInfo( (&cpuname), &fpu, &endian, &tmp))) { return false; }
  m_littleendian = (endian != 0);
  m_cpustring = reinterpret_cast<const char*>(cpuname);
  m_log << "Attached to: " << m_cpustring << (isBigEndian() == true ? " (BIG-endian)" : " (little-endian)") << std::endl;

//  discoverMemory(DISCOVER_PROGRAM_RAM, m_program_memory_map);
//  discoverMemory(DISCOVER_DATA_RAM, m_data_memory_map);

  if(m_data_memory_map.size() > 0) {
    showDataRegions();
  }

  if(m_program_memory_map.size() > 0) {
    showProgramRegions();
  }

  m_connection_established = true;
  // TODO send init script.
  return true;
}

void T32Connector::discoverMemory(int access, memory_map_t& map) {
  dword pstart = 0, pend;
  word paccess;
  int c = 1;
  do {
    std::cout << "Discovering Memory Regions: " << c++ << std::endl;
    paccess = access;
    err(T32_GetRam(&pstart, &pend, &paccess));
    if(paccess != DISCOVER_END){
          map.push_back(std::make_pair<address_t, address_t>( pstart, pend ));
    }
  } while(paccess != DISCOVER_END);
}

bool T32Connector::scriptRunning() const {
  int retval = 0;
  err(T32_GetPracticeState( &retval ));
  return (retval == 1);
}

int T32Connector::getState() const {
  int retval = 0;
  err(T32_GetState( &retval ));
  return retval;
}


void T32Connector::showMemoryRegions(const memory_map_t& map) const {

  for(unsigned int i = 0; i < map.size(); i++){
    std::cout << "[" << i << "] 0x" << std::hex << map[i].first << " - 0x" << std::hex << map[i].second << std::endl;
  }
}

void T32Connector::go() {
  err(T32_Go());
}

void T32Connector::brk() {
  err(T32_Break());
}

#include "sal/t32/T32Constants.hpp"

void T32Connector::test() {
}

/* Default T32 error handler */
bool T32Connector::err(int errornum) const {
	if(errornum != 0){
    m_log << "Error: " << errornum << std::endl;
    return false;
	}
  return true;
}



