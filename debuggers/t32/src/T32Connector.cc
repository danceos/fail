#include "T32Connector.hpp"
#include <iostream>
#include <t32.h>

using namespace fail;

T32Connector::T32Connector(const char *hostname, const char*  port, const char*  packlen)
  : m_hostname(hostname), m_port(port), m_packetlength(packlen), m_log("T32", false)
{

}


T32Connector::~T32Connector() {
  // Close Connection to T32 on object deletion. Also works, on simulator.terminate -> global object.
  m_log << "Closing connection." << std::endl;
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
  char* cpuname[128];
  //if(!err(T32_GetCpuInfo( reinterpret_cast<char**>(&m_cpustring), &fpu, &endian, &tmp))) { return false; }
  if(!err(T32_GetCpuInfo( reinterpret_cast<char**>(&cpuname), &fpu, &endian, &tmp))) { return false; }
  m_littleendian = (endian != 0);
  m_cpustring = reinterpret_cast<const char*>(cpuname);

  m_log << "Attached to: " << m_cpustring << std::endl;

 // discoverMemory(DISCOVER_PROGRAM_RAM, m_program_memory_map);
 // discoverMemory(DISCOVER_DATA_RAM, m_data_memory_map);

  // TODO send init script.
  return true;
}

void T32Connector::discoverMemory(int access, memory_map_t& map) {
  dword pstart = 0, pend;
  word paccess;
  do {
    paccess = access;
    err(T32_GetRam(&pstart, &pend, &paccess));
    if(paccess != DISCOVER_END){
          map.push_back(std::make_pair<address_t, address_t>( pstart, pend ));
    }
  } while(paccess != DISCOVER_END);
}

bool T32Connector::scriptRunning() {
  int retval = 0;
  err(T32_GetPracticeState( &retval ));
  return (retval == 1);
}

int T32Connector::getState() {
  int retval = 0;
  err(T32_GetState( &retval ));
  return retval;
}


/* Default T32 error handler */
bool T32Connector::err(int ernum){
	if(ernum != 0){
    m_log << "Error: " << ernum << std::endl;
    return false;
	}
  return true;
}



