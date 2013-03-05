#ifndef __T32CONNECTOR_HPP__
#define __T32CONNECTOR_HPP__

#include <util/Logger.hpp>
#include <utility>
#include <vector>

#include "sal/SALInst.hpp"
#include "t32config.hpp"

namespace fail {

  /**
   * \class T32Connector
   *
   * \brief Connector to the remote T32 device.
   */
  class T32Connector {
    public:
      T32Connector() : m_hostname("localhost"), m_port(T32_PORTNUM), m_packetlength(T32_PACKLEN), m_log("T32", false), m_connection_established(false) { };
      T32Connector(const char* hostname, const char*  port, const char*  packlen) : m_hostname(hostname), m_port(port),
           m_packetlength(packlen), m_log("T32", false), m_connection_established(false) { };

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

      bool isRunning(void) const { return getState() == RUNNING;  } ;
      bool isHalted(void)  const { return getState() == HALTED;  } ;
      bool isStopped(void) const { return getState() == STOPPED;  } ;
      bool isDown(void)    const { return getState() == DOWN; };

      bool isBigEndian(void) const { return !m_littleendian; };
      bool isLittleEndian(void) const { return m_littleendian; };

      void showDataRegions(void) const { showMemoryRegions(m_data_memory_map); };
      void showProgramRegions(void) const { showMemoryRegions(m_program_memory_map); };

      /**
       * @brief Start real time emulation
       */
      void go(void);
      void brk(void);
      void test(void);
    private:
      const char* m_hostname; //!< The hostname of the T32 device
      const char* m_port;     //!< The port to connect as configure in config.t32. Here we use strings, as required by the API
      const char* m_packetlength; //!< The packetlength of a command packet?
      const char* m_script;   //!< The user defined startup script to bring up the device
      mutable fail::Logger m_log;     //!< Logging (mutable for calls of the err method)

      std::string m_cpustring;

      typedef std::pair< address_t, address_t > region_t;
      typedef std::vector< region_t > memory_map_t;
      memory_map_t m_data_memory_map;
      memory_map_t m_program_memory_map;

      bool m_littleendian; //!< Big endian: 0, little endian != 0

      bool m_connection_established;

      /* ---- Private methods ---- */
      /**
       * @brief Internal error handling
       */
      bool err(int) const;

      bool scriptRunning(void) const;

      enum ICE_state_t { DOWN = 0, HALTED = 1, STOPPED = 2, RUNNING = 3, };
      int getState(void) const;

      enum MemoryDiscoverCommand_t { DISCOVER_PROGRAM_RAM = 2, DISCOVER_DATA_RAM = 1, DISCOVER_END = 0, };
      void discoverMemory(int discovertype, memory_map_t & map);
      void showMemoryRegions(const memory_map_t & map) const;
  };

} // end-of-namespace fail
#endif // __T32CONNECTOR_HPP__

