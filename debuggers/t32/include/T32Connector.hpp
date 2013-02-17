#ifndef __T32CONNECTOR_HPP__
#define __T32CONNECTOR_HPP__

namespace fail {

class T32Connector {
  char* m_hostname;
  unsigned m_port;
  unsigned m_packetlength;

  public:
     T32Connector() { };
     T32Connector(char* hostname, unsigned  port, unsigned  packlen);
    ~T32Connector();

     
};

} // end-of-namespace fail
#endif // __T32CONNECTOR_HPP__

