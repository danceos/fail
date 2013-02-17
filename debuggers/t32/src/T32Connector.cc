#include "T32Connector.hpp"
#include <iostream>
namespace fail {

T32Connector::T32Connector(char *hostname, unsigned  port, unsigned  packlen) : m_hostname(hostname), m_port(port), m_packetlength(packlen) { 
}


T32Connector::~T32Connector() {
  // Close Connection to T32 on object deletion. Also works, on simulator.terminate -> global object.
  std::cout << "[T32] Closing connection." << std::endl;
}



}

