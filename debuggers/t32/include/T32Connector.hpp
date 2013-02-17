#ifndef __T32CONNECTOR_HPP__
#define __T32CONNECTOR_HPP__

#include <util/Logger.hpp>
#include <utility>
#include <vector>

#include "sal/SALInst.hpp"

namespace fail {

  /**
   * \class T32Connector
   *
   * \brief Connector to the remote T32 device.
   */
  class T32Connector {
    public:
      T32Connector() : m_hostname("localhost"), m_port("20010"), m_packetlength("1024"), m_log("T32", false) { };
      T32Connector(const char* hostname, const char*  port, const char*  packlen);
      ~T32Connector();

      // Setters/getters
      void setHostname(const char* host) { m_hostname = host; };
      const char* getHostname(const char* host) const { return m_hostname; };

      void setPort(const char* port) { m_port = port; };
      const char* getPort(const char* port) const { return m_port; };

      void setPacketlength(const char* packetlength) { m_packetlength = packetlength; };
      const char* getPacketlength(const char* packetlength) const { return m_packetlength; };

      void setScript(const char* script) { m_script = script; };
      const char* getScript(const char* script) const { return m_script; };

      /**
       * @brief Connect and initialize T32 device
       */
      bool startup(void);

      bool isRunning(void) { return getState() == RUNNING;  } ;
      bool isHalted(void)  { return getState() == HALTED;  } ;
      bool isStopped(void) { return getState() == STOPPED;  } ;
      bool isDown(void) { return getState() == DOWN; };

      bool isBigEndian() { return !m_littleendian; };
      bool isLittleEndian() { return m_littleendian; };

    private:
      const char* m_hostname; //!< The hostname of the T32 device
      const char* m_port;     //!< The port to connect as configure in config.t32. Here we use strings, as required by the API
      const char* m_packetlength; //!< The packetlength of a command packet?
      const char* m_script;   //!< The user defined startup script to bring up the device
      fail::Logger m_log;     //!< Logging

      std::string m_cpustring;

      typedef std::vector< std::pair< address_t, address_t > > memory_map_t;
      memory_map_t m_data_memory_map;
      memory_map_t m_program_memory_map;

      bool m_littleendian; //!< Big endian: 0, little endian != 0

      /**
       * @brief Internal error handling
       */
      bool err(int);
      bool scriptRunning(void);
      enum ICE_state_t {
        DOWN = 0,
        HALTED = 1,
        STOPPED = 2,
        RUNNING = 3,
      };

      enum { DISCOVER_PROGRAM_RAM = 2, DISCOVER_DATA_RAM = 1, DISCOVER_END = 0, };
      int getState(void);
      void discoverMemory(int discovertype, memory_map_t & map);
  };

} // end-of-namespace fail
#endif // __T32CONNECTOR_HPP__

